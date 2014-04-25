By Hongbiao Yang
For course COSC 462 Parallel Programming
#################################################
## Notice: This program is tested on WYSE      ##
## cluster using OpenMPI, so it can't run      ## 
## directly on newton. You need to configure   ##
## the environment and cluster.txt first       ##
## before running the program                  ##
################################################

There are two main programs in this project, to repeat the results as mine, the two program need to run one by one, with some preparation work and data transfer in between. All these are well recorded in the script. One can simply type the command below:

sh runTest.sh

After type the command above, everything will run automatically, what you need to do is just wait until it finishes, and you will get the same result as mine. 
I run the code on WYSE cluster using 4 machines, which are in the file ‘cluster.txt’. I run my experiment on the main node ‘terbium’, so if you are running this project on some other environments, you might need to first configure the machine names correctly, and make sure you have enough privilege on all your nodes. Also, please make sure the network between the main node and the rest of the nodes are secure, and logging in without typing password is also guaranteed. 

