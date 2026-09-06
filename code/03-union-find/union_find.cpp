// Union-heavy benchmark: n objects, n random unions.
// This is the table in the post -- it is what separates the four implementations.
#include <chrono>
#include <cstdio>
#include <vector>

#include "bench.hpp"
#include "union_find.hpp"

template <typename UF>
double time_unions(int n, const std::vector<Pair>& pairs) {
    UF uf(n);
    const auto start = std::chrono::steady_clock::now();
    for (const Pair& pair : pairs)
        if (!uf.connected(pair.first, pair.second)) uf.unite(pair.first, pair.second);
    const auto end = std::chrono::steady_clock::now();

    volatile int sink = uf.find(0);   // stops the whole loop being optimized away
    (void)sink;
    return std::chrono::duration<double>(end - start).count();
}

int main() {
    std::printf("%8s %10s %10s %10s %10s\n", "n", "quick-f", "quick-u", "weighted", "wtd+PC");
    for (int n = 8000; n <= 256000; n *= 2) {
        const std::vector<Pair> pairs = random_pairs(n);

        const double quick_find = time_unions<QuickFind>(n, pairs);
        const double quick_union = time_unions<QuickUnion>(n, pairs);
        const double weighted = time_unions<WeightedQuickUnion>(n, pairs);
        const double weighted_pc = time_unions<WeightedQuickUnionPC>(n, pairs);

        std::printf("%8d %10.4f %10.4f %10.4f %10.4f\n", n, quick_find, quick_union, weighted,
                    weighted_pc);
        std::fflush(stdout);
    }
}
