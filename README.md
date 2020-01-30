<p align="center"><img src="https://github.com/abasak24/SAGA-Bench/blob/master/img/saga.png" width="350"></p>

#

[![License](https://img.shields.io/badge/License-BSD%203--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)
[![Version](https://img.shields.io/badge/version-alpha-orange.svg)](https://img.shields.io/badge/version-alpha-orange.svg)

This repository contains code, scripts, and user instructions related to the following [ISPASS 2020](https://ispass.org/ispass2020/) paper: 

> **A. Basak, J. Lin, R. Lorica, X. Xie, Z. Chishti, A. Alameldeen, and Y. Xie, *SAGA-Bench: Software and Hardware Characterization of Streaming Graph Analytics Workloads***

**SAGA-Bench** is a C++ benchmark for **S**tre**A**ming **G**raph **A**nalytics containing a collection of data structures and compute models on the same platform for a fair and systematic study. SAGA-Bench simultaneously provides 1) a common platform for performance analysis studies of software techniques and 2) a benchmark for architecture studies. SAGA-Bench implements runtime profiling, as well as hardware analysis via [Intel Processor Counter Monitor (PCM)](https://github.com/opcm/pcm).

## Components of SAGA-Bench
Please refer to the paper for a detailed description of each component. 
1. **Data Structures**: 
     + Adjacency List (shared style multithreading)
     + Adjacency List (chunked style multithreading)
     + Stinger 
     + Degree-Aware Hashing
2. **Compute Models**:
     + Recomputation from scratch
     + Incremental
3. **Algorithms** (each algorithm has been implemented in both the aforementioned compute models): 
     + Breadth-First Search (BFS)
     + Single-Source Shortest Paths (SSSP)
     + PageRank (PR)
     + Connected Components (CC)
     + Single-Source Widest Paths (SSWP)
     + Max Computation (MC)

## Overview of the Directory Structure 
1. **src/dynamic**: Core implementations of the benchmark. They include the following:
    + `frontEnd.cc` is the main/top file which reads command-line parameters, reads edge batches from the input file, initiates the data structure, and launches the scheduler thread. 
    + `builder.cc` contains function `dequeAndInsertEdge` which is executed by the scheduler thread. This functions updates the data structure and performs an algorithm on it.
    + *Data structures*: `abstract_data_struc.h` is the top-level abstract class for a data structure. Specific implementations are contained in files `adListShared.h`, `adListCunked.h`, `stinger.h/stinger.cc`, and `darhh.h`. Each file implements the specific fashion in which the *update* operation needs to be performed on the given data structure.
    + *Graph Traversal*: `traversal.h` implements how each data structure needs to be traversed to get the in-neighbors and the out-neighbors. Traversal operation is achieved with two API functions: `in_neigh()` and `out_neigh()`. The specific traversal mechanism details of each data structure must be hidden from these two API functions. 
    + *Compute Models and Algorithms*: `topAlg.h` is the top-level algorithm file where every algorithm is registered. The specific implementation of each algorithm is contained in a file starting with *dyn_* (e.g., `dyn_bfs.h`). Each file implements both the compute models for a specific algorithm. For example, `dyn_bfs.h` contains functions `dynBFSAlg` for the *incremental* compute model and `BFSStartFromScratch` for the *recomputation from scratch* compute model. Most of the *recomputation from scratch* implementations have been borrowed from [GAP Benchmark Suite](https://github.com/sbeamer/gapbs) with slight modifications to conform to the API of SAGA-Bench. 
2. **src/common**: Some utility elements borrowed from [GAP Benchmark Suite](https://github.com/sbeamer/gapbs).
3. **inputResource**: Several resources to produce input dataset file formats (see below).
4. **pcmResource**: Several resources to integrate [Intel PCM](https://github.com/opcm/pcm) with SAGA-Bench for hardware-level characterization (see below).
5. Others: `profile.sh` and `runme.sh` are example scripts to run experiments. `test.csv` is an example of the input dataset format. 

## Input Datasets
We used *.csv* file format and the example of a typical dataset is provided in `test.csv`. Each line of the file means the following:
```
[source vertex ID], [destination vertex ID], [timestamp], [weight]
```
Graph datasets are first randomly shuffled to break any ordering in the input files. This is done to ensure the realistic scenario that streaming edges are not likely to come in any pre-defined order. The shuffled input file is then read in batches of 500K edges in our evaluation setup. Please refer to the paper to check which datasets we used for our evaluation. The resources for preparing the input datasets are provided in the folder **inputResource**. `inputResource/shuffle.sh` can be used to shuffle a dataset file in .txt format (e.g., those found in [SNAP](https://snap.stanford.edu/data/)). After shuffling, timestamps and weights can be added using `inputResource/addWeightAndTime.sh` and `inputResource/appendValues.py`, which will result in the final *.csv* format.

Note: To use other file formats, please change the file `src/dynamic/fileReader.h` to modify how SAGA-Bench should read the input file (i.e., change the function `convertCSVLineIntoEdge`). 

## Compiling and Running SAGA-Bench 
### Basic Instructions for Running the Software
SAGA-Bench is implemented in C++11 and the build system uses GNU Make. It has been tested on Ubuntu 18.04 LTS, Ubuntu 16.04 LTS, and CentOS. The experiments for our paper have been run on Intel Xeon Gold 6142 (Skylake) server (please refer to Section IV of the paper for more details). 

1. git clone https://github.com/abasak24/SAGA-Bench.git
2. mkdir bin obj
3. make 

An executable `frontEnd` will be created. `frontEnd` should be run with the following parameters. `./frontEnd --help` also provides this information.

```
-f : provides a location to an input graph file
-b : batch size (500K is used in our paper evaluation)
-d : whether the input graph is directed or undirected. 0 = undirected; 1 = directed.
-w : whether weights should be read from the input file. 0 = don't read weights; 1 = read weights. Weights are required only for SSSP and SSWP. 
-s : data structure to be used. 
-a : algorithm to be run. 
-t : 

DATA STRUCTURE OPTIONS: 1) adListShared 2) adListChunked 3) degAwareRHH 4) stinger
ALGORITHM OPTIONS: 1) prfromscratch 2) prdyn 3) ccfromscratch 4) ccdyn 5) mcfromscratch 6) mcdyn 7) bfsfromscratch 8) bfsyn 9) ssspfromscratch 10) ssspdyn 11) sswpfromscratch 12) sswpdyn
```

`runme.sh` provides example command-line for running experiments. 
Each run creates two csv files: 1) Alg.csv and 2) Update.csv. These files contain per-batch compute and update times, respectively. 

### Reproducing Software-Level Profiling Results in the SAGA-Bench Paper
Need to mention to run the profile.sh script. 

### Reproducing Hardware-Level Profiling Results in the SAGA-Bench Paper
_TODO_
+ We used Intel Processor Counter Monitor (PCM) for memory bandwidth, QPI bandwidth, and cache hit ratio/MPKI measurements. PCM can be found here: https://github.com/opcm/pcm
+ Provide instructions on how to use PCM.
+ Download install PCM 
+ Modify the Makefile (example provided in folder pcmResource)
+ Information on how to include PCM code before and after experimental code is provided in folder pcmResource

## Additional Results

## Adding your own data structure, compute model, or algorithm 
_TODO_

## Miscellaneous
_TODO_

+ Mention errorExtractor.cc: validation between FS and INC models 
+ traverse algorithm 
+ adList single threaded
+ acknowledge GAP from where we borrow and modify software techniques 

## Contact
In case of issues, please contact Abanti at abasak@ucsb.edu. You could also raise an issue so that the response can help other users. 