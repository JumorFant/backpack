#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <algorithm>
#include <chrono>
using namespace std;
using namespace chrono;

struct Item {
    int value;
    int weight;
    double ratio;

    Item(int v, int w) {
        value = v;
        weight = w;
        ratio = (double)v / w;
    }
};

struct Node {
    int level;
    int profit;
    int weight;
    double bound;
    vector<bool> taken;

    Node(int l, int p, int w, int n) {
        level = l;
        profit = p;
        weight = w;
        bound = 0.0;
        taken = vector<bool>(n, false);
    }
};

double bound(Node u, int n, int W, const vector<Item>& items) {
    if (u.weight >= W)
        return 0;

    double result = u.profit;
    int totalWeight = u.weight;

    for (int i = u.level; i < n && totalWeight + items[i].weight <= W; ++i) {
        totalWeight += items[i].weight;
        result += items[i].value;
    }

    if (u.level < n && totalWeight < W) {
        int i = u.level;
        result += (W - totalWeight) * items[i].ratio;
    }

    return result;
}

struct Compare {
    bool operator()(const Node& a, const Node& b) {
        return a.bound < b.bound;
    }
};

int main() {
    ifstream input("ks_100_0");
    if (!input) {
        cerr << "error: can't open file!" << endl;
        return 1;
    }

    int N, W;
    input >> N >> W;

    vector<Item> items;
    for (int i = 0; i < N; ++i) {
        int v, w;
        input >> v >> w;
        items.push_back(Item(v, w));
    }

    sort(items.begin(), items.end(), [](const Item& a, const Item& b) {
        return a.ratio > b.ratio;
    });

    priority_queue<Node, vector<Node>, Compare> pq;

    Node u(0, 0, 0, N);
    Node v(0, 0, 0, N);
    u.bound = bound(u, N, W, items);
    pq.push(u);

    int maxProfit = 0;
    vector<bool> bestTaken;
    high_resolution_clock::time_point start = high_resolution_clock::now();

    while (!pq.empty()) {
        u = pq.top();
        pq.pop();

        if (u.level == N || u.bound <= maxProfit)
            continue;

        v = Node(u.level + 1, u.profit, u.weight, N);
        v.taken = u.taken;
        v.weight += items[u.level].weight;
        v.profit += items[u.level].value;
        v.taken[u.level] = true;

        if (v.weight <= W && v.profit > maxProfit) {
            maxProfit = v.profit;
            bestTaken = v.taken;
        }

        v.bound = bound(v, N, W, items);
        if (v.bound > maxProfit)
            pq.push(v);

        v = Node(u.level + 1, u.profit, u.weight, N);
        v.taken = u.taken;
        v.taken[u.level] = false;
        v.bound = bound(v, N, W, items);
        if (v.bound > maxProfit)
            pq.push(v);
    }
    high_resolution_clock::time_point end = high_resolution_clock::now();
    duration<double,milli> time = end-start;
    cout << "max benefit: " << maxProfit << endl;
    cout << "time:" <<time.count()<< endl;
    cout << "items:" << endl;

    for (int i = 0; i < N; ++i) {
        if (bestTaken[i]) {
            cout << "benefit: " << items[i].value << ", weight: " << items[i].weight << endl;
        }
    }

    return 0;
}
