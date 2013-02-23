/* bits.c, bit-level input */

#include "bits.h"

#define BUFFER_SIZE 1024*1024

/* private data */
static unsigned char bfr[BUFFER_SIZE];
static int byteidx;
static int bitidx;
static int bufcount;
static double totbits;
FILE *bitfile;
bool eobs;

/* return total number of generated bits */
double bitcount()
{
  return totbits;
}

static bool refill_buffer()
{
  int i;

  i = fread(bfr, sizeof(unsigned char), BUFFER_SIZE, bitfile);
  if (i <= 0)
  {
    eobs = true;
    return false;
  }
  bufcount = i;
  return true;
}

/* open the device to read the bit stream from it */
void init_getbits(char *bs_filename)
{
  if ((bitfile = fopen(bs_filename, "rb")) == NULL)
  {
    printf("Unable to open file %s for reading.\n", bs_filename);
    exit(1);
  }
  byteidx = 0;
  bitidx = 8;
  totbits = 0.0;
  bufcount = 0;
  eobs = false;
  if (!refill_buffer())
  {
    if (eobs)
    {
      printf("Unable to read from file %s.\n", bs_filename);
      exit(1);
    }
  }
}

/*close the device containing the bit stream after a read process*/
void finish_getbits()
{
  if (bitfile)
    fclose(bitfile);
  bitfile = NULL;
}

int masks[8]={0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80};

/*read 1 bit from the bit stream */
unsigned int get1bit()
{
  unsigned int bit;

  if (eobs)
    return 0;

  bit = (bfr[byteidx] & masks[bitidx - 1]) >> (bitidx - 1);
  totbits++;
  bitidx--;
  if (!bitidx)
  {
    bitidx = 8;
    byteidx++;
    if (byteidx == bufcount)
    {
      if (bufcount == BUFFER_SIZE)
        refill_buffer();
      else
        eobs = true;
      byteidx = 0;
    }
  }
  return bit;
}

/*read N bit from the bit stream */
unsigned int getbits(int N)
{
 unsigned int val = 0;
 int i = N;
 unsigned int j;

 // Optimize: we are on byte boundary and want to read multiple of bytes!
 if ((bitidx == 8) && ((N & 7) == 0))
 {
   i = N >> 3;
   while (i > 0)
   {
     val = (val << 8) | bfr[byteidx];
     byteidx++;
     totbits += 8;
     if (byteidx == bufcount)
     {
       if (bufcount == BUFFER_SIZE)
         refill_buffer();
       else
         eobs = true;
       byteidx = 0;
     }
     i--;
   }
   return val;
 }

 while (i > 0)
 {
   j = get1bit();
   val = (val << 1) | j;
   i--;
 }
 return val;
}

/*return the status of the bit stream*/
/* returns 1 if end of bit stream was reached */
/* returns 0 if end of bit stream was not reached */

int end_bs()
{
  return eobs;
}

/*this function seeks for a byte aligned sync word (max 32 bits) in the bit stream and
  places the bit stream pointer right after the sync.
  This function returns 1 if the sync was found otherwise it returns 0  */

int seek_sync(unsigned int sync, int N)
{
  unsigned int val;
  unsigned int maxi = (int)pow(2.0, (double)N) - 1;

  while (bitidx != 8)
    get1bit();

  val = getbits(N);

  while ((val & maxi) != sync)
  {
    if (eobs)
      return 0;
    val <<= 8;
    val |= getbits(8);
  }
  return 1;
}

/*look ahead for the next N bits from the bit stream */
unsigned int look_ahead(int N)
{
  unsigned long val = 0;
  unsigned char *tmp_bfr = bfr;
  unsigned char tmp_bfr1[4];
  int j = N;
  int eo_bs = eobs;
  int buf_count = bufcount;
  int bit_idx = bitidx;
  int byte_idx = byteidx;

  while (j > 0)
  {
    if (eo_bs)
      return 0;
    val <<= 1;
    val |= (tmp_bfr[byte_idx] & masks[bit_idx - 1]) >> (bit_idx - 1);
    bit_idx--;
    j--;
    if (!bit_idx)
    {
      bit_idx = 8;
      byte_idx++;
      if (byte_idx == buf_count)
      {
        if (buf_count == BUFFER_SIZE)
        {
          if (fread(tmp_bfr1, sizeof(unsigned char), 4, bitfile) != 4)
            eo_bs = true;
          else
            tmp_bfr = &tmp_bfr1[0];
          if (fseek(bitfile, -4, SEEK_CUR))
          {
            printf("Unable to set file position.\n");
            exit(1);
          }
        }
        else
          eo_bs = true;
        byte_idx = 0;
      }
    }
  }
  return val;
}
