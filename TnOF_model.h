/************************************************************************
 * Interface of the algorithmic model to manipulate a 'TOF' type file   *
 *        - SFSD (File & Data Structures) / 2CP / ESI / 2024 - 	        *
 ************************************************************************/

#ifndef TnOF_MODEL_H
#define TnOF_MODEL_H

#include <stdio.h>

// maximum block capacity (in number of records)
#define MAXTAB 10

// type of records (adapt to the using context)
typedef long int t_rec; // in this example, a recor is just a 'long int'

// type of a data block (and therefore of buffer variables as well)
typedef struct blck
{
    t_rec tab[MAXTAB]; // array of records inside a block
    int nb;            // number of records inserted in the block
} t_block;

// the file header (characteristics)
typedef struct hdr
{
    long nBlock; // number of blocks in the file (this is also the number of the last block)
    long nIns;   // number of records in the file
} t_header;

// TnOF file structure
typedef struct TnOFstr
{
    FILE *f;    // C stream implementing the file
    t_header h; // the header in main memory
} t_TnOF;

/**** ****************************************** ****/
/**** The functions of the abstract machine      ****/
/**** ****************************************** ****/

// open a TnOF file / mode ='N' for a New file and mode ='E' for an Existing file
// returns a pointer to a newly allocated variable of type 't_TnOF'
void TnOF_open(t_TnOF **F, char *fname, char mode);

// close a TnOF file :
// the header is first saved at the beginning of the file and the t_TnOF variable is freed
void TnOF_close(t_TnOF *F);

// reading data block number i into variable buf
void TnOF_readBlock(t_TnOF *F, long i, t_block *buf);

// writing the contents of the variable buf in data block number i
void TnOF_writeBlock(t_TnOF *F, long i, t_block *buf);

// header modification
void TnOF_setHeader(t_TnOF *F, char *hname, long val);

// header value
long TnOF_getHeader(t_TnOF *F, char *hname);

#endif