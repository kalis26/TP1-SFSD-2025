/**********************************************************
 * Implementation of TOF module functions (TOF_model.c)   *
 * SFSD (File & Data Structures) / 2CP / ESI / 2024       *
 **********************************************************/

#include "TOF_model.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// open a TOF file / mode ='N' for a New file and mode ='E' for an Existing file
// returns a pointer to a newly allocated variable of type 't_TOF'
void TOF_open(t_TOF **F, char *fname, char mode)
{
   *F = malloc(sizeof(t_TOF));
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
      fread(&((*F)->h), sizeof(t_header), 1, (*F)->f);
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
      fwrite(&((*F)->h), sizeof(t_header), 1, (*F)->f);
   }
}

// close a TOF file :
// the header is first saved at the beginning of the file and the t_TOF variable is freed
void TOF_close(t_TOF *F)
{
   // saving header part in secondary memory (at the begining of the stream F->f)
   fseek(F->f, 0L, SEEK_SET);
   fwrite(&F->h, sizeof(t_header), 1, F->f);
   fclose(F->f);
   free(F);
}

// reading data block number i into variable buf
void TOF_readBlock(t_TOF *F, long i, t_block *buf)
{
   fseek(F->f, sizeof(t_header) + (i - 1) * sizeof(t_block), SEEK_SET);
   fread(buf, sizeof(t_block), 1, F->f);
}

// writing the contents of the variable buf in data block number i
void TOF_writeBlock(t_TOF *F, long i, t_block *buf)
{
   fseek(F->f, sizeof(t_header) + (i - 1) * sizeof(t_block), SEEK_SET);
   fwrite(buf, sizeof(t_block), 1, F->f);
}

// header modification
void TOF_setHeader(t_TOF *F, char *hname, long val)
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
long TOF_getHeader(t_TOF *F, char *hname)
{
   if (strcmp(hname, "nBlock") == 0)
      return F->h.nBlock;
   if (strcmp(hname, "nIns") == 0)
      return F->h.nIns;
   if (strcmp(hname, "nDel") == 0)
      return F->h.nDel;
   fprintf(stderr, "getHeader : Unknown headerName: \"%s\"\n", hname);
}
