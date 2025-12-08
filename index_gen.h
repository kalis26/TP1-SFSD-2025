#ifndef INDEX_GEN_H
#define INDEX_GEN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "TOF_model.h"

void generate_random_TOF_index(const char *fname, int nBlocks, int maxPerBlk, int maxDataBlk);

void print_index(T1 *root);
void free_index(T1 *root);

#endif