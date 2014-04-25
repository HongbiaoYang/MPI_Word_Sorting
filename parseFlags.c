#include "parseFlags.h"

usageErrorType
parseArgs ( int argc, char **argv, int numFlags, commandLineFlagType *flag )
 {
 int argNum, keywordNum, valueNum;
 int flagNum;
 int flagMatched = 0;

 /* examine all the command line arguments */
 for ( argNum = 1; argNum < argc; argNum++ )  /* argNum = 1 for 1st flag */
   {
   /* fprintf ( stderr, "%d %s\n", argNum, argv[argNum] ); */
   keywordNum = argNum;
   valueNum   = argNum+1;
   if ( valueNum >= argc )
     {
     fprintf ( stderr, "value missing after keyword %s\n", argv[keywordNum] );
     return ( argError );
     }
   argNum++;  /* advance beyond the value after the keyword */

   /* compare argument against all the defined command line flags */
   for ( flagNum = 0; flagNum < numFlags; flagNum++ )
     {
     if ( strcmp ( argv[keywordNum], flag[flagNum].label ) == 0 )
       {
       flagMatched = 1;
       switch ( flag[flagNum].ioType)
         {
         case _int:
           sscanf( argv[valueNum], "%d", (int *) flag[flagNum].dst);
           break;
         case _long:
           sscanf( argv[valueNum], "%ld", (long *) flag[flagNum].dst);
           break;
         case _float:
           sscanf( argv[valueNum], "%f", (float *) flag[flagNum].dst);
           break;
         case _double:
           sscanf( argv[valueNum], "%lf", (double *) flag[flagNum].dst);
           break;
         case _string:
            *((char **) (flag[flagNum].dst)) = argv[valueNum];
            break;
         case _FILE:
            break;
         }
       }
     }
   if ( !flagMatched )
     {
     fprintf ( stderr, "unrecognized flag: %s\n", argv[keywordNum] );
     return ( argError );
     }
   }
 return ( noError );
 }


