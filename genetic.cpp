#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
using namespace std;
struct Item {
    int value;
    int weight;
};

int N;
int W;
vector<Item> items;

const int POP_SIZE = 100;
const int GENERATIONS = 1000;
const double CROSSOVER_RATE = 0.7;
const double MUTATION_RATE = 0.01;


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

    int penalty = (total_weight - W) * 10;
    return total_value - penalty;
}

vector<int> randomChromosome() {
    vector<int> chromosome(N);
    for (int i = 0; i < N; ++i) {
        chromosome[i] = rand() % 2;
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

int main() {

    ifstream infile("ks_10000_0");
    infile >> N >> W;

    for (int i = 0; i < N; ++i) {
        Item item;
        infile >> item.value >> item.weight;
        items.push_back(item);
    }

    vector< vector<int> > population;

    for (int i = 0; i < POP_SIZE; ++i) {
        population.push_back(randomChromosome());
    }

    vector<int> best_chromosome;
    int best_fitness = 0;

    for (int gen = 0; gen < GENERATIONS; ++gen) {
        vector< vector<int> > new_population;

        for (int i = 0; i < POP_SIZE; ++i) {
            vector<int> parent1 = tournamentSelection(population);
            vector<int> parent2 = tournamentSelection(population);

            vector<int> child;

            if ((double)rand() / RAND_MAX < CROSSOVER_RATE) {
                child = crossover(parent1, parent2);
            } else {
                child = parent1;
            }

            mutate(child);

            new_population.push_back(child);

            int current_fitness = fitness(child);
            int w = 0;
            for (int i = 0; i < N; ++i) {
                if (child[i] == 1) {
                    w+=items[i].weight;
                }
            }
            if (current_fitness > best_fitness && w<=W) {
                best_fitness = current_fitness;
                best_chromosome = child;
            }
        }

        population = new_population;
    }

    int w = 0,c = 0;
    cout << "Max value " << best_fitness << endl;
    cout << "items: ";
     for (int i = 0; i < N; ++i) {
        if (best_chromosome[i] == 1) {
            cout << i << " ";
            w+=items[i].weight;
            c+=items[i].value;
        }
    }
    cout << endl<< c<<" "<<w;
    ofstream fout("output.txt");
    fout<<"ks_1000_0: "<<best_fitness;
    fout.close();
    return 0;
}
