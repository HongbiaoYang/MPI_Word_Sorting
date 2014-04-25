#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum
 {
 noError,
 parseError,
 argError,
 ioError
 }
usageErrorType;

typedef enum
 {
 _int,
 _long,
 _float,
 _double,
 _string,
 _FILE
 }
ioType1;

typedef struct
 {
 char    label[1000];
 ioType1  ioType;
 void   *dst;
 }
commandLineFlagType;

usageErrorType
parseArgs ( int argc, char **argv, int numFlags, commandLineFlagType *flag );


