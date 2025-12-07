// ...existing code...
#ifndef GEN_DATA_H
#define GEN_DATA_H

#include "TnOF_model.h"
#include "TOF_model.h"
#include <stdlib.h>
#include <string.h>

/* generate a TnOF file of nBlocks, each block containing up to maxPerBlock random records.
   Records in each block are written using 1..NB indexing (index 0 left unused),
   to match the conventions used in the rest of your code. */
void generate_random_TnOF(const char *fname, int nBlocks, int maxPerBlock);

/* Build an in-memory index (T1 tree) from an existing TnOF file.
   Returns the root pointer (caller must free with free_index). */
T1 *build_index_from_TnOF_file(const char *fname);

/* print and free helpers */
void print_index(T1 *root);
void free_index(T1 *root);

#endif
// ...existing code...