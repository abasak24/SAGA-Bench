#!/bin/bash

# Get the necessary inputs from the user
if [ $# -eq 3 ] 
then	# All arguments are provided
	inputFileName=$1
	if [ $2 -gt 0 ] && [ $2 -lt 4 ]
	then
		option=$2
	else
		echo "Invalid option - selecting default option (1)"
		option=1
	fi
	if [ $3 -gt 0 ]
	then
		maxWeight=$3
	else 
		echo "Invalid maximum weight - the weights must be a non-zero positive integer"
		exit 1
	fi
else 	# All arguments are not provided
	echo ""
	echo "Not all arguments are provided. The program requires 3 arguments:"
	echo "./addWeightAndTime.sh [input filename] [value option] [max weight value > 0]"
	echo "	Value Option 1 - Add weights only"
	echo "	Value Option 2 - Add timestamps only"
	echo "	Value Option 3 - Add both weights and timestamps"
	echo ""
	echo "Exiting the program..."
	exit 1
fi
echo ""

# Use the Python program to add the desired values to the input edgelist file
python -u src/appendValues.py ${inputFileName} ${option} ${maxWeight}
echo ""
echo "DONE"
