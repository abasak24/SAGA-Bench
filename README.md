<p align="center"><img src="https://github.com/abasak24/SAGA-Bench/blob/master/img/saga.png" width="350"></p>

#

[![License](https://img.shields.io/badge/License-BSD%203--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)
[![Version](https://img.shields.io/badge/version-alpha-orange.svg)](https://img.shields.io/badge/version-alpha-orange.svg)

This repository contains code, scripts, and user instructions related to the following [ISPASS 2020](https://ispass.org/ispass2020/) paper: 

> A. Basak, J. Lin, R. Lorica, X. Xie, Z. Chishti, A. Alameldeen, and Y. Xie, *SAGA-Bench: Software and Hardware Characterization of Streaming Graph Analytics Workloads*


**SAGA-Bench** is a C++ benchmark for **S**tre**A**ming **G**raph **A**nalytics containing a collection of data structures and compute models on the same platform for a fair and systematic study. SAGA-Bench simultaneously provides 1) a common platform for performance analysis studies of software techniques and 2) a benchmark for architecture studies. SAGA-Bench implements runtime profiling, as well as hardware analysis via [Intel Processor Counter Monitor (PCM)](https://software.intel.com/en-us/articles/intel-performance-counter-monitor).

## Requirements
* SAGA-Bench is implemented in **C++11**, with scripts in **Python3** and **Bash**. 
* The build system uses **GNU Make**.
* Hardware Profiling is done with [**Intel PCM**](https://software.intel.com/en-us/articles/intel-performance-counter-monitor), which runs on Intel&reg; Core&trade;, Xeon&reg;, Atom&trade; and Xeon Phi&trade; processors.
* It has been tested on Ubuntu 18.04 LTS, Ubuntu 16.04 LTS, and CentOS.

## Data Structures in SAGA-Bench
+ Adjacency List
+ Stinger [1]
+ DegAwareRHH [2]

## Compute Models
+ Recomputation from scratch
+ Incremental

## Algorithms
Each algorithm has been implemented in both the aforementioned compute mdoels. 
+ Breadth-First Search (BFS)
+ Single-Source Shortest Paths (SSSP)
+ PageRank (PR)
+ Connected Components (CC)
+ Single-Source Widest Paths (SSWP)
+ Max Computation (MC)

## Overview of the Directory Structure 
_TODO_
+ Brief description of how src folder is organized
+ what is inputInfo? pcmInfo? errorExtractor.cc? 

## Input Datasets
_TODO_
+ Shuffle datasets (script provided in folder inputResource)
+ Add weight and timestamps (script provided in folder inputResource)
+ The result input graph should look something like test.csv

## Installing, Compiling, and Running SAGA-Bench 
_TODO_
1) git clone .....
2) mkdir bin obj
3) make 

An executable `frontEnd` will be created within the main directory. To run it, use the command with specific parameters:

```
./frontEnd -d directed -w weighted -f /datasetPath -b batchSize -s dataStructure -n numOfNodes -a algorithm -t numOfThreads
```
Alternatively, there is an example script, runme.sh, which could be executed for an example run. 
Each run creates two csv files: 1) Alg.csv and 2) Update.csv. These files contain per-batch compute and update times, respectively. 

## Reproducing Software-Level Profiling Results in the SAGA-Bench Paper
_TODO_

Need to mention to run the profile.sh script. 

## Reproducing Hardware-Level Profiling Results in the SAGA-Bench Paper
_TODO_
+ We used Intel Processor Counter Monitor (PCM) for memory bandwidth, QPI bandwidth, and cache hit ratio/MPKI measurements. PCM can be found here: https://github.com/opcm/pcm
+ Provide instructions on how to use PCM.
+ Download install PCM 
+ Modify the Makefile (example provided in folder pcmResource)
+ Information on how to include PCM code before and after experimental code is provided in folder pcmResource

## Adding your own data structure, compute model, or algorithm 
_TODO_

## Miscellaneous
_TODO_

+ Mention errorExtractor.cc: validation between FS and INC models 
+ traverse algorithm 
+ adList single threaded
+ acknowledge GAP from where we borrow and modify software techniques 

## References
[1] D. Ediger, R. McColl, J. Riedy, D. Bader, "Stinger: High performance data structure for streaming graphs", _IEEE Conference on High Performance Extreme Computing (HPEC),_ pp. 1-5, 2012.

[2] K. Iwabuchi, S. Sallinen, R. Pearce, B. V. Essen, M. Gokhale, S. Matsuoka, "Towards a distributed large-scale dynamic graph data store", _Graph Algorithms Building Blocks (GABB 2016) Workshop at IPDPS,_ 2016.