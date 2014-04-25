// st(wordFreq* wordList)Created by Hongbiao Yang
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
int bookCnt = 0, i;
char nullString[]="\0";
char inputFile[FILE_LEN] = {0};
char* inputFolder = nullString;
char* timeFile = nullString;
char* outputFile = nullString;
double* simMat;

int nprocs, myrank, error;



double timeStart, timeMid, timeEnd, timeElapsed;
double timeParaElapsed, timeSerialElapsed, timeParaEnd, timeParaStart;
int initSecond;

int ntag = 100;
MPI_Status status;


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
 {"outputFile",      _string, &outputFile },
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

error = MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
error = MPI_Comm_rank(MPI_COMM_WORLD, &myrank);


  wallTime( &timeParaStart, &initSecond);
  // word load shared among nprocs nodes, all nodes work parallelly
  struct dirent *dir;
  char *filename;
  DIR* d;

  d = opendir(inputFolder);
  if (d)
  {

  while ((dir = readdir(d)) != NULL)
  {
    if (dir->d_type == DT_REG)
    {
      bookCnt ++;
      i = bookCnt % nprocs;
      if (i == myrank)
      {
        strcpy(inputFile, dir->d_name);
	
	 bt = (bookType*) malloc(sizeof(bookType));
	 memset(bt, 0, sizeof(bookType));

	 bt->fileName = (char*) malloc(strlen(inputFolder) + strlen(inputFile) + 1);
	 strcpy(bt->fileName, inputFolder);
	 strcat(bt->fileName, inputFile);


	 // 2. fill the structs with the desired content (P)
	 CreateFileObjects(bt);

	 // 3. sort the word list according to the word frequency (P)
	 SortWordForBook(bt);
	 // DebugPrintList(bt->wordList);
	 RemoveStopWords(bt);


	char wordFile[FILE_LEN] = {0};	
	sprintf(wordFile, "words/%s.dat", inputFile);
	DebugPrintList(bt->wordList, wordFile);
      }
      // MPI_Send(inputFile, FILE_LEN, MPI_CHAR, i,  ntag, MPI_COMM_WORLD);
      // printf("Node %d sent %s to %d\n", myrank, inputFile, i);
    }
  }
 }
 else
 {
  perror("Folder read error");
  return ERROR_READ;
 }
 closedir(d); 

 wallTime( &timeParaEnd, &initSecond);

 printf("rank=%d np=%d\n", myrank, nprocs);

  // tell node 0 that the work is done
  if (myrank != 0)
  {
	MPI_Send(&myrank, 1, MPI_INT, 0, ntag, MPI_COMM_WORLD);
  }

  if (myrank == 0)
  {
   int numSum = 0;
   int num = 0;

   // wait until all other nodes report
   for (i = 1; i < nprocs; i++)
   {
	MPI_Recv(&num, 1, MPI_INT, i, ntag, MPI_COMM_WORLD, &status);
	numSum += num;
   }
   
   printf("numSum=%d nprocs=%d\n", numSum, nprocs); 

   if (numSum != (nprocs - 1)*(nprocs)/2)
   {
	printf("Some node didn't report\n");
	return 1; 
   }

   wallTime( &timeEnd, &initSecond);

   timeElapsed = timeEnd - timeStart;
   timeParaElapsed = timeParaEnd - timeParaStart;
   timeSerialElapsed = timeElapsed - timeParaElapsed;

   printf("SaveTime(%d): %1f %1f %1f %1f \n", nprocs,  timeStart, timeParaStart, timeParaEnd, timeEnd);

   pSaveRunStat(timeStart, timeEnd, timeElapsed, timeSerialElapsed, timeFile);
  }	

 // all  nodes receive file name 
 // MPI_Recv(inputFile, FILE_LEN, MPI_CHAR, 0, ntag, MPI_COMM_WORLD, &status);

 // printf("Node  %d recv %s (%d)\n", myrank,  inputFile, i);
MPI_Finalize () ; 


return 0;

}



// 2. fill the structs with the desired content (P)
errorType CreateFileObjects(bookType* b)
{
  int i, rs;
  FILE* fp = NULL;
  char buf[MAX_LEN] = {0};
  
    fp = fopen(b->fileName, "r");
    if (fp == NULL)
    {
      perror("fopen");
      return ERROR_OPEN;
    }  
    
    while (fgets(buf, MAX_LEN, fp) != NULL) 
    {
      ConvertLowerFilter(buf);
      CountWordFreq(buf, b);
      
      // printf("%s", buf);
    }
    rs = fclose(fp);
    if (rs != 0)
    {
      perror("fclose");
      return ERROR_CLOSE;
    }
  
  return ERROR_NONE;
}

errorType CountWordFreq(char* line, bookType* book)
{
 int i;
 char* pch;
 char delim[] = " ,.-\"'\n\t"; 

 pch = strtok (line, delim); 
 while (pch != NULL)
 {
    InsertWord(book, pch);
    pch = strtok (NULL, delim);
 }

 return ERROR_NONE;
}


errorType ConvertLowerFilter(char* letters)
{
  char* lower;
  lower = letters;
  int j;

  for(j = 0; lower[j]; j++)
  { 
     // convert each letter to lower case, so the capital 
     // will not be a problem 
     lower[j] = tolower(lower[j]);
     
     // filter the special characters
     if (!((lower[j] >= 'a' && lower[j] <= 'z') ||
           (lower[j] >= '0' && lower[j] <= '9')))
     {
	lower[j] = '.';
     }
          
  }
  return ERROR_NONE;
}

errorType InsertWord(bookType* book, char* word)
{
  // get the word list pointer
  wordFreq* wf = book->wordList;
  wordFreq *curWord, *newWord, *lastWord;  
  
  int wordLen;
  
  // w
  // if the word length is less than WORD_LEN, assign WORD_LEN 
  // space, if exceed the WORD_LEN, add another WORD_LEN
  wordLen = ((strlen(word)+1) / WORD_LEN + 1) * WORD_LEN;


  // this is the first time the word list pointer 
  // is accessed, initialize the wordFreq object.
  // This block will only be called once
  if (wf == NULL)
  {  
     wf = (wordFreq*)malloc(sizeof(wordFreq));
     wf->word = (char*)malloc(wordLen);
     strcpy(wf->word, word);
     wf->freq = 1;
     wf->next = NULL; 
     
     book->wordList = wf;     

     book->wordCnt = 1;
     book->wordSum = 1;
     
     return ERROR_NONE;
  }
  else
  {
    curWord = wf;
    while (curWord != NULL)
    {
      if (strcmp(word, curWord->word) == 0)
      { 
 	// find a match, increase the frequency count and return 
	curWord->freq++;
	book->wordSum++;  	

        return ERROR_NONE;
      }
      else
      { 
	lastWord = curWord;
	curWord = curWord->next;
      }
    }
   
   // Can't find a match, create a new node in the end of the lista
   // Here the pointer curWord should be NULL
   newWord = (wordFreq* )malloc(sizeof(wordFreq));
   newWord->word = (char*)malloc(wordLen);
   strcpy(newWord->word, word);
   newWord->next = NULL;   
   newWord->freq = 1;

   lastWord->next = newWord;
    
   
   book->wordCnt++;
   book->wordSum++;	
   
   return ERROR_NONE;
   } 
}


// 3. sort the word list according to the word frequency (P)
errorType SortWordForBook(bookType* book)
{
 wordFreq* wf = NULL;

   wf = book->wordList;
   QuickSort(wf, NULL);

   return ERROR_NONE;
}


errorType QuickSort(wordFreq* pBeign, wordFreq* pEnd)
{
	if(pBeign != pEnd)
	{
		wordFreq* partion = GetPartion(pBeign,pEnd);
		QuickSort(pBeign, partion);
		QuickSort(partion->next,pEnd);
	}
	return ERROR_NONE;
}

wordFreq* GetPartion(wordFreq* pBegin, wordFreq* pEnd)
{
	int key = pBegin->freq;
	wordFreq* p = pBegin;
	wordFreq* q = p->next;

	while(q != pEnd)
	{
		if(q->freq > key)
		{
			p = p->next;
			Swap(p, q);
		}

		q = q->next;
	}
	Swap(p, pBegin);
	return p;
}

errorType Swap(wordFreq* a, wordFreq* b)
{
  int freq;
  char *word;
  
  // Swap the value of two word
  // Both frequency and word pointer
  freq = a->freq;
  word = a->word;
  
  a->freq = b->freq;
  a->word = b->word;
 
  b->freq = freq;
  b->word = word;
 
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


errorType pSaveRunStat(double start, double end, double elapsed, double serialElapsed,  char* tFile)
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

  sprintf(statLog, "numProcesses %02d start %1f end %1f timeElapsed %1f serialElapsed %1f\n", nprocs,  start, end, elapsed, serialElapsed);
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

errorType DebugPrintList(wordFreq* wordList, char* wordFile)
{
  wordFreq* wlist = wordList;
  int rs, sz, wordCnt = 0;
  FILE* fp = NULL;   
  char buf[MAX_LEN] = {0};

  fp = fopen(wordFile, "w+");
  if (fp == NULL)
  {
      perror("fopen");
      return ERROR_OPEN;
  }

  while (wlist)
  {
    sprintf(buf, "%s %d\n", wlist->word, wlist->freq);
    wlist = wlist->next;
    wordCnt++;
    sz = fwrite(buf, 1, strlen(buf), fp); 
    if (sz <= 0)
    {
        perror("fwrite");
        return ERROR_WRITE;
    }
  
  }
  
  rs = fclose(fp);
  if (rs != 0)
  {
      perror("fclose");
      return ERROR_CLOSE;
  }


   printf("\n[%d]", wordCnt);
   return ERROR_NONE;
}


// remove the top 3% words that is too frequent and meaningless
errorType RemoveStopWords(bookType* bt)
{
  int i, stopCount;

  stopCount = STOP_RATE * bt->wordCnt;
  wordFreq *wl = NULL, *stopWord = NULL;
  wl = bt->wordList;  

  // delete 'stopCount' words from the list
  for (i = 0; i < stopCount; i++)
  {
	stopWord = wl;
	wl = wl->next;
	
	// release removed word resource
	free(stopWord->word);
	free(stopWord);
  }
  
  bt->wordList = wl;

  return ERROR_NONE;
}



