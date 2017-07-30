#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>
#include <string.h>

/* basic structure for a word */
typedef struct Twor     TWOR;

struct Twor
{
  TWOR *next;
  char  roten[24];
  char  ending[32];
  char  describe[968]; // This will be changed!
};

int listsize;

#define WOR_SIZE 1024

FILE *worhandler, *srthandler;

TWOR *worget( int   wornum );
void  worset( TWOR *wor );
void  free_wor( TWOR *wor );
TWOR *new_wor();

