#include <stdio.h>
#include "TnOF_model.h"
#include "TOF_model.h"

T1 *Index;
TnOFData *DataFile;
TblocData bufData;
TOFIndex *IndexFile;
TblocIndex bufIndex;

void SaveToTOF() {
    TOF_open(&IndexFile, "index.tof", 'N');
    int i = 1;
    int j = 1;
    InordreT1(Index, i, j);
    bufIndex.NB = j-1;
    TOF_writeBlock(IndexFile, i, &bufIndex);
    TOF_setHeader(IndexFile, 1, i);
    TOF_close(IndexFile);
}

void InordreT1(T1 *A, int i, int j) {
    if (A != NULL) {
        InordreT1(A->fg, i, j);
        InordreT2(A->val.R, i, j);
        InordreT1(A->fd, i, j);
    }
}

void InordreT2(T2 *A, int i, int j) {
    if (A != NULL) {
        InordreT2(A->fg, i, j);
        if (j <= MAXTAB) {
            bufIndex.tab[j] = A->val;
            j++;
        } else {
            bufIndex.NB = j - 1;
            TOF_writeBlock(IndexFile, i, &bufIndex);
            i++;
            bufIndex.tab[1] = A->val;
            j = 2;
        }
        InordreT2(A->fd, i, j);
    }
}

