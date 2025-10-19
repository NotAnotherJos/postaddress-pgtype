# MALH: Multi-Attribute Linear Hashed Files

A tiny storage engine implementing **multi-attribute hashing** + **linear hashing**,
with a simple CLI to **create**, **insert**, **query** and **inspect stats** of a relation.

## ✨ Features
- **Multi-attribute hashing (MAH)** via a **choice vector** to combine hash bits from
  multiple attributes into one composite hash.  
- **Linear hashing (LH)** to grow the data file incrementally by page splits.  
- **Selection (n-dim PMR)** with wildcards (`?`) and patterns (`%`), and **projection** (no distinct).  
- Simple disk layout: `R.info` (global), `R.data` (main pages), `R.ovflow` (overflow pages).

## 💫 Design Overview

### File layout
- `R.info`: number of attributes, depth `d`, split pointer `sp`, page count, tuple count, choice vector.
- `R.data`: fixed-size pages (header + variable-length tuple strings).  
- `R.ovflow`: overflow pages chained off a data page.  
(These mirror the spec’s structure and growth model.)  

### Hashing
- **MAH**: compute per-attribute hash, assemble bits according to the **choice vector** to form a 32-bit combined hash.  
- **LH**: with `2^d` primary buckets and split pointer `sp`, use `d` or `d+1` low-order bits to route tuples; after every `c` insertions (approx page capacity), split bucket `sp`, append a new page, then advance `sp` (wrap to `0` and increment `d` when `sp == 2^d`).  

### Query model
- **Selection** supports:
  - literal equality,  
  - single-attribute wildcard `?`,  
  - pattern with `%` (zero or more chars).  
- **Projection**: choose attribute indexes or `*`.

## Build

    make    # builds: create, insert, query, stats, dump, gendata

## 🚀 Quickstart
Create a relation with 3 attributes, 4 initial pages (rounded to 2^n) and a choice vector:

    ./create R 3 4 "0,0:0,1:0,2:1,0:1,1:2,0"

Insert data:

    echo "100,abc,xyz" | ./insert R
    ./gendata 250 3 101 | ./insert R

Inspect storage and distribution:

    ./stats R

Query with selection + projection:

    ./query "2,1" from R where "101,?,?"
    ./query "*"   from R where "?,%ab%,?

## 🖊 Implementation Notes
MAH: tupleHash() picks bits from per-attribute hashes per the choice vector and composes a 32-bit hash.

LH: addToRelation() triggers split after threshold c≈1024/(10*n), redistributes tuples between old/new buckets using d+1 bits, and advances sp.

Pages: each page stores (freeStart, ovflowIdx, nTuples) in the header, followed by packed tuple strings; overflow pages share the same layout.

Selection/Projection: custom scanners with % pattern matching and attribute re-ordering (no distinct).
# postaddress-pgtype
Custom PostgreSQL data type implementation for structured address storage and comparison.
