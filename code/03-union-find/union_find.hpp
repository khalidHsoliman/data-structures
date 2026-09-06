#pragma once

#include <numeric>
#include <vector>

// Four takes on dynamic connectivity, in order of improvement.
// Each one is deliberately standalone so they can be read side by side.

// ---------- 1. quick-find ----------
// find O(1), union O(n) -- the scan inside union is what kills it
struct QuickFind {
    std::vector<int> id;   // id[i] is the component label of i

    explicit QuickFind(int n) : id(n) { std::iota(id.begin(), id.end(), 0); }

    int find(int p) const { return id[p]; }
    bool connected(int p, int q) const { return find(p) == find(q); }

    void unite(int p, int q) {
        const int from = id[p], to = id[q];
        if (from == to) return;
        for (int& label : id)
            if (label == from) label = to;
    }
};

// ---------- 2. quick-union ----------
// both operations cost the tree height, and nothing stops the tree growing tall
struct QuickUnion {
    std::vector<int> parent;

    explicit QuickUnion(int n) : parent(n) { std::iota(parent.begin(), parent.end(), 0); }

    int find(int p) const {
        while (p != parent[p]) p = parent[p];
        return p;
    }
    bool connected(int p, int q) const { return find(p) == find(q); }

    void unite(int p, int q) {
        const int a = find(p), b = find(q);
        if (a != b) parent[a] = b;
    }
};

// ---------- 3. weighted quick-union ----------
// always hang the smaller tree under the larger, which caps the height at log2(n)
struct WeightedQuickUnion {
    std::vector<int> parent;
    std::vector<int> tree_size;

    explicit WeightedQuickUnion(int n) : parent(n), tree_size(n, 1) {
        std::iota(parent.begin(), parent.end(), 0);
    }

    int find(int p) const {
        while (p != parent[p]) p = parent[p];
        return p;
    }
    bool connected(int p, int q) const { return find(p) == find(q); }

    void unite(int p, int q) {
        int a = find(p), b = find(q);
        if (a == b) return;
        if (tree_size[a] < tree_size[b]) std::swap(a, b);   // a is now the larger root
        parent[b] = a;
        tree_size[a] += tree_size[b];
    }
};

// ---------- 4. weighted quick-union + path compression ----------
// identical to (3) except that find() flattens the tree on its way up,
// which is why find() is no longer a const operation
struct WeightedQuickUnionPC {
    std::vector<int> parent;
    std::vector<int> tree_size;

    explicit WeightedQuickUnionPC(int n) : parent(n), tree_size(n, 1) {
        std::iota(parent.begin(), parent.end(), 0);
    }

    int find(int p) {
        while (p != parent[p]) {
            parent[p] = parent[parent[p]];   // point p at its grandparent
            p = parent[p];
        }
        return p;
    }
    bool connected(int p, int q) { return find(p) == find(q); }

    void unite(int p, int q) {
        int a = find(p), b = find(q);
        if (a == b) return;
        if (tree_size[a] < tree_size[b]) std::swap(a, b);
        parent[b] = a;
        tree_size[a] += tree_size[b];
    }
};
