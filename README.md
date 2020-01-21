<p align="center"><img src="https://github.com/abasak24/SAGA-Bench/blob/master/img/saga.png" width="350"></p>

#

[![License](https://img.shields.io/badge/License-BSD%203--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)
[![Version](https://img.shields.io/badge/version-alpha-orange.svg)](https://img.shields.io/badge/version-alpha-orange.svg)

**SAGA-Bench (Streaming Graph Analytics)** is designed to serve both the hardware and software communities as an open-source platform for streaming graph analytics research. Our goal is to provide a **unified, open-source** implementation of various dnamic graph processing data structures and algorithms. SAGA-Bench implements coarse-grained runtime profiling, as well as hardware analysis via Intel Processor Counter Monitor (PCM).

## Requirements
* SAGA-Bench is implemented in **C++11**, with scripts in **Python3** and **Bash**. 
* The build system uses **GNU Make**.
* Hardware Profiling is done with [**Intel PCM**](https://software.intel.com/en-us/articles/intel-performance-counter-monitor), which runs on Intel&reg; Core&trade;, Xeon&reg;, Atom&trade; and Xeon Phi&trade; processors.
* It has been tested on Ubuntu 18.04 LTS, Ubuntu 16.04 LTS, and CentOS.

## Kernels
Each kernel includes two versions: a traditional, "static" variant that processes the entire graph on every batch, and a "dynamic" variant that only processes "affected" vertices on every batch. 

In addition to a graph traversal micro-kernel, SAGA-Bench includes
+ Breadth-First Search (BFS)
+ Single-Source Shortest Paths (SSSP)
+ PageRank (PR)
+ Connected Components (CC)
+ Single-Source Widest Paths (SSWP)
+ Max Computation (MC)

## Data Structures 
The aforementioned kernels can be run with any of the following data structures:
+ Adjacency List
+ Stinger [1]
+ DegAwareRHH [2]

## Input Datasets
_TODO_

## Installing, Compiling, and Running SAGA-Bench 
1) git clone .....
2) mkdir bin obj
3) make 

An executable `frontEnd` will be created within the main directory. To run it, use the command with specific parameters:

```
./frontEnd -d directed -w weighted -f /datasetPath -b batchSize -s dataStructure -n numOfNodes -a algorithm -t numOfThreads
```
Alternatively, there is an example script, runme.sh, which could be executed for an example run. 
Each run creates two csv files: 1) Alg.csv and 2) Update.csv. These files contain per-batch compute and update times, respectively. 

## Using Intel Processor Counter Monitor (PCM) on SAGA-Bench

## References
[1] D. Ediger, R. McColl, J. Riedy, D. Bader, "Stinger: High performance data structure for streaming graphs", _IEEE Conference on High Performance Extreme Computing (HPEC),_ pp. 1-5, 2012.

[2] K. Iwabuchi, S. Sallinen, R. Pearce, B. V. Essen, M. Gokhale, S. Matsuoka, "Towards a distributed large-scale dynamic graph data store", _Graph Algorithms Building Blocks (GABB 2016) Workshop at IPDPS,_ 2016.
