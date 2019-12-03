#!/bin/bash
dataDir=/home/abasak/graph_project/graphs/snap/
STRUCTURES=(adListShared)
batchSize=500000 
RUNS=1
NumThreads=46

# whether each algorithm is weighted or unweighted
declare -A ALGORITHMS 
ALGORITHMS=(         
         [ccdyn]=0                  
         [prdyn]=0            
         [bfsdyn]=0        
         [mcdyn]=0          
         [sswpdyn]=1         
         [ssspdyn]=1              
)

# Max num_nodes to initialize for each dataset
declare -A DATASETS
DATASETS=(
       [soc-LiveJournal1.shuffle.t.w.csv]=4847571       
       [com-orkut.ungraph.shuffle.t.w.csv]=3072441
       #[soc-pokec-relationships.shuffle.t.w.csv]=1632803
       #[wiki-topcats.shuffle.t.w.csv]=1791489
       #[twitter_rv.shuffle.t.w.csv]=61578415
       [rmat.csv]=33554432
)

runs=${RUNS}
while [ $runs -gt 0 ]
do 
  for dataset in "${!DATASETS[@]}"; do  
    for structure in "${STRUCTURES[@]}"; do
      for algorithm in "${!ALGORITHMS[@]}"; do 
        # if-else to make sure orkut runs in undirected mode 
        if [ "$dataset" == "com-orkut.ungraph.shuffle.t.w.csv" ]; 
        then
         echo ./frontEnd -d 0 -w ${ALGORITHMS[$algorithm]} -f ${dataDir}$dataset -b ${batchSize} -s $structure -n ${DATASETS[$dataset]} -a $algorithm -t ${NumThreads}   
         ./frontEnd -d 0 -w ${ALGORITHMS[$algorithm]} -f ${dataDir}$dataset -b ${batchSize} -s $structure -n ${DATASETS[$dataset]} -a $algorithm -t ${NumThreads}   
        else 
         echo ./frontEnd -d 1 -w ${ALGORITHMS[$algorithm]} -f ${dataDir}$dataset -b ${batchSize} -s $structure -n ${DATASETS[$dataset]} -a $algorithm -t ${NumThreads}   
         ./frontEnd -d 1 -w ${ALGORITHMS[$algorithm]} -f ${dataDir}$dataset -b ${batchSize} -s $structure -n ${DATASETS[$dataset]} -a $algorithm -t ${NumThreads}
        fi        
        # make right directory
        # move the two generated files into it  
        #DIRECTORY=/home/abasak/SAGA_data/dynSchedule/${algorithm}/${structure}/${dataset}/Run${runs}  
        DIRECTORY=/home/abasak/SAGA_data_updated/WithoutPref/${algorithm}/${structure}/${dataset}/Run${runs}   
        mkdir -p ${DIRECTORY}
        mv Alg*.csv ${DIRECTORY}/
        mv Update*.csv ${DIRECTORY}/
      done    
    done  
  done 
runs=$(( $runs - 1 ))
done 