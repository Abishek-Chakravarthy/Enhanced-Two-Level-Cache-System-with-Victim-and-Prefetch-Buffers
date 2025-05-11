
# Comparative Cache Simulation Project

## Overview
This project simulates and compares two cache architectures: a **Basic Cache** and an **Enhanced Multi-Level Cache**. The goal is to evaluate how advanced cache features affect performance under spatial and temporal memory access patterns.

## Author
- Abishek Chakravarthy (CS22B2054)
- IIITDM, February 2025

## Files
- `ca.cpp`: Implements a basic single-level cache using FIFO replacement.
- `assign3i.cpp`: Implements an enhanced cache with a two-level hierarchy, victim cache, stream buffers, write buffer, and LRU replacement for L2.

## Features

### Basic Cache (`ca.cpp`)
- Single-level cache structure with FIFO policy.
- Simulates spatial and temporal locality.
- Dynamically boosts access probabilities to mimic real-world access patterns.

### Enhanced Cache (`assign3i.cpp`)
- Two-level cache system (L1: Direct-mapped, L2: 4-way set-associative).
- Victim Cache: Recovers recently evicted blocks from L1.
- Stream Buffers: Prefetches sequential blocks for spatial locality.
- Write Buffer: Stores dirty blocks before flushing to memory.
- LRU Policy in L2: Keeps frequently accessed blocks in cache.

## Simulation
Two access patterns were simulated for both implementations:
- **Spatial Access Pattern**: Neighboring blocks are more likely to be accessed.
- **Temporal Access Pattern**: Recently accessed blocks are more likely to be accessed again.

## Results Summary

| Pattern        | Basic Hit Rate | Enhanced Hit Rate |
|----------------|----------------|-------------------|
| Spatial        | 50.7%          | 88.34%            |
| Temporal       | 8.69%          | Significantly Improved (due to LRU, buffers) |

## Key Takeaways
- The enhanced architecture significantly improves performance.
- Stream buffers and LRU policy improve hit rates under real-world access scenarios.
- Advanced caching mechanisms are crucial for reducing access latency.

## How to Run
Compile and run `assign3i.cpp` using any standard C++ compiler:

```bash
g++ assign3i.cpp -o cache_sim
./cache_sim
```

## Output
Simulation logs are saved in `extended_cache_simulation_log.txt`, and performance stats are printed to the console.

## Future Work
- Fine-tuning of cache/buffer sizes for various workloads.
- Simulate additional access patterns and policies.
