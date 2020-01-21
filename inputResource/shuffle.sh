#!/bin/bash

: '
This tool is based on the suggestion for shuffling edges in the Cassovary vs. GraphJet Github
repository (https://github.com/lintool/Cassovary-vs-GraphJet). It takes a graph edgelist in 
plain-text format (.txt) and shuffles them randomly. This tool is needed because a lot of our
graphs in the repository have their edges sorted numerically, which will not be representative
of real-life evolving graphs when being streamed. The format of the command to run is as follows:

$ ./shuffle.sh [input file]

This tool has only one required parameter, whic is the path+name of the input graph. After the
shuffling, the output graph will be written to the same directory as the input graph. In addition,
the shuffled graph will have the suffix xxxx.shuffle.txt appended to it. 
'


# Get the input filename from the user
if [ $# -eq 1 ] 
then 
	inputFileName=$1
else
	echo "Input filename not provided."
	echo "Exiting program ..."
	exit 1
fi
echo ""

# Shuffle the contents of the edgelist and write them out to a file (with suffix .shuffle.txt)
outputFileName="${inputFileName%.*}.shuffle.txt"
cat ${inputFileName} | awk 'BEGIN{srand();}{print rand()"\t"$0}' | sort -k1 -g -T /home/abanti/sortDump/ | cut -f2- > ${outputFileName}

echo "SHUFFLING COMPLETE"
