// Created by Hongbiao Yang
// Email: hyang22@utk.edu
// For course cosc462 parallel programming

#include "saveWords.h"
#include "parseFlags.h"


void usage (char *progName)
{
printf ( "usage: %s inputFolder books timeFile timing.txt nProcs 2\n", progName );
}


int main(int argc, char** argv)
{

bookType* bt;
int bookCnt, i, j;
char nullString[]="\0";
char* inputFolder = nullString;
char* timeFile = nullString;
char* simMatFile = nullString;
double *simMat;
int nprocs, myrank, ntag = 100;
MPI_Status status;

double timeStart, timeMid, timeEnd, timeElapsed;
double timeParaElapsed, timeSerialElapsed, timeParaEnd,  timeParaStart;
int initSecond;

MPI_Init(&argc, &argv);
struct timeval init_tv;
gettimeofday(&init_tv, (struct timezone*)0);
initSecond = init_tv.tv_sec;

// record the start time
wallTime( &timeStart, &initSecond);

//////////////////////////
// Argument process
commandLineFlagType flag[] =
 {
 {"inputFolder", _string,  &inputFolder },
 {"timeFile",       _string, &timeFile },
 {"simMatFile",      _string, &simMatFile },
 };


// set the number of processes


int numFlags = sizeof ( flag ) / sizeof ( commandLineFlagType );
usageErrorType parseErrorCode = parseArgs ( argc, argv, numFlags, flag ) ;

if ( parseErrorCode == argError  || parseErrorCode == parseError )
{
 usage( argv[0] );
 return ( 1 );
}

// make sure the "inputFolder" contains a "/" in the end
if (inputFolder[strlen(inputFolder) - 1] != '/') 
{
  char* tmpIF = (char*) malloc(strlen(inputFolder)+2);
  strcpy(tmpIF, inputFolder);
  strcat(tmpIF, "/");
  inputFolder = tmpIF;
}

MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

FindTextFiles(inputFolder, &bt, &bookCnt);

ImportFileObject(bt, bookCnt);

// calculat the similarities between books using Cosine Coefficient
simMat = (double*) malloc(bookCnt * bookCnt * sizeof(double));
memset(simMat, 0, bookCnt * bookCnt * sizeof(double));

// parallel part start
wallTime(&timeParaStart, &initSecond);

// 4. calculate the similarity matrix of all the books
CalcSimMatrix(bt, bookCnt, simMat);


// parallel part end
wallTime(&timeParaEnd, &initSecond);

// other nodes send matrix 
if (myrank != 0)
{
  MPI_Send(simMat, bookCnt * bookCnt, MPI_DOUBLE, 0, ntag, MPI_COMM_WORLD);
}

// node 0 recv matrix
if (myrank == 0)
{
  double* recvMat;
  recvMat = (double*) malloc(bookCnt * bookCnt * sizeof(double));
  memset(recvMat, 0, bookCnt * bookCnt * sizeof(double));

  for (i = 1; i < nprocs; i++)
  {
    MPI_Recv(recvMat , bookCnt * bookCnt, MPI_DOUBLE, i, ntag, MPI_COMM_WORLD, &status );

   for (j = 0; j < bookCnt * bookCnt; j++)
   {
	simMat[j] += recvMat[j];
   } 

  }
 for (i = 0; i < bookCnt; i++)
 {
   for (j = 0; j < bookCnt; j++)
   {
	if (i > j)
	{
	   simMat[i* bookCnt + j] = simMat[j * bookCnt + i];
	}
   }
 } 

// 5. Put the result matrix into file with good format (S)
VisualizeMatrix(simMat, bookCnt, simMatFile);

// record the end time
wallTime(&timeEnd, &initSecond);

// calculate the time  elapsed
timeElapsed = timeEnd - timeStart;
timeParaElapsed = timeParaEnd - timeParaStart;
timeSerialElapsed = timeElapsed - timeParaElapsed;
// release memory for book struct and matrix
// releaseBookSpace(bt, bookCnt);
// free(simMat);

// debug, print the time
printf("Compare Time(%d): %1f %1f %1f %1f\n", nprocs,  timeStart, timeParaStart, timeParaEnd, timeEnd);

// save the stat to file
pSaveRunStat(timeStart, timeEnd, timeElapsed, timeSerialElapsed, timeFile);

}


MPI_Finalize ( ) ;

return 0;

}

// 1. read (S)
// find the book files in the folder and 
// store the file name in the structure
errorType FindTextFiles(char* folder, bookType** b, int* cnt)
{
DIR *d;
struct dirent *dir;
int i = 0, n = 0;
char *filename;

d = opendir(folder);
if (d)
{

  while ((dir = readdir(d)) != NULL)
  {
    if (dir->d_type == DT_REG)
    {
      n++;
      // printf("%s len=%d %d\n", dir->d_name, dir->d_reclen, dir->d_type);
    }
  }
}
else
{
  perror("Folder read error");
  return ERROR_READ;
}


closedir(d);
d = opendir(folder);

// read the folder again

(*b) = (bookType*) malloc(sizeof(bookType) * n);
memset(*b, 0, sizeof(bookType) * n);
// assign the file name to the bookType structs
while (i < n)
{
  dir = readdir(d);
  if (dir->d_type != DT_REG)
  { 
   continue;
  }

  filename = dir->d_name;
  
  // (*b)[i].wordList = (wordFreq*)malloc(sizeof(wordFreq));
  (*b)[i].fileName = (char*) malloc(strlen(folder) + strlen(filename)+1);
  sprintf((*b)[i].fileName, "%s%s", folder, filename);
  

  i++;
}

// assign the number of books
*(cnt) = n;

return ERROR_NONE;

}

// 4. calculate the similarity matrix of all the books
errorType CalcSimMatrix(bookType* book, int cnt, double* mat)
{
 int nprocs, myrank;
 int error;

 error = MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
 error = MPI_Comm_rank(MPI_COMM_WORLD, &myrank);


 int i, j;
 
 for (i = 0; i < cnt; i++)
 {
  if (i % nprocs == myrank)
  {
   for (j = 0; j < cnt; j++)
   {

      if (i < j)
      {
        mat[i * cnt + j] = CalcCosSim(book + i, book + j);
      }
      else if (i == j)
      {
	mat[i * cnt + j] = 1.0;
      }
      /*
      else
      {
	mat[i * cnt + j] = mat[j * cnt + i];
      }
      */
   }
  }
 }

 return ERROR_NONE;

}

double CalcCosSim(bookType* b1, bookType* b2)
{
   freqVector* fv;
   int i, j, vSize1, vSize2, wSum1, wSum2, vSize;
   double cosine;

   // number of different words for each book
   vSize1 = b1->wordCnt;
   vSize2 = b2->wordCnt;
   vSize = vSize1 + vSize2;

   // number of words for each book
   wSum1 = b1->wordSum;
   wSum2 = b2->wordSum;

   // Alloceate the common vector using the upper bound as vector size
   fv  = (freqVector*) malloc (vSize * sizeof(freqVector));   
   memset(fv, 0, vSize * sizeof(freqVector));   

   wordFreq *wf1, *wf2;
   wf1 = b1->wordList;
   wf2 = b2->wordList;

   i = 0;   
   while (wf1)
   {
     // put word in book1 into the common vector
     fv[i].word = wf1->word;
     fv[i].x1 = 1.0 * wf1->freq / wSum1;
     
     // search the same word in book2
     while (wf2)
     {
       // find a match, put the word in book2
       if (strcmp(wf2->word, wf1->word) == 0)
       {
	  fv[i].x2 = 1.0 * wf2->freq / wSum2;   
	  wf2->taken = 1;
	  break;
       }

       wf2 = wf2->next;	
     }
     
     // No match found, put 0 into book2
     if (!wf2)
     {
	fv[i].x2 = 0;
     }
     
     // reset wf2
     wf2 = b2->wordList;
   
     // search next
     i++;
     wf1 = wf1->next;
   }

   // book1 is finished, search the rest of book2
   while (wf2)
   {
	// already in the common vector
	if (wf2->taken)
	{
	  wf2 = wf2->next;
	  continue;
	}
	// Those words are in book2 but not in book1, put 0 in book1
	fv[i].x1 = 0;
	fv[i].x2 = 1.0 * wf2->freq / wSum2;
	fv[i].word = wf2->word;

	wf2 = wf2->next;
	i++;
   }

   // Calculate the cosine similarity 
   // cosine(x1, x2) = sum(x1*x2)/sqrt(sum(x1*x1)*sum(x2*x2))
   double x1Sq = 0;
   double x2Sq = 0;
   double x12 = 0;

   for (j = 0; j < i; j++)
   {
	double x1, x2;
	x1 = fv[j].x1;
	x2 = fv[j].x2;
	
	x1Sq += x1*x1;
	x2Sq += x2*x2;
	x12 += x1*x2;
   }

   cosine = x12/sqrt(x1Sq * x2Sq);  

   // release memory
   free(fv);
   
   return cosine;
}


// 5. Put the result matrix into file with good format (S)
errorType VisualizeMatrix(double* mat, int row, char* mFile)
{
  int i, j, rs, error;
  int nprocs, myrank;
  size_t sz;
  FILE* fp;
  char matFile[FILE_LEN] = {0};
  char matrix[MAX_LEN] = {0};

  for (i = 0; i < row * row; i++)
  {
    if (i % row == 0)
    {
	strcat(matrix, "\n"); 
    }

    sprintf(matrix, "%s%.3f\t", matrix, mat[i]);
  }
 
  error = MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  error = MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

  sprintf(matFile, "%d-%s", nprocs, mFile);
  fp = fopen(matFile, "w+");  
  if (fp == NULL)
  {
      perror("fopen");
      return ERROR_OPEN;
  }

  sz = fwrite(matrix, 1, strlen(matrix), fp);
  if (sz <= 0) 
  {
	perror("fwrite");
	return ERROR_WRITE;
  }
  
  rs = fclose(fp);
  if (rs != 0)
  {
      perror("fclose");
      return ERROR_CLOSE;
  }


  return ERROR_NONE;
}


double wallTime(double *t, int *initSec)
{
double time;
int sec;
struct timeval tv;
gettimeofday(&tv, (struct timezone *)0);
sec = tv.tv_sec;
*t = (tv.tv_sec - *initSec) + 1.0e-6*tv.tv_usec;
time = *t;
return (time);
}


errorType releaseBookSpace(bookType* book, int cnt)
{
  int i;
  wordFreq *wf, *old;

  for (i = 0; i < cnt; i++)
  {
     wf = book[i].wordList;
     
     // release the word one by one
     while (wf)
     {
	old = wf;
	wf = wf->next;

	// free the word value, then free the node
	free(old->word);
	free(old);
     }
     
     // free the filename string
     free(book[i].fileName);
  }
  
  // free the book list
  free(book);

  return ERROR_NONE;
}


errorType pSaveRunStat(double start, double end, double elapsed, double serialElapsed, char* tFile)
{
  FILE *fp;
  int rs;
  char statLog[MAX_LEN] = {0};
  int nprocs, myrank, error;

  error = MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  error = MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

  fp = fopen(tFile, "a+");
  if (fp == NULL)
  {
      perror("fopen");
      return ERROR_OPEN;
  }

  sprintf(statLog, "numProcesses  %02d start %1f end %1f timeElapsed %1f serialElapsed %1f\n", nprocs, start, end, elapsed, serialElapsed);
  // sprintf(statLog, "start %1f end %1f timeElapsed %1f\n", start, end, elapsed);

  rs = fwrite(statLog, 1, strlen(statLog), fp);
  if (rs <= 0)
  {
   perror("Error Write");
   return ERROR_WRITE;
  }

  rs = fclose(fp);
  if (rs != 0)
  {
      perror("fclose");
      return ERROR_CLOSE;
  }


  return ERROR_NONE;
}  


errorType ImportFileObject(bookType* b, int cnt)
{
 int i, rs, count;
 FILE* fp = NULL;
 char buf[MAX_LEN] = {0};
 char word[FILE_LEN] = {0};
 wordFreq *wl = NULL, *newWord = NULL;

  for (i = 0; i < cnt; i++)
  {
    fp = fopen(b[i].fileName, "r");
    if (fp == NULL)
    {
      perror("fopen");
      return ERROR_OPEN;
    }
    
    b[i].wordCnt = 0;
    b[i].wordSum = 0; 
    b[i].wordList = NULL;

    wl = (wordFreq*) malloc(sizeof(wordFreq));
    memset(wl, 0, sizeof(wordFreq));   

    while (fgets(buf, MAX_LEN, fp) != NULL)
    {
      sscanf(buf, "%s %d", word, &count);     	
      
      if (b[i].wordList == NULL)
      {
	 wl = (wordFreq*) malloc(sizeof(wordFreq));
         memset(wl, 0, sizeof(wordFreq));
         wl->word = (char*)malloc(strlen(word) + 1);
	 strcpy(wl->word, word);
	 wl->freq = count;
	 b[i].wordCnt = 1;
	 b[i].wordSum = count;
	 b[i].wordList = wl;
      }
      else
      {
	 newWord = (wordFreq*) malloc(sizeof(wordFreq));
         memset(newWord, 0, sizeof(wordFreq));
         newWord->word = (char*)malloc(strlen(word) + 1);
         strcpy(newWord->word, word);
         newWord->freq = count;
	 wl->next = newWord;
	 wl = wl->next;
	 b[i].wordCnt ++;
	 b[i].wordSum += count;
      }      	 
    }
 
      rs = fclose(fp);
      if (rs != 0)
      {
        perror("fclose");
        return ERROR_CLOSE;
      }
  }

  return ERROR_NONE;

}
