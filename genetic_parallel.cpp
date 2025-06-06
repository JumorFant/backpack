#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <thread>
#include <mutex>
#include <random>
using namespace std;

struct Item {
    int value;
    int weight;
};

int N;
int W;
vector<Item> items;

const int POP_SIZE = 1000;
const int GENERATIONS = 5000;
const double CROSSOVER_RATE = 0.9;
const double MUTATION_RATE = 0.01;

mutex mtx;

int fitness(const vector<int>& chromosome) {
    int total_value = 0;
    int total_weight = 0;

    for (int i = 0; i < N; ++i) {
        if (chromosome[i] == 1) {
            total_value += items[i].value;
            total_weight += items[i].weight;
        }
    }

    if (total_weight <= W) {
        return total_value;
    }

    int penalty = (total_weight - W) * 20;
    return total_value - penalty;
}

vector<int> randomChromosome() {
    vector<int> chromosome(N);
    for (int i = 0; i < N; ++i) {
        chromosome[i] = rand() % 2;
    }
    return chromosome;
}
vector<int> greedyChromosome() {
    vector< pair<double, int> > ratio_index;

    for (int i = 0; i < N; ++i) {
        double ratio = (double)items[i].value / items[i].weight;
        ratio_index.push_back(make_pair(-ratio, i)); // отрицательное для сортировки по убыванию
    }

    sort(ratio_index.begin(), ratio_index.end());

    vector<int> chromosome(N, 0);
    int total_weight = 0;

    for (int i = 0; i < N; ++i) {
        int idx = ratio_index[i].second;
        if (total_weight + items[idx].weight <= W) {
            chromosome[idx] = 1;
            total_weight += items[idx].weight;
        }
    }

    return chromosome;
}
vector<int> tournamentSelection(const vector< vector<int> >& population) {
    int i = rand() % POP_SIZE;
    int j = rand() % POP_SIZE;

    if (fitness(population[i]) > fitness(population[j])) {
        return population[i];
    }
    return population[j];
}

vector<int> crossover(const vector<int>& parent1, const vector<int>& parent2) {
    vector<int> child(N);
    int point = rand() % N;

    for (int i = 0; i < N; ++i) {
        if (i < point) {
            child[i] = parent1[i];
        } else {
            child[i] = parent2[i];
        }
    }
    return child;
}

void mutate(vector<int>& chromosome) {
    for (int i = 0; i < N; ++i) {
        if ((double)rand() / RAND_MAX < MUTATION_RATE) {
            chromosome[i] = 1 - chromosome[i];
        }
    }
}

void worker(int start, int end,
            const vector< vector<int> >& population,
            vector< vector<int> >& new_population,
            vector<int>& best_chromosome,
            int& best_fitness) {

    // Инициализируем локальный генератор случайных чисел
    random_device rd;
    mt19937 rng(rd() + start);  // уникальное зерно на каждый поток
    uniform_real_distribution<double> prob(0.0, 1.0);
    uniform_int_distribution<int> bit_dist(0, 1);
    uniform_int_distribution<int> index_dist(0, POP_SIZE - 1);
    uniform_int_distribution<int> gene_dist(0, N - 1);

    for (int i = start; i < end; ++i) {
        // Турнирная селекция
        int idx1 = index_dist(rng);
        int idx2 = index_dist(rng);
        vector<int> parent1 = fitness(population[idx1]) > fitness(population[idx2])
                                ? population[idx1]
                                : population[idx2];

        idx1 = index_dist(rng);
        idx2 = index_dist(rng);
        vector<int> parent2 = fitness(population[idx1]) > fitness(population[idx2])
                                ? population[idx1]
                                : population[idx2];

        vector<int> child(N);

        // Кроссовер
        if (prob(rng) < CROSSOVER_RATE) {
            int point = gene_dist(rng);
            for (int j = 0; j < N; ++j) {
                if (j < point)
                    child[j] = parent1[j];
                else
                    child[j] = parent2[j];
            }
        } else {
            child = parent1;
        }

        // Мутация
        for (int j = 0; j < N; ++j) {
            if (prob(rng) < MUTATION_RATE) {
                child[j] = 1 - child[j];
            }
        }

        // Вычисление приспособленности
        int current_fitness = fitness(child);

        // Защищаем доступ к общим данным
        mtx.lock();
        new_population[i] = child;

        int weight = 0;
        for (int j = 0; j < N; ++j) {
            if (child[j] == 1) {
                weight += items[j].weight;
            }
        }

        if (current_fitness > best_fitness && weight <= W) {
            best_fitness = current_fitness;
            best_chromosome = child;
        }
        mtx.unlock();
    }
}

int main() {
    ifstream infile("ks_400_0");
    if (!infile.is_open()) {
        cerr << "Error:can't open file " << endl;
        return 1;
    }

    infile >> N >> W;
    if (N <= 0 || W <= 0) {
        cerr << "Error:incorrect N or W" << endl;
        return 1;
    }

    for (int i = 0; i < N; ++i) {
        Item item;
        infile >> item.value >> item.weight;
        items.push_back(item);
    }

    vector< vector<int> > population;
    for (int i = 0; i < POP_SIZE; ++i) {
        if (i == 0) {
            population.push_back(greedyChromosome());
        } else {
            population.push_back(randomChromosome());
        }
    }
    /*int w  = 0;
    for(int i = 0;i<POP_SIZE;i++){
        for(int j =0;j<N;j++){
                w+=population[i][j]*items[j].weight;
        }
        cout<<i<<" "<<w<<" "<<W<<endl;
        w = 0;
    }*/
    vector<int> best_chromosome;
    int best_fitness = 0;
    auto start1 = chrono::high_resolution_clock::now();

    for (int gen = 0; gen < GENERATIONS; ++gen) {
        vector< vector<int> > new_population(POP_SIZE);

        int threads_count = 6;
        int batch_size = POP_SIZE / threads_count;
        vector<thread> threads;

        for (int t = 0; t < threads_count; ++t) {
            int start = t * batch_size;
            int end = (t == threads_count - 1) ? POP_SIZE : (t + 1) * batch_size;

            threads.push_back(thread(worker, start, end,
                                     cref(population),
                                     ref(new_population),
                                     ref(best_chromosome),
                                     ref(best_fitness)));
        }
        for (int t = 0; t < threads.size(); ++t) {
            threads[t].join();
        }

        population = new_population;
    }
    auto end1 = chrono::high_resolution_clock::now();

    if (best_chromosome.empty()) {
        cout << "No one best." << endl;
        return 1;
    }

    int total_weight = 0, total_value = 0;
    cout << "Max value: " << best_fitness << endl;
    cout << "Items: ";
    for (int i = 0; i < N; ++i) {
        if (best_chromosome[i] == 1) {
            cout << i << " ";
            total_weight += items[i].weight;
            total_value += items[i].value;
        }
    }
    cout << endl << "Total value: " << total_value << ", Total weight: " << total_weight << endl;
    cout << "completed in " << chrono::duration<double, milli>(end1 - start1).count() << " ms" << endl;

    ofstream fout("output.txt");
    fout<<"ks_400_0 parallel: Best value :"<<best_fitness<<" ,completed in: "<<chrono::duration<double, milli>(end1 - start1).count()<<" ms"<<endl;
    fout.close();

    return 0;
}
