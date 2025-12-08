/************************************************************************
 * Interface of the algorithmic model to manipulate a 'TOF' type file   *
 *        - SFSD (File & Data Structures) / 2CP / ESI / 2024 - 	        *
 ************************************************************************/

#ifndef TOF_MODEL_H
#define TOF_MODEL_H

#include <stdio.h>
#include "Structure_model.h"

// maximum block capacity (in number of records)
#define MAXTAB 10

// type of records (adapt to the using context)

typedef struct t_TblocIndex
{
	ValT2 tab[MAXTAB+1];
	int NB;
} TblocIndex;

// the file header (characteristics)
typedef struct t_hdr
{
	long nBlock; // number of blocks in the file (this is also the number of the last block)
	long nIns;	 // number of records in the file
	long nDel;	 // number of records deleted (logically) from file
} THeaderIndex;

// TOF file structure
typedef struct t_TOFIndex
{
	FILE *f;	// C stream implementing the file
	THeaderIndex h; // the header in main memory
} TOFIndex;

/**** ****************************************** ****/
/**** The functions of the abstract machine      ****/
/**** ****************************************** ****/

// open a TOF file / mode ='N' for a New file and mode ='E' for an Existing file
// returns a pointer to a newly allocated variable of type 't_TOF'
void TI_open(TOFIndex **F, char *fname, char mode);

// close a TOF file :
// the header is first saved at the beginning of the file and the t_TOF variable is freed
void TI_close(TOFIndex *F);

// reading data block number i into variable buf
void TI_readBlock(TOFIndex *F, long i, TblocIndex *buf);

// writing the contents of the variable buf in data block number i
void TI_writeBlock(TOFIndex *F, long i, TblocIndex *buf);

// header modification
void TI_setHeader(TOFIndex *F, char *hname, long val);

// header value
long TI_getHeader(TOFIndex *F, char *hname);

#endif
