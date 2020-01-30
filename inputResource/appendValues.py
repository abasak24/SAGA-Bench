# Python program to append weights and/or timestamps to datasets in edgelist format

import random
import sys

# Get the input arguments for the program
inputFileName = sys.argv[1]
outputFileName = inputFileName[:-4]
option = int(sys.argv[2])
maxWeight = int(sys.argv[3])

# Get the correct suffix for the output for correct identification of the graphs
if option == 1:
	print ("Appending only weights to the graph")
	outputFileName += ".w.txt"
elif option == 2:
	print ("Appending only timestamps to the graph")
	outputFileName += ".t.txt"
else:
	print ("Appending both weights and timestamps to the graph")
	outputFileName += ".t.w.csv"

# Open both the input and output graphs
fin = open(inputFileName, "r")
fout = open(outputFileName, "w")

# Iterate through each line in the inupt graph, append the necessary values, and write it out to 
# the output file 
time = 0
for line in fin:
	# Don't modify the comment lines (which begin with a '#')
	# skip the comment lines altogether
	if line[0] == "#":
		#fout.write(line.strip() + "\n")
		continue

	modifiedLine = line.strip() 
	modifiedLine += ("\t")
	modifiedLine = " ".join(line.split())	
	
	modifiedLine += ','    
	if option == 2 or option == 3:   
		modifiedLine += str(time)      
	else:                              
		modifiedLine += str(int(random.random() * maxWeight) + 1)   
	
	if option == 3:               
		modifiedLine += (',' + str(int(random.random() * maxWeight) + 1))  
	
	modifiedLine = modifiedLine.replace(" ", ",") 
 
	#if option == 1 or option == 3:
		#modifiedLine += str(int(random.random() * maxWeight) + 1)
	#else:
		#modifiedLine += str(time)
	
	#if option == 3:
		#modifiedLine += ("\t" + str(time))

	modifiedLine += "\n"
	fout.write(modifiedLine)

	time += 1

# Close both the input and output files
fin.close()
fout.close()