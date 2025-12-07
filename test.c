// ...existing code...
#include <stdio.h>
#include "gen_data.h"

T1 *Index;
TnOFData *DataFile;
TblocData bufData;
TOFIndex *IndexFile;
TblocIndex bufIndex;

T2* dicho2(int bi, int bs) {
    if (bi <= bs) {
        int k = (bi + bs) / 2;
        T2 *P = malloc(sizeof(T2));
        P->val.cle = bufIndex.tab[k].cle;
        P->val.nbloc = bufIndex.tab[k].nbloc;
        P->val.depl = bufIndex.tab[k].depl;
        T2 *G = dicho2(bi, k - 1);
        T2 *D = dicho2(k + 1, bs);
        P->fg = G;
        P->fd = D;
        return P;
    } else {
        return NULL;
    }
}

T1* dicho1(int bi, int bs) {
    if (bi <= bs) {
        int k = (bi + bs) / 2;
        TI_readBlock(IndexFile, k, &bufIndex);
        T2* R = dicho2(1, bufIndex.NB);
        int v1 = bufIndex.tab[1].cle;
        int v2 = bufIndex.tab[bufIndex.NB].cle;
        T1 *P = malloc(sizeof(T1));
        P->val.v1 = v1;
        P->val.v2 = v2;
        P->val.R = R;
        T1 *G = dicho1(bi, k - 1);
        T1 *D = dicho1(k + 1, bs);
        P->fg = G;
        P->fd = D;
        return P;
    } else {
        return NULL;
    }
}

void LoadFromTOF() {
    TI_open(&IndexFile, "index.tof", 'E');
    int bi = 1;
    int bs = TI_getHeader(IndexFile, "nBlock");
    Index = dicho1(bi, bs);
    TI_close(IndexFile);
}

void InordreT2(T2 *A, int *i, int *j)
{
    if (A != NULL)
    {
        InordreT2(A->fg, i, j);
        if (*j <= MAXTAB)
        {
            bufIndex.tab[*j] = A->val;
            (*j)++;
        }
        else
        {
            bufIndex.NB = *j - 1;
            TI_writeBlock(IndexFile, *i, &bufIndex);
            printf("WROTE BLOCK %d with %d entries\n", *i, bufIndex.NB);
            (*i)++;
            bufIndex.tab[1] = A->val;
            *j = 2;
        }
        InordreT2(A->fd, i, j);
    }
}

void InordreT1(T1 *A, int *i, int *j)
{
    if (A != NULL)
    {
        InordreT1(A->fg, i, j);
        InordreT2(A->val.R, i, j);
        InordreT1(A->fd, i, j);
    }
}

void SaveToTOF()
{
    TI_open(&IndexFile, "index.tof", 'N');
    int i = 1;
    int j = 1;
    memset(&bufIndex, 0, sizeof(bufIndex));
    InordreT1(Index, &i, &j);
    bufIndex.NB = j - 1;
    TI_writeBlock(IndexFile, i, &bufIndex);
    TI_setHeader(IndexFile, "nBlock", i);
    TI_close(IndexFile);
}

int main(void)
{
    const char *fname = "data.tnof";
    generate_random_TnOF(fname, 5, MAXTAB);
    Index = build_index_from_TnOF_file(fname);
    printf("Index content:\n");
    print_index(Index);
    SaveToTOF();
    LoadFromTOF();
    printf("Index content:\n");
    print_index(Index);
    free_index(Index);
    return 0;
}