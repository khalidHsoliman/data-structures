---
title: "Data Structures : Union-Find"
description: "Four attempts at the same problem, from 35 seconds down to 3 milliseconds, with every step justified by measurement."
pubDate: 2026-09-06
tags: ["cpp", "algorithms", "computerscience", "datastructures"]
series: "Data Structures"
---

In the [last post](/blog/02-analysis-of-algorithms/) we built a way to measure how an algorithm grows : run it, double the input, look at the ratio of the times. That was all theory with a deliberately silly example. This time we use it on a real problem, and we are going to solve the same problem four times, each one better than the last, with the measurements deciding when we are allowed to move on.

By the end we will have gone from 35 seconds to 3 milliseconds on the same input. Not by writing faster C++, but by changing what the data structure actually is.

# The problem

**Dynamic connectivity.** You have `n` objects, and connections keep arriving one at a time. At any moment you need to answer one question : *are these two objects connected?*

That sounds abstract, so here is what it actually looks like in practice :

- Pixels in an image that touch and share a colour, which is how flood fill and connected-component labelling work. I have written a version of this more than once for imaging tools.
- Machines on a network, where you want to know whether two of them can reach each other at all.
- Whether water can percolate from the top of a porous material to the bottom, which is the classic physics application.
- Building a minimum spanning tree with Kruskal's algorithm, which we will get to later in this series and which needs exactly this structure.

The important word is **dynamic**. Connections arrive over time, and we have to answer queries in between them. If we had all the connections up front we could just do a graph traversal once and be done.

One property saves us a lot of work : being connected is an **equivalence relation**. Every object is connected to itself, the relation goes both ways, and if `a` connects to `b` and `b` connects to `c` then `a` connects to `c`. That means the objects fall into disjoint groups, and we never need to remember *which* wire connects two machines, only *which group* each machine ended up in. All we have to track is the grouping.

# The API

Following the same approach as the intro, we design the interface before we write a single line of implementation :

```cpp
struct UnionFind {
    UnionFind(int n);          // n objects, none connected yet
    int  find(int p);          // which group is p in?
    void unite(int p, int q);  // put p and q in the same group
};
```

`find` returns some identifier for the group. We don't care what it is, only that two objects are connected exactly when their identifiers match. That single decision — *what does the identifier mean?* — is what separates the four implementations below.

> A note on naming : the operation is called *union* everywhere in the literature, but `union` is a keyword in C++, so it cannot be a function name. I have used `unite`. You will also see `join`, `merge` and `union_` in the wild.

# Attempt 1 : quick-find

The simplest thing that could possibly work. Keep an array where `id[p]` **is** the group identifier, directly.

```cpp
struct QuickFind {
    std::vector<int> id;
    QuickFind(int n) : id(n) { for (int i = 0; i < n; i++) id[i] = i; }

    int find(int p) { return id[p]; }

    void unite(int p, int q) {
        int pid = id[p], qid = id[q];
        for (int i = 0; i < (int)id.size(); i++)
            if (id[i] == pid) id[i] = qid;
    }
};
```

`find` is a single array read. It is impossible to do better than that, which is where the name comes from.

The problem is `unite`. To merge two groups we have to walk the **entire array** and relabel every member of one of them. So a single union costs `n` operations, and doing `n` unions costs `n²`. Quadratic, and we already know from the last post what quadratic means at scale : dead on arrival.

# Attempt 2 : quick-union

Let's flip the trade-off. Instead of storing the group identifier directly, store a **parent** — each entry points at another object, and following the chain far enough gets you to a root that points at itself. The root is the group identifier.

```cpp
struct QuickUnion {
    std::vector<int> parent;
    QuickUnion(int n) : parent(n) { for (int i = 0; i < n; i++) parent[i] = i; }

    int find(int p) {
        while (p != parent[p]) p = parent[p];
        return p;
    }

    void unite(int p, int q) { parent[find(p)] = find(q); }
};
```

`unite` is now a single assignment : hang one root under the other. Beautiful. And the array is secretly a forest of trees, which is a genuinely satisfying thing to realise about a flat array of integers.

But we have moved the cost rather than removed it. `find` now walks up a chain, and nothing stops that chain from getting long. In the worst case you union things in an order that builds a single line of `n` nodes, and every `find` walks all of it.

So : is that better or worse than quick-find? This is exactly the question the doubling test exists to answer, so let's stop arguing and measure.

# Attempt 3 : weighting

The reason quick-union degrades is that we pick which root goes under which one **arbitrarily**. `parent[find(p)] = find(q)` always hangs p's tree under q's tree, even when p's tree is enormous and q's is a single node. That's how you build a chain.

The fix is almost embarrassingly small : keep track of how big each tree is, and always hang the **smaller** tree under the bigger one.

```cpp
struct Weighted {
    std::vector<int> parent, size;
    Weighted(int n) : parent(n), size(n, 1) { for (int i = 0; i < n; i++) parent[i] = i; }

    int find(int p) {
        while (p != parent[p]) p = parent[p];
        return p;
    }

    void unite(int p, int q) {
        int a = find(p), b = find(q);
        if (a == b) return;
        if (size[a] < size[b]) { parent[a] = b; size[b] += size[a]; }
        else                   { parent[b] = a; size[a] += size[b]; }
    }
};
```

One extra array, one comparison, and the guarantee changes completely. A node's depth only increases when the tree it lives in gets absorbed by a bigger one, and each time that happens the size of its tree at least **doubles**. You can only double from 1 to `n` about `log₂ n` times, so no node can ever be deeper than `log₂ n`. For a million objects that is a depth of 20, forever.

That argument is the whole idea. Nothing about the code is clever — the cleverness is entirely in noticing which choice was arbitrary and making it deliberately.

# Attempt 4 : path compression

One more refinement, and it is nearly free. Every time we walk up a chain in `find`, we already know that everything we touched belongs to the same root. So why leave them deep? Flatten them on the way past.

```cpp
int find(int p) {
    while (p != parent[p]) {
        parent[p] = parent[parent[p]];   // point at the grandparent
        p = parent[p];
    }
    return p;
}
```

That single added line is **path halving** : as we walk, each node we pass gets re-pointed at its grandparent, which roughly halves the length of the path for everyone who comes after. No second pass, no recursion, one extra assignment inside a loop we were already running.

The theoretical result here is famous : weighted quick-union with path compression makes `m` operations on `n` objects cost effectively `m` times a constant, where the "constant" is the inverse Ackermann function and is below 5 for any `n` you will ever encounter in this universe. In practice it means the structure is, for engineering purposes, flat.

Hold on to that word "theoretical", because the measurements have something to say about it.

# The measurements

Same benchmark for all four : `n` objects, `n` random unions, release build.

| n | quick-find | quick-union | weighted | weighted+PC |
|---|---|---|---|---|
| 8,000 | 0.0237 | 0.0024 | 0.0001 | 0.0001 |
| 16,000 | 0.0965 | 0.0130 | 0.0002 | 0.0001 |
| 32,000 | 0.4788 | 0.0712 | 0.0004 | 0.0002 |
| 64,000 | 2.0172 | 0.3256 | 0.0008 | 0.0005 |
| 128,000 | 8.5959 | 1.9967 | 0.0018 | 0.0013 |
| 256,000 | **35.3568** | 12.3044 | 0.0040 | **0.0032** |

And the doubling ratios, which are where the actual information is :

| | ratios | verdict |
|---|---|---|
| quick-find | 4.1, 5.0, 4.2, 4.3, 4.1 | ~4 → **quadratic** |
| quick-union | 5.4, 5.5, 4.6, 6.1, 6.2 | worse than 4 → **worse than quadratic here** |
| weighted | 2.0, 2.0, 2.0, 2.2, 2.2 | ~2 → **linear** |
| weighted+PC | 1.0, 2.0, 2.5, 2.6, 2.5 | ~2 → **linear** |

Three things worth pulling out of that.

**Quick-find is textbook quadratic**, exactly as predicted, and 35 seconds against 3 milliseconds is an **11,000x** gap on identical input. That is the headline, and it is entirely a data structure decision. Nobody wrote faster code.

**Quick-union is the interesting one.** Look at it carefully : in absolute terms it beats quick-find at every size, 12 seconds against 35 at the top of the table. But its ratio is *worse* — around 5.5 to 6 where quick-find sits at 4.2. It is winning now and losing later. Extrapolate both columns and they cross somewhere in the low millions, after which the "improvement" is slower than the thing it improved on. If I had benchmarked at `n = 8000` only, and looked at absolute times, I would have shipped it. This is the single best argument I know for why order of growth is worth caring about at all.

**Path compression barely shows up.** Weighted and weighted+PC are within about 25% of each other, and both are linear. Not the dramatic win the theory promises. That bothered me enough to go and check properly, so I wrote a second benchmark that builds the structure once and then does five million `find` calls against it — a read-heavy workload rather than a union-heavy one. There path compression finally earns its line : it wins by about **3.7x** at n = 32,000, easing to roughly **3.1x** by n = 1,000,000. That drift is worth noticing — as the array outgrows the cache, both versions start paying for memory rather than for pointer chasing, and the advantage of being flat shrinks. The million-object row is also the noisiest of the four, swinging between 2.0x and 3.3x across repeated runs, which is its own reminder that a single measurement at the top of the range is not a result.

So the honest summary is this : **weighting does essentially all of the work, and path compression is a constant-factor refinement on top of it.** Both are worth having, they are three lines between them, but they are not remotely equal partners, and you would not learn that from the asymptotic analysis alone. The inverse Ackermann result is true and also not the thing that made the difference.

> **The bug that cost me an hour.** My first version of this benchmark didn't use random pairs. I thought I was being clever and generated them arithmetically instead, something like `int p = (i * 104729) % n;`. It ran fine for the first two rows and then segfaulted. The reason : at around `i = 20500` that multiplication overflows a 32-bit `int`, the result goes negative, and `% n` on a negative number stays negative in C++. I was indexing an array with a negative number, inside a data structure that was completely correct. The simple version — just ask a random number generator for two numbers — was both easier to read and the one without the bug. There is a lesson in that which goes well beyond this post.

# In the wild

**Union-find is the right answer more often than people expect.** Any time you find yourself asking "are these two things in the same group" and the groups keep merging, this is the structure, and reaching for a graph traversal instead is a common and expensive mistake. Flood fill, connected-component labelling, mesh region merging, Kruskal's MST, detecting cycles as you add edges — all the same shape underneath.

**It is one array. Sometimes two.** No nodes, no pointers, no allocation after construction, everything contiguous. In a cache-heavy workload that matters more than the asymptotics suggest, and it is why this structure tends to beat its own theory on real hardware. It is also why the whole thing fits in ninety lines.

**Know which operation dominates.** The weighted-versus-compression result above is a specific case of a general point : an average cost is only meaningful against a workload. If your access pattern is union-heavy, path compression is nearly irrelevant. If you build once and query forever, it's worth better than 3x. Measure the mix you actually have.

**The thing union-find will not do is come apart.** There is no `disconnect`. Once two groups merge, that is permanent, and if your problem involves connections being *removed* then this structure is the wrong one and no amount of tuning fixes it. Choosing a data structure is mostly about noticing this kind of thing before you have written 400 lines against it.

# Try it yourself

The crossover claim above is a real prediction, and predictions are for testing. Extend the benchmark past `n = 256,000` and find the point where quick-union actually becomes slower than quick-find. Tell me if it lands where the ratios say it should, because I would like to know whether that's a genuine effect or an artefact of my particular input pattern.

If you want a harder one : replace path halving with **full path compression**, where `find` does two passes and points every node it visited directly at the root. It is a stronger flattening and more work per call. Measure whether it actually wins.

Next up : **stacks, queues and bags**, where we finally get to put a resizing array and a linked list side by side and watch cache behaviour make a mess of the theory.

All the code from this post is in the [series repo](https://github.com/khalidHsoliman/data-structures/tree/main/code/03-union-find) : the four structures in `union_find.hpp`, and both benchmarks beside it. The repo versions add a small `connected()` helper that the listings above leave out, so the benchmark loop reads the way the API section says it should.

---

*This series is motivated by Robert Sedgewick's [Algorithms, Part I](https://www.coursera.org/learn/algorithms-part1) and [Part II](https://www.coursera.org/learn/algorithms-part2) on Coursera.*
