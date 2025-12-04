/**********************************************************
 * Implementation of TnOF module functions (TnOF_model.c)   *
 * SFSD (File & Data Structures) / 2CP / ESI / 2024       *
 **********************************************************/

#include "TnOF_model.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// open a TnOF file / mode ='N' for a New file and mode ='E' for an Existing file
// returns a pointer to a newly allocated variable of type 't_TnOF'
void TnOF_open(t_TnOF **F, char *fname, char mode)
{
    *F = malloc(sizeof(t_TnOF));
    if (mode == 'E' || mode == 'e')
    {
        // openning an existing TnOF file ...
        (*F)->f = fopen(fname, "rb+");
        if ((*F)->f == NULL)
        {
            perror("TnOF_open");
            exit(EXIT_FAILURE);
        }
        // loading header part in main memory (in (*F)->h)
        fread(&((*F)->h), sizeof(t_header), 1, (*F)->f);
    }
    else
    { // mode == 'N' || mode == 'n'
        // creating a new TnOF file ...
        (*F)->f = fopen(fname, "wb+");
        if ((*F)->f == NULL)
        {
            perror("TnOF_open");
            exit(EXIT_FAILURE);
        }
        // initializing the header part in main memory (in (*)->h)
        (*F)->h.nBlock = 0;
        (*F)->h.nIns = 0;
        fwrite(&((*F)->h), sizeof(t_header), 1, (*F)->f);
    }
}

// close a TnOF file :
// the header is first saved at the beginning of the file and the t_TnOF variable is freed
void TnOF_close(t_TnOF *F)
{
    // saving header part in secondary memory (at the begining of the stream F->f)
    fseek(F->f, 0L, SEEK_SET);
    fwrite(&F->h, sizeof(t_header), 1, F->f);
    fclose(F->f);
    free(F);
}

// reading data block number i into variable buf
void TnOF_readBlock(t_TnOF *F, long i, t_block *buf)
{
    fseek(F->f, sizeof(t_header) + (i - 1) * sizeof(t_block), SEEK_SET);
    fread(buf, sizeof(t_block), 1, F->f);
}

// writing the contents of the variable buf in data block number i
void TnOF_writeBlock(t_TnOF *F, long i, t_block *buf)
{
    fseek(F->f, sizeof(t_header) + (i - 1) * sizeof(t_block), SEEK_SET);
    fwrite(buf, sizeof(t_block), 1, F->f);
}

// header modification
void TnOF_setHeader(t_TnOF *F, char *hname, long val)
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
    fprintf(stderr, "setHeader : Unknown headerName: \"%s\"\n", hname);
}

// header value
long TnOF_getHeader(t_TnOF *F, char *hname)
{
    if (strcmp(hname, "nBlock") == 0)
        return F->h.nBlock;
    if (strcmp(hname, "nIns") == 0)
        return F->h.nIns;
    fprintf(stderr, "getHeader : Unknown headerName: \"%s\"\n", hname);
}
