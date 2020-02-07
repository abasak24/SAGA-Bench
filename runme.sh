#dataDir=/home/abanti/datasets/SAGAdatasets/

# Run LiveJournal 
#./frontEnd -d 1 -w 0 -f ${dataDir}soc-LiveJournal1.shuffle.t.w.csv -b 500000 -s adListShared -n 4847571 -a bfsdyn -t 64 

# Run Orkut 
#./frontEnd -d 0 -w 0 -f ${dataDir}com-orkut.ungraph.shuffle.t.w.csv -b 500000 -s stinger -n 3072441 -a bfsdyn -t 64

# Run Wiki-Topcats 
#./frontEnd -d 1 -w 0 -f ${dataDir}wiki-topcats.shuffle.t.w.csv -b 500000 -s degAwareRHH -n 1791489 -a bfsdyn -t 64

# Run rmat 
#./frontEnd -d 1 -w 0 -f ${dataDir}rmat.csv -b 500000 -s adListChunked -n 33554432 -a bfsdyn -t 64

# Run Wiki-Talk
#./frontEnd -d 1 -w 0 -f ${dataDir}wiki-talk-pure.shuffle.t.w.csv -b 500000 -s adListChunked -n 2394385 -a bfsdyn -t 64

#Run test.csv on algorithm "traverse". "Traverse" is a single-threaded micro-kernel that reads the neighbors of all vertices in the graph
./frontEnd -d 1 -w 1 -f ./test.csv -b 10 -t 24 -s stinger -n 40 -a prdyn