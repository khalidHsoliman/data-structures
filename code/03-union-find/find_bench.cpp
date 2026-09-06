// Read-heavy benchmark: build the structure once, then hammer find().
// The union-heavy table barely separates weighting from path compression;
// this is the workload where path compression actually earns its line.
#include <chrono>
#include <cstdio>
#include <vector>

#include "bench.hpp"
#include "union_find.hpp"

template <typename UF>
double time_finds(int n, const std::vector<Pair>& pairs, const std::vector<int>& queries) {
    UF uf(n);
    for (const Pair& pair : pairs)   // construction is not timed
        if (!uf.connected(pair.first, pair.second)) uf.unite(pair.first, pair.second);

    const auto start = std::chrono::steady_clock::now();
    long long total = 0;
    for (int q : queries) total += uf.find(q);
    const auto end = std::chrono::steady_clock::now();

    volatile long long sink = total;   // the roots have to be used for the loop to survive
    (void)sink;
    return std::chrono::duration<double>(end - start).count();
}

int main() {
    const int finds = 5000000;
    std::printf("%9s %12s %12s %9s\n", "n", "weighted", "wtd+PC", "speedup");
    for (int n : {32000, 100000, 250000, 1000000}) {
        const std::vector<Pair> pairs = random_pairs(n);
        const std::vector<int> queries = random_queries(n, finds, 7);

        const double weighted = time_finds<WeightedQuickUnion>(n, pairs, queries);
        const double weighted_pc = time_finds<WeightedQuickUnionPC>(n, pairs, queries);

        std::printf("%9d %12.4f %12.4f %8.2fx\n", n, weighted, weighted_pc, weighted / weighted_pc);
        std::fflush(stdout);
    }
}
