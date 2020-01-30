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
    + `builder.cc` contains the function `dequeAndInsertEdge()` which is executed by the scheduler thread. This function updates the data structure and performs an algorithm on it.
    + *Data structures*: `abstract_data_struc.h` is the top-level abstract class for a data structure. Specific implementations are contained in files `adListShared.h`, `adListCunked.h`, `stinger.h/stinger.cc`, and `darhh.h`. Each file implements the specific fashion in which the *update* operation needs to be performed on the given data structure.
    + *Graph Traversal*: `traversal.h` implements how each data structure needs to be traversed to get the in-neighbors and the out-neighbors. Traversal operation is achieved with two API functions: `in_neigh()` and `out_neigh()`. The specific traversal mechanism details of each data structure must be hidden under these two API functions. 
    + *Compute Models and Algorithms*: `topAlg.h` is the top-level algorithm file where every algorithm is registered. The specific implementation of each algorithm is contained in a file starting with *dyn_* (e.g., `dyn_bfs.h`). Each file implements both the compute models for a specific algorithm. For example, `dyn_bfs.h` contains functions `dynBFSAlg()` for the *incremental* compute model and `BFSStartFromScratch()` for the *recomputation from scratch* compute model. Most of the *recomputation from scratch* implementations have been borrowed from [GAP Benchmark Suite](https://github.com/sbeamer/gapbs) with slight modifications to conform to the API of SAGA-Bench. 
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

Note: To use other file formats, please change the file `src/dynamic/fileReader.h` to modify how SAGA-Bench should read the input file (i.e., change the function `convertCSVLineIntoEdge()`). 

## Compiling and Running SAGA-Bench 
### Basic Instructions for Running the Software
*Note: These basic instructions are for running SAGA-Bench software only and are NOT sufficient for integrating PCM for hardware characterization. For instructions to integrate PCM, please see below.*

SAGA-Bench is implemented in C++11 and the build system uses GNU Make. It uses both OPENMP and Pthreads to launch software threads. It has been tested on Ubuntu 18.04 LTS, Ubuntu 16.04 LTS, and CentOS. The experiments for our paper have been run on Intel Xeon Gold 6142 (Skylake) server (please refer to Section IV.A of the paper for more details). 

1. git clone https://github.com/abasak24/SAGA-Bench.git
2. mkdir bin obj
3. make 

An executable `frontEnd` will be created. `frontEnd` should be run with the following parameters. `./frontEnd --help` also provides this information.

```
-f : provides a location to an input graph file in .csv format
-b : batch size (500K is used in our paper evaluation)
-d : whether the input graph is directed or undirected. 0=undirected; 1=directed.
-w : whether weights should be read from the input file. 0=don't read weights; 1=read weights. Weights are required only for SSSP and SSWP. 
-s : data structure to be used (see DATA STRUCTURE OPTIONS below). 
-a : algorithm to be run (see ALGORITHM OPTIONS below). 
-n : max number of nodes the data structure must be initialized with. 
-t : number of data structure chunks for chunked-style adjacency list or degree-aware hashing. Each chunk corresponds to one thread. This parameter has no meaning for shared-style adjacency list and stinger (the value is not read for these two data structures).

DATA STRUCTURE OPTIONS: 1) adListShared 2) adListChunked 3) degAwareRHH 4) stinger
ALGORITHM OPTIONS: 1) prfromscratch 2) prdyn 3) ccfromscratch 4) ccdyn 5) mcfromscratch 6) mcdyn 7) bfsfromscratch 8) bfsyn 9) ssspfromscratch 10) ssspdyn 11) sswpfromscratch 12) sswpdyn
```

`runme.sh` provides example command lines for running experiments. 
Each run creates two csv files: **Alg.csv** and **Update.csv**. These files contain per-batch compute and update times, respectively, in seconds. 

### Reproducing Software-Level Characterization Results in the SAGA-Bench Paper
Please refer to Section IV.B of the paper for a detailed description of our methodology. We turned off the Turbo Boost feature. `profile.sh` is the script we used for producing all the software-level characterization results in the paper. The script uses *OMP_PROC_BIND* and *OMP_PLACES* for pinning OPENMP-generated software threads to hardware threads. To bind pthread-generated software threads to hardware threads, we use *pthread_setaffinity_np* (please see `src/dynamic/frontEnd.cc`, `src/dynamic/adListChunked.h`, and `src/dynamic/darhh.h`).

### Reproducing Hardware-Level Characterization Results in the SAGA-Bench Paper
We used Intel Processor Counter Monitor (PCM) for memory bandwidth, QPI bandwidth, and cache hit ratio/MPKI measurements. Please download, install, and compile PCM from here: https://github.com/opcm/pcm. Several resources for integrating PCM into SAGA-Bench have been provided in the folder **pcmResource**. First, it is necessary to change the *Makefile* of SAGA-Bench to link with PCM. We have provided an example *Makefile* in `pcmResource/Makefile_example`. Please change `PCM_DIR` in the example Makefile to the directory where you have installed your PCM. 
`pcmResource/pcmBasic.h` contains code to measure L2/L3 MPKI, hit ratios, and QPI link utilizations. Please check the comments in `pcmResource/pcmBasic.h` to understand the output format/order. 
`pcmResource/pcmMemory.h` contains code to measure memory bandwidth. Please check the comments and the functions `display_bandwidth_alg()` and `display_bandwidth_update()` in `pcmResource/pcmMemory.h` to understand the output format/order.  

Please read `pcmResource/PCM.txt` to understand how to include `pcmResource/pcmBasic.h` or `pcmResource/pcmMemory.h` into SAGA-Bench's *update* or *compute* phases to measure the hardware counters. `pcmResource/PCM.txt` also provides the intiliatization and finalization code that must be included before and after the test code. Execution with PCM will produce *.csv* files for the *update* and *compute* phases with the corresponding hardware-level measurements.

For example, to measure the memory bandwidth utilization details of the *update* phase, please do the following:
   + include `pcmResource/pcmMemory.h` in `src/dynamic/builder.cc` 
   + include the memory-measurement related initialization and finalization code provided in `pcmResource/PCM.txt` before and after `ds->update(el)` in `src/dynamic/builder.cc` (just whether the timers are currently started and stopped). In the intitialization code, please assign *true* to the boolean variable *update*.

Similarly, to measure the memory bandwidth utilization details of the *compute* phase (when let's say running *incremental pagerank*), please do the following:
   + include `pcmResource/pcmMemory.h` in `src/dynamic/dyn_pr.h` 
   + include the memory-measurement related initialization and finalization code provided in `pcmResource/PCM.txt` in the function `dynPRAlg()` before and after the algorithm implementation (just whether the timers are currently started and stopped). In the intitialization code, please assign *false* to the boolean variable *update*.

## Including Other Software Techniques in SAGA-Bench 
These are some guidelines to include one's own data structure or compute model in SAGA-Bench. 
1. *Including a new data structure*: Any new data structure must be inherited from the class `dataStruc` in `src/dynamic/abstract_data_struc.h`. The most important function to implement for a new data structure is `update()` which defines the mechanism to update a batch of edges into the data structure. Next, it is essential to implement the traversal mechanism of the data structure in the file `src/dynamic/traversal.h`.
2. *Including a new compute model*: It is possible to introduce a new compute model for any algorithm (let's say BFS) by writing a new function in the algorithm's file (`src/dynamic/dyn_bfs.h` for BFS). For example, `src/dynamic/dyn_bfs.h` currently contains functions `dynBFSAlg()` and `BFSStartFromScratch()` for incremental and non-incremental compute models. Next, the new function much be registered in the class `Algorithm` in `src/dynamic/topAlg.h`.

## Additional Results

## Contact
In case of issues, please contact Abanti at abasak@ucsb.edu. You could also raise an issue in Github so that the response can help other users. 