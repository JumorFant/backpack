#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <chrono>
using namespace std;
using namespace chrono;

struct Item {
    int value;
    int weight;

    double density()  {
        return (double)value / weight;
    }
};

int main() {
    ifstream input("ks_40_0");
    if (!input) {
        cerr << "Error:can't open file!" << endl;
        return 1;
    }

    int N, W;
    input >> N >> W;

    vector<Item> items(N);


    for (int i = 0; i < N; ++i) {
        input >> items[i].value >> items[i].weight;
    }


    sort(items.begin(), items.end(), []( Item &a,  Item &b) {
        return a.density() > b.density();
    });

    int totalWeight = 0;
    int totalValue = 0;
    vector<Item> selected;
    high_resolution_clock::time_point start = high_resolution_clock::now();
    for (int i = 0; i < (int)items.size(); ++i) {
        if (totalWeight + items[i].weight <= W) {
            totalWeight += items[i].weight;
            totalValue += items[i].value;
            selected.push_back(items[i]);
        }
    }
    high_resolution_clock::time_point end1 = high_resolution_clock::now();
    duration<double,milli> time1= end1-start;
    cout << "Max benefit: " << totalValue << endl;
    cout << "time:"<<time1.count()<<endl;
    cout << "items:" << endl;

    for (int i = 0; i < (int)selected.size(); ++i) {
        cout << "benefit: " << selected[i].value
             << ", weight: " << selected[i].weight << endl;
    }
    cout<<totalWeight<<" "<<W<<endl;
    ofstream fout("output.txt");
    fout << "ks_10000_0: " << totalValue;

    return 0;
}
