//----------------------------------------------------------------------------
//   bits header
//----------------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

void init_getbits(char *bs_filename);
void finish_getbits();
unsigned int get1bit();
unsigned int getbits(int N);
double bitcount();
int end_bs();
int seek_sync(unsigned int sync, int N);
unsigned int look_ahead(int N);


