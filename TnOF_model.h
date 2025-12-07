/************************************************************************
 * Interface of the algorithmic model to manipulate a 'TOF' type file   *
 *        - SFSD (File & Data Structures) / 2CP / ESI / 2024 - 	        *
 ************************************************************************/

#ifndef TnOF_MODEL_H
#define TnOF_MODEL_H

#include <stdio.h>
#include "TOF_model.h"

// maximum block capacity (in number of records)
#define MAXTAB 10

// type of a data block (and therefore of buffer variables as well)

typedef struct t_TblocData
{
    Tenreg tab[MAXTAB];
    int NB;
} TblocData;

// the file header (characteristics)
typedef struct hdr
{
    long nBlock; // number of blocks in the file (this is also the number of the last block)
    long nIns;   // number of records in the file
} THeaderData;

// TnOF file structure
typedef struct t_TnOFData
{
    FILE *f;    // C stream implementing the file
    THeaderData h; // the header in main memory
} TnOFData;


/**** ****************************************** ****/
/**** The functions of the abstract machine      ****/
/**** ****************************************** ****/

// open a TnOF file / mode ='N' for a New file and mode ='E' for an Existing file
// returns a pointer to a newly allocated variable of type 'TnOFData'
void TnOF_open(TnOFData **F, char *fname, char mode);

// close a TnOF file :
// the header is first saved at the beginning of the file and the TnOFData variable is freed
void TnOF_close(TnOFData *F);

// reading data block number i into variable buf
void TnOF_readBlock(TnOFData *F, long i, TblocData *buf);

// writing the contents of the variable buf in data block number i
void TnOF_writeBlock(TnOFData *F, long i, TblocData *buf);

// header modification
void TnOF_setHeader(TnOFData *F, char *hname, long val);

// header value
long TnOF_getHeader(TnOFData *F, char *hname);

#endif