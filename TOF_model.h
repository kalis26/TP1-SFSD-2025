/************************************************************************
 * Interface of the algorithmic model to manipulate a 'TOF' type file   *
 *        - SFSD (File & Data Structures) / 2CP / ESI / 2024 - 	        *
 ************************************************************************/

#ifndef TOF_MODEL_H
#define TOF_MODEL_H

#include <stdio.h>

// maximum block capacity (in number of records)
#define MAXTAB 10

// type of records (adapt to the using context)
typedef long int t_rec; // in this example, a recor is just a 'long int'

// type of a data block (and therefore of buffer variables as well)
typedef struct blck
{
	t_rec tab[MAXTAB]; // array of records inside a block
	char del[MAXTAB];  // logical erase indicators ('*' erased / ' ' not erased)
	int nb;			   // number of records inserted in the block
} t_block;

// the file header (characteristics)
typedef struct hdr
{
	long nBlock; // number of blocks in the file (this is also the number of the last block)
	long nIns;	 // number of records in the file
	long nDel;	 // number of records deleted (logically) from file
} t_header;

// TOF file structure
typedef struct TOFstr
{
	FILE *f;	// C stream implementing the file
	t_header h; // the header in main memory
} t_TOF;

/**** ****************************************** ****/
/**** The functions of the abstract machine      ****/
/**** ****************************************** ****/

// open a TOF file / mode ='N' for a New file and mode ='E' for an Existing file
// returns a pointer to a newly allocated variable of type 't_TOF'
void TOF_open(t_TOF **F, char *fname, char mode);

// close a TOF file :
// the header is first saved at the beginning of the file and the t_TOF variable is freed
void TOF_close(t_TOF *F);

// reading data block number i into variable buf
void TOF_readBlock(t_TOF *F, long i, t_block *buf);

// writing the contents of the variable buf in data block number i
void TOF_writeBlock(t_TOF *F, long i, t_block *buf);

// header modification
void TOF_setHeader(t_TOF *F, char *hname, long val);

// header value
long TOF_getHeader(t_TOF *F, char *hname);

#endif
