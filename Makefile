CCC      = mpicxx
CCLINKER = mpicxx

all:saveWords compareWords
saveWords:saveWords.o parseFlags.o
	${CCLINKER} saveWords.o parseFlags.o -g  -o saveWords -lm 
compareWords:compareWords.o parseFlags.o
	$(CCLINKER) compareWords.o parseFlags.o -g -o compareWords -lm
saveWords.o:saveWords.c saveWords.h parseFlags.h
	${CCC} saveWords.c -c -g -I. 
compareWords.o:compareWords.c saveWords.h parseFlags.h
	${CCC} compareWords.c -c -g -I. 
parseFlags.o:parseFlags.c parseFlags.h
	$(CCC) -c parseFlags.c
test:clean saveWords
	mpirun -n 4  -hostfile cluster.txt saveWords  inputFolder books/ timeFile timing.txt outputFile data.txt   
test2: clean compareWords
	mpirun -n 4  compareWords  inputFolder words/ timeFile timing2.txt simMatFile data.txt   
clean:
	rm -rf *.o saveWords compareWords timing.txt timing2.txt *data.txt 
	rm -rf words/*
release:
	cd ..; tar cvfz saveWordsProj.tgz finalProject/; mv saveWordsProj.tgz finalProject/
