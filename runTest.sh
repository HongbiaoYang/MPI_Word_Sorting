#!/bin/bash

# compile
make clean
make

# other nodes except self
otherNodes=`sed "s/terbium//g" cluster.txt`
 
# prepare the files and folders
for node in  $otherNodes
do

ssh $node "mkdir -p  ~/cosc462/projMPI/"

# send executable
ssh $node "rm -rf ~/cosc462/projMPI/*"

ssh $node "mkdir ~/cosc462/projMPI/words/"

scp saveWords compareWords $node:~/cosc462/projMPI/

scp -r books/ $node:~/cosc462/projMPI/

done

# run the project using 1-10 cores
for i in  1 2 3 4 5 6 7 8 9 10
do 
 echo "Running the program using $i core(s)"
 
 mpirun -n $i  -hostfile cluster.txt saveWords  inputFolder books/ timeFile timing.txt outputFile data.txt

 # copy the words file
 for node in $otherNodes
 do
   scp -r  $node:~/cosc462/projMPI/words/ .

 done 

 for node in $otherNodes
 do
   scp -r  words/* $node:~/cosc462/projMPI/words/
 done


 # run the compareWords program parallely
 mpirun -n $i -hostfile cluster.txt  compareWords  inputFolder words/ timeFile timing2.txt simMatFile data.txt
 
 # clean the words file for the next run case
 for node in $otherNodes
 do
   ssh $node "rm -rf ~/cosc462/projMPI/words/*"
 done

done


