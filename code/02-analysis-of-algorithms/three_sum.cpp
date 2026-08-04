#include <chrono>
#include <cstdio>
#include <random>
#include <vector>

// counts triples (i < j < k) whose values sum to zero
int count_triples(const std::vector<int>& a) {
    const std::size_t n = a.size();
    int count = 0;
    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t j = i + 1; j < n; ++j)
            for (std::size_t k = j + 1; k < n; ++k)
                if (a[i] + a[j] + a[k] == 0) ++count;
    return count;
}

std::vector<int> random_input(std::size_t n) {
    std::mt19937 gen(12345);
    std::uniform_int_distribution<int> dist(-1000000, 1000000);
    std::vector<int> a(n);
    for (auto& x : a) x = dist(gen);
    return a;
}

double time_it(std::size_t n) {
    auto a = random_input(n);
    auto start = std::chrono::steady_clock::now();
    volatile int sink = count_triples(a);
    (void)sink;
    auto end = std::chrono::steady_clock::now();
    return std::chrono::duration<double>(end - start).count();
}

int main() {
    double previous = 0.0;
    for (std::size_t n = 250; n <= 8000; n *= 2) {
        double seconds = time_it(n);
        if (previous > 0.0)
            std::printf("%6zu %10.3f %8.1f\n", n, seconds, seconds / previous);
        else
            std::printf("%6zu %10.3f %8s\n", n, seconds, "-");
        previous = seconds;
    }
}
