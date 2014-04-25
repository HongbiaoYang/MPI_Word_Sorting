#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <math.h>
#include <mpi.h>
// #include <omp.h>
#include <sys/time.h>


#define WORD_LEN 10
#define FILE_LEN 128
#define MAX_LEN  1024
#define STOP_RATE 0.02


typedef enum 
{
  MEMORY_ERROR,
  ERROR_NONE,
  ERROR_DIVIDE_BY_ZERO,
  ERROR_OPEN,
  ERROR_CLOSE,
  ERROR_WRITE,
  ERROR_READ,
  ERROR_OTHER,
} errorType;


typedef struct _wordFreq
{
char* word;
int freq;
int taken;

struct _wordFreq* next;
} wordFreq;


typedef struct
{
char* fileName;
wordFreq* wordList;
int wordCnt;
int wordSum;

} bookType;

typedef struct 
{
  double x1;
  double x2;
  char* word;
} freqVector;


errorType CreateFileObjects(bookType* b);
errorType CountWordFreq(char* line, bookType* book);
errorType ConvertLowerFilter(char* letters);
errorType InsertWord(bookType* book, char* word);
errorType SortWordForBook(bookType* book);
errorType QuickSort(wordFreq* pBeign, wordFreq* pEnd);
wordFreq* GetPartion(wordFreq* pBegin, wordFreq* pEnd);
errorType Swap(wordFreq* a, wordFreq* b);
errorType DebugPrintList(wordFreq* wordList, char* wordFile);
double wallTime(double *t, int *initSec);
errorType releaseBookSpace(bookType* book, int cnt);
errorType pSaveRunStat(double start, double end, double elapsed, double paraElapsed, char* tFile);
errorType ImportFileObject(bookType* b, int cnt);
errorType FindTextFiles(char* folder, bookType** b, int* cnt);
errorType CalcSimMatrix(bookType* book, int cnt, double* mat);
double CalcCosSim(bookType* b1, bookType* b2);
errorType VisualizeMatrix(double* mat, int row, char* mFile);
errorType RemoveStopWords(bookType* bt);
