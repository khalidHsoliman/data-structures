# Data Structures

### [cpp-data-structures.pages.dev](https://cpp-data-structures.pages.dev/)

A series working through Sedgewick's *Algorithms, Part I* in C++, one post at a time.
The blog and the code it talks about live in the same repo, so a post and its code are always in the same commit.

## Layout

- `site/` — the Astro blog that gets deployed
- `code/` — one folder per post, each buildable on its own

## Posts

| # | Post | Code |
|---|---|---|
| 02 | [Analysis of algorithms](https://cpp-data-structures.pages.dev/blog/02-analysis-of-algorithms/) | [`code/02-analysis-of-algorithms`](code/02-analysis-of-algorithms) |

## Building the code

Any post folder builds the same way:

```bash
cd code/02-analysis-of-algorithms
cmake -S . -B build
cmake --build build --config Release   # --config is required on Windows/Xcode
./build/three_sum                      # build/Release/three_sum.exe on Windows
```

Release is not optional — these programs are benchmarks, and debug timings are meaningless.

## Running the site

```bash
cd site
npm install
npm run dev          # preview at localhost:4321
```
