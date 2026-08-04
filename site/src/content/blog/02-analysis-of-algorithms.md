---
title: "Data Structures : Analysis of Algorithms"
description: "Why we analyze algorithms, how to do it scientifically, and what the math actually buys you in production C++."
pubDate: 2026-08-04
tags: ["cpp", "algorithms", "computerscience", "datastructures"]
series: "Data Structures"
---

So, it has been a while since the first post in this series. Six years, to be precise. I wrote an [introduction to this series](https://dev.to/khalidhsoliman/data-structures-intro-5d86) on dev.to, covering why I wanted to write it, who it's for, and why everything here is built as an abstract data type in C++ rather than pulled out of the STL. That's all still true, so rather than repeat it, go read it if you want the framing. This is where the actual work starts.

In the years since, I have been writing performance-critical C++ in CAD, 3D tooling and medical imaging, which means I got to watch a lot of this theory either save a project or completely fail to matter, and I have opinions about which is which now. Those opinions will show up throughout.

Before we start implementing anything, we need a way to talk about **how fast** our code is, and more importantly, a way to talk about it that still means something when the input gets bigger. That's what this post is about.

# Why do we analyze algorithms

The honest answer is that we want to **predict** something without running it. If I hand you a program and tell you it sorts a million records in 3 seconds, you still can't tell me what happens with a billion records. Maybe it's 3000 seconds, maybe it's 3 million. The difference between those two numbers is the difference between "run it overnight" and "this will never finish".

There are a few other reasons, but they all come back to prediction :

- Avoid **performance bugs**, which are the worst kind of bug, because the program is correct, the tests pass, and it only falls apart in production when the customer's dataset is 100x bigger than yours.
- Compare two candidate solutions **before** committing to one of them, since rewriting a data structure that is already wired into 40 files is expensive.
- Know when to stop. If you can prove your algorithm is already near the theoretical limit, you stop optimizing and go do something useful.

Donald Knuth's insight - which is really the foundation of this whole field - is that a computer program is a physical process we can study scientifically. We can build a mathematical model of it, use the model to make a prediction, and then run an experiment to check whether the prediction holds. If it doesn't, the model is wrong and we refine it. That's the whole method, and it works.

# An experiment

Let's do the science part first, because it's more fun, and the math will make more sense afterwards.

Here's the classic **3-sum** problem : given `n` distinct integers, how many triples sum to exactly zero? The obvious solution is to try every triple.

```cpp
// counts triples (i < j < k) whose values sum to zero
int count_triples(const std::vector<int>& a) {
    const std::size_t n = a.size();
    int count = 0;
    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t j = i + 1; j < n; ++j)
            for (std::size_t k = j + 1; k < n; ++k)
                if (a[i] + a[j] + a[k] == 0)
                    ++count;
    return count;
}
```

Now we need to time it. In modern C++ this is `<chrono>` and it's mercifully simple :

```cpp
double time_it(std::size_t n) {
    auto a = random_input(n);
    auto start = std::chrono::steady_clock::now();
    volatile int sink = count_triples(a);   // see the note below
    (void)sink;
    auto end = std::chrono::steady_clock::now();
    return std::chrono::duration<double>(end - start).count();
}
```

> **A trap you will fall into exactly once.** The first time I ran this, every single input size took 0.000 seconds. The compiler noticed that I never used the return value of `count_triples`, decided the entire call was dead code, and deleted it. My benchmark was measuring nothing at all, very quickly. The `volatile` sink is the cheap fix : it forces the compiler to keep the result. Whenever a benchmark reports a suspiciously round zero, this is the first thing to check.

Then we run what Sedgewick calls a **doubling test** : run the program for some `n`, then for `2n`, then `4n`, and look at the *ratio* between consecutive running times. Here's what I get on my machine :

| n | debug (s) | release (s) | debug ratio | release ratio |
|---|---|---|---|---|
| 250 | 0.012 | 0.000 | - | - |
| 500 | 0.099 | 0.002 | 8.0 | 8.8 |
| 1,000 | 0.791 | 0.012 | 8.0 | 6.0 |
| 2,000 | 6.377 | 0.086 | 8.1 | 7.5 |
| 4,000 | 51.597 | 0.651 | 8.1 | 7.6 |
| 8,000 | - | 4.950 | - | 7.6 |

I ran this twice by accident, once in a debug build and once in release, and I'm glad I did, because the two columns together say more than either one alone. The release build is roughly **79 times faster** at `n = 4000`. Seventy-nine times. And the ratio column didn't move.

That's the point of the whole exercise. Both ratios settle on **8**, and that number is the whole answer. Here's why : if the running time follows a power law `T(n) = a · n^b`, then doubling the input gives

```
T(2n) / T(n) = a·(2n)^b / (a·n^b) = 2^b
```

The constant `a` cancels out completely, which is the beautiful part - we don't need to know anything about the machine, the compiler, the optimization level or the allocator. Turning on the optimizer changed `a` by a factor of 79 and left `b` exactly where it was. We measured `2^b ≈ 8`, so `b = log₂(8) = 3`, and our algorithm is **cubic** no matter how we build it.

Two smaller things worth noticing in that table. The `n = 250` release row reads 0.000 because the work finished faster than the clock could see it, and the noisy 8.8 and 6.0 ratios just above it come from the same problem - those timings are so small that startup cost and scheduling noise are a large fraction of the measurement. Only trust a doubling ratio once the times are comfortably above your timer's resolution, which here means somewhere north of a tenth of a second. If you need numbers for small inputs, run the whole thing a thousand times and divide.

And now we can predict, using the release column since that's the one that matters. One hundred thousand records comes out at about 2.7 hours, and one million lands at roughly `10⁷` seconds, or **112 days**. I didn't need a supercomputer to learn that, I needed six measurements and a logarithm.

# The mathematical model

The experiment gave us the exponent, but we should also be able to derive it on paper. To do that we need two things.

The first is a **cost model** : we pick one operation that dominates the work and we count how many times it happens, instead of trying to count nanoseconds. For 3-sum, the natural choice is array accesses.

The second is the count itself. The number of triples with `i < j < k` is

```
n(n-1)(n-2) / 6
```

and each one performs 3 array accesses, so the total is about `n³/2` array accesses.

Notice I said "about". Writing out `3n(n-1)(n-2)/6` is precise and completely useless for reasoning, because when `n` is large the lower-order terms contribute almost nothing. So we use **tilde notation** and write `~ n³/2`, which formally means the ratio between the true count and `n³/2` approaches 1 as `n` grows. Everything that doesn't matter gets thrown away, and what's left is the shape of the curve.

### Order of growth

Once you drop the constants too, almost every algorithm you'll ever write falls into one of a handful of classes :

| growth | name | what it looks like in code | example |
|---|---|---|---|
| 1 | constant | a couple of statements | adding two numbers |
| log n | logarithmic | halving a range each step | binary search |
| n | linear | one loop | finding the maximum |
| n log n | linearithmic | divide and conquer | mergesort |
| n² | quadratic | two nested loops | insertion sort |
| n³ | cubic | three nested loops | our 3-sum |
| 2ⁿ | exponential | exhaustive search | subset enumeration |

This little table is most of what practical analysis is. When you look at a function you've just written, count the nesting, find the row, and you know what you're dealing with.

### A note on big-O

You have certainly seen **O(n²)** written everywhere, so it's worth being precise about it, because most of us - myself included, for years - use it wrong.

- **O(f(n))** is an **upper** bound. It says the algorithm is *no worse than* this.
- **Ω(f(n))** is a **lower** bound.
- **Θ(f(n))** means both, so it's the actual growth.

The problem is that an upper bound alone doesn't tell you what will happen when you run the program, it only tells you what won't happen. Saying insertion sort is O(n⁵) is perfectly true and perfectly worthless. When people say "this is O(n log n)" they almost always mean Θ, and when a paper proves an O bound and nothing else, the algorithm may well be far faster in practice than the notation admits. Sedgewick is fairly blunt about this in the course and I think he's right : for engineering purposes, we want the tilde approximation and the actual constant, not just a bound somebody can't be sued over.

# Memory

Time gets all the attention but memory follows the same method, you just count bytes instead of operations. In C++ this is more transparent than in most languages, which is one of the reasons I like writing this series in C++.

On a typical 64-bit build :

- `int` is 4 bytes, `double` is 8, a pointer is 8.
- A `std::vector<int>` object is usually 24 bytes - three pointers - plus `4n` bytes on the heap for the elements, plus whatever the allocator rounds up to, plus possible unused capacity.
- A linked list node holding a single `int` is 4 bytes of payload, an 8 byte pointer, and 4 bytes of padding to keep the alignment, so **16 bytes to store 4 bytes of data**.

That last one is worth staring at. The same data in a vector costs 4 bytes per element and in a linked list costs 16, and we haven't even talked about what that does to the cache yet. We will, when we implement both.

# In the wild

Everything above is the theory. Here is what five years of shipping performance-critical C++ actually taught me about it.

**Order of growth tells you what is impossible, constants tell you what to ship.** If two candidates are both `n log n`, the analysis is finished and it has told you nothing useful, and the decision now comes down to memory layout, allocation behaviour and cache misses. If one candidate is quadratic and your `n` can reach a million, you don't need to benchmark anything, it's already dead.

**Asymptotically identical does not mean equally fast.** Traversing a `std::list` and traversing a `std::vector` are both Θ(n), and on real data the vector is routinely an order of magnitude faster because every element is already in cache while the list is chasing pointers all over the heap. The model counted operations and assumed they all cost the same. On modern hardware, they emphatically do not - a cache miss can cost a hundred times what a hit costs. When your measurements disagree with your model this badly, that's not a failure of the method, it's the method telling you your cost model was wrong.

**n is usually small, and then everything reverses.** Below a few dozen elements, insertion sort beats quicksort, and a linear scan through a small array beats a hash lookup. This isn't a curiosity, it's why every serious sort implementation switches to insertion sort under some threshold. Know your real input sizes before you get clever.

**Measure the build you actually ship.** The 79x gap between the two columns in that table is not unusual for C++, it's normal. A debug build turns off inlining, keeps every temporary alive and instruments the standard library containers with bounds checking, and the result tells you almost nothing about production behaviour. I have seen more than one "optimization" get merged on the strength of a debug-build measurement and then do nothing, or actively hurt, in release. If you take one habit from this post, make it this one.

**Measure before you optimize, every single time.** The number of times I have been certain I knew where the time was going, and been wrong, is genuinely embarrassing. The hot loop is almost never where it feels like it should be. Analysis narrows the search, the profiler finds the answer.

# Try it yourself

Our 3-sum is cubic, but there's a much better way : sort the array first, then for each element scan the rest with two pointers moving toward each other. Implement it, run the same doubling test on it, and watch the ratio drop from 8 to about 4. Then work out from the ratio what the order of growth is and check it against the code. Build it in release, and push `n` well past 8,000 while you're at it - a quadratic algorithm will take you further in five seconds than the cubic one managed in five minutes.

If you do it, tell me what ratio you measured and on what hardware, I'm curious how much it varies.

Next post we start with the actual data structures, beginning with **union-find** and the dynamic connectivity problem, where we'll take a solution from quadratic down to nearly linear in about four steps, and every one of those steps will be justified with exactly the method we just built.

All the code from this post is in the [series repo](https://github.com/khalidHsoliman/data-structures/tree/main/code/02-analysis-of-algorithms).

---

*This series is motivated by Robert Sedgewick's [Algorithms, Part I](https://www.coursera.org/learn/algorithms-part1) and [Part II](https://www.coursera.org/learn/algorithms-part2) on Coursera.*
