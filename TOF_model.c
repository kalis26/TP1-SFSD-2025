/**********************************************************
 * Implementation of TOF module functions (TOF_model.c)   *
 * SFSD (File & Data Structures) / 2CP / ESI / 2024       *
 **********************************************************/

#include "TOF_model.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXTAB 10

// open a TOF file / mode ='N' for a New file and mode ='E' for an Existing file
// returns a pointer to a newly allocated variable of type 'TOFIndex'
void TI_open(TOFIndex **F, char *fname, char mode)
{
   *F = malloc(sizeof(TOFIndex));
   if (mode == 'E' || mode == 'e')
   {
      // openning an existing TOF file ...
      (*F)->f = fopen(fname, "rb+");
      if ((*F)->f == NULL)
      {
         perror("TOF_open");
         exit(EXIT_FAILURE);
      }
      // loading header part in main memory (in (*F)->h)
      fread(&((*F)->h), sizeof(THeaderIndex), 1, (*F)->f);
   }
   else
   {  // mode == 'N' || mode == 'n'
      // creating a new TOF file ...
      (*F)->f = fopen(fname, "wb+");
      if ((*F)->f == NULL)
      {
         perror("TOF_open");
         exit(EXIT_FAILURE);
      }
      // initializing the header part in main memory (in (*)->h)
      (*F)->h.nBlock = 0;
      (*F)->h.nIns = 0;
      (*F)->h.nDel = 0;
      fwrite(&((*F)->h), sizeof(THeaderIndex), 1, (*F)->f);
   }
}

// close a TOF file :
// the header is first saved at the beginning of the file and the TOFIndex variable is freed
void TI_close(TOFIndex *F)
{
   // saving header part in secondary memory (at the begining of the stream F->f)
   fseek(F->f, 0L, SEEK_SET);
   fwrite(&F->h, sizeof(THeaderIndex), 1, F->f);
   fclose(F->f);
   free(F);
}

// reading data block number i into variable buf
void TI_readBlock(TOFIndex *F, long i, TblocIndex *buf)
{
   fseek(F->f, sizeof(THeaderIndex) + (i - 1) * sizeof(TblocIndex), SEEK_SET);
   fread(buf, sizeof(TblocIndex), 1, F->f);
}

// writing the contents of the variable buf in data block number i
void TI_writeBlock(TOFIndex *F, long i, TblocIndex *buf)
{
   fseek(F->f, sizeof(THeaderIndex) + (i - 1) * sizeof(TblocIndex), SEEK_SET);
   fwrite(buf, sizeof(TblocIndex), 1, F->f);
}

// header modification
void TI_setHeader(TOFIndex *F, char *hname, long val)
{
   if (strcmp(hname, "nBlock") == 0)
   {
      F->h.nBlock = val;
      return;
   }
   if (strcmp(hname, "nIns") == 0)
   {
      F->h.nIns = val;
      return;
   }
   if (strcmp(hname, "nDel") == 0)
   {
      F->h.nDel = val;
      return;
   }
   fprintf(stderr, "setHeader : Unknown headerName: \"%s\"\n", hname);
}

// header value
long TI_getHeader(TOFIndex *F, char *hname)
{
   if (strcmp(hname, "nBlock") == 0)
      return F->h.nBlock;
   if (strcmp(hname, "nIns") == 0)
      return F->h.nIns;
   if (strcmp(hname, "nDel") == 0)
      return F->h.nDel;
   fprintf(stderr, "getHeader : Unknown headerName: \"%s\"\n", hname);
}
