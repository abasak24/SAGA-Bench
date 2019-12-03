# Run LiveJournal 
#./frontEnd -d 1 -w 0 -f /home/abasak/graph_project/graphs/snap/soc-LiveJournal1.shuffle.t.w.csv -b 500000 -s adListShared -n 4847571 -a bfsdyn -t 46 

#DIRECTORY=/home/abasak/test/LJASDynamic
#mkdir -p ${DIRECTORY}/
#mv Alg*.csv ${DIRECTORY}/
#mv Update*.csv ${DIRECTORY}/

# Run Orkut 
#./frontEnd -d 0 -w 0 -f /home/abasak/graph_project/graphs/snap/com-orkut.ungraph.shuffle.t.w.csv -b 500000 -s adListShared -n 3072441 -a bfsdyn -t 46
#DIRECTORY=/home/abasak/test/OrkutASDynamic
#mkdir -p ${DIRECTORY}/
#mv Alg*.csv ${DIRECTORY}/
#mv Update*.csv ${DIRECTORY}/

# Run Wiki-Topcats 
./frontEnd -d 1 -w 0 -f /home/abasak/graph_project/graphs/snap/wiki-topcats.shuffle.t.w.csv -b 500000 -s adListShared -n 1791489 -a bfsdyn -t 46

rm Alg*.csv 
rm Update*.csv 

# Run rmat 
#./frontEnd -d 1 -w 0 -f /home/abasak/graph_project/graphs/snap/rmat.csv -b 500000 -s adListShared -n 33554432 -a bfsdyn -t 46
#DIRECTORY=/home/abasak/test/RMATASDynamic
#mkdir -p ${DIRECTORY}/
#mv Alg*.csv ${DIRECTORY}/
#mv Update*.csv ${DIRECTORY}/

# Run Pokec
#./frontEnd -d 1 -w 0 -f /home/abasak/graph_project/graphs/soc-pokec-relationships.shuffle.t.w.csv -b 500000 -s adListChunked -n 1632803 -a prdyn -t 40

# Run Twitter
#./frontEnd -d 1 -w 0 -f /home/abasak/graph_project/graphs/snap/twitter_rvshuffle.t.w.csv -b 500000 -s adList -n 61578415 -a prdyn -t 46

#Run test 
#./frontEnd -d 1 -w 1 -f ./test.csv -b 10 -t 24 -s adListChunked -n 40 -a traverse 
