# Category-Aware E-Commerce Product Ranking System

A heap-based product ranking engine written in C that ranks products **within their own category** using a weighted scoring formula — eliminating the cross-category bias found in global ranking systems.

---

## How It Works

Each product receives a score computed from four metrics, with weights that vary by category:

```
S = (w_r × R/5) + (w_rev × log(reviews+1)/log(10000))
      + (w_s × log(sales+1)/log(100000)) + (w_p × 1/(1 + price/1000))
```

Products are inserted into a **per-category Max-Heap**, keeping the highest-scoring product at the root for O(log n) insertion and retrieval.

### Category Weight Distribution

| Category    | Rating | Reviews | Sales | Price |
|-------------|--------|---------|-------|-------|
| Electronics | 40%    | 30%     | 20%   | 10%   |
| Fashion     | 20%    | 40%     | 30%   | 10%   |
| Books       | 45%    | 35%     | 15%   | 5%    |
| Audio       | 50%    | 25%     | 15%   | 10%   |
| Storage     | 35%    | 25%     | 25%   | 15%   |

---

## Features

- **CRUD Operations** — Add, update, delete, and search products via a CLI menu
- **Top-N Retrieval** — Fetch the highest-ranked products in any category
- **Weight Customization** — Adjust category weights interactively; scores and heaps rebuild automatically
- **Analytics** — Per-category and global stats (avg rating, total reviews, total sales)
- **CSV Export** — Export ranked product data for use in external tools
- **Input Validation** — Range checks on rating (0–5), heap overflow prevention (max 100 products/category), and graceful error messages

---

## Complexity

| Operation               | Time Complexity  |
|-------------------------|------------------|
| Insert product          | O(log n)         |
| Delete / Update product | O(C × n)         |
| Top-N retrieval         | O(n + N log n)   |
| Search by name/category | O(C × n)         |
| Rebuild heap            | O(n)             |
| Category/global stats   | O(C × n)         |

> C = number of categories, n = products per category. Space complexity: O(C × n).

---

## Build & Run

**Requirements:** GCC (13+), standard C libraries only (`stdio.h`, `stdlib.h`, `string.h`, `math.h`)

```bash
# Compile
gcc -o ranking ranking.c -lm

# Run
./ranking
```

Tested on **Windows 10** and **Ubuntu 22.04**.

---

## Architecture

The system is organized into five layers:

```
Interface Layer   →   CLI menu, user input/output
     │
Manager Layer     →   CategoryManager: global access across all categories
     │
Category Layer    →   CategoryHeap: heap + weight config per category
     │
Heap Layer        →   MaxHeap: heapifyUp / heapifyDown for ordering
     │
Product Layer     →   Product struct: ID, name, rating, reviews, sales, price, score
```

### Modules

- **Product Management** — CRUD + score computation
- **Heap Management** — `heapifyUp()` / `heapifyDown()`, insertion, extraction
- **Category Management** — dynamic category creation, weight assignment
- **Search & Query** — case-insensitive substring match across all categories
- **Ranking & Analytics** — Top-N retrieval, per-category and global stats
- **Weight Customization** — interactive weight update + full heap rebuild
- **Export** — CSV output for ranked product lists

---

## Sample Output

```
------------------------------------------------------------
TOP PRODUCTS BY CATEGORY
------------------------------------------------------------
Category: Electronics
  1. Smart Watch X1            Score: 0.9231
  2. Gaming Mouse Elite        Score: 0.8920
  3. Mechanical Keyboard RGB   Score: 0.8834
------------------------------------------------------------
```

---

## Limitations

- No persistent storage — all data is RAM-only and lost on exit
- Weights must be set manually (no auto-tuning)
- Console-only interface

## Future Scope

- SQL / MongoDB integration for data persistence
- Web dashboard via Flask or Streamlit
- ML-based automatic weight optimization
- REST API for live e-commerce database integration
