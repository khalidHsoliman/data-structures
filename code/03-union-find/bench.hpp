#pragma once

#include <random>
#include <utility>
#include <vector>

using Pair = std::pair<int, int>;

// Generated once per n and reused, so every implementation sees exactly the same
// workload and the random number generator stays outside the timed region.
inline std::vector<Pair> random_pairs(int n) {
    std::mt19937 gen(42);
    std::uniform_int_distribution<int> pick(0, n - 1);
    std::vector<Pair> pairs(n);
    for (Pair& pair : pairs) pair = {pick(gen), pick(gen)};
    return pairs;
}

// n random objects to call find() on, for the read-heavy benchmark
inline std::vector<int> random_queries(int n, int count, unsigned seed) {
    std::mt19937 gen(seed);
    std::uniform_int_distribution<int> pick(0, n - 1);
    std::vector<int> queries(count);
    for (int& q : queries) q = pick(gen);
    return queries;
}
