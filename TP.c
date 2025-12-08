#include <stdio.h>
#include "index_gen.h"
#include <stdbool.h>

T1 *Index;
// TnOFData *DataFile;
// TblocData bufData;
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
        int v1 = bufIndex.tab[1].cle;
        int v2 = bufIndex.tab[bufIndex.NB].cle;
        T2* R = dicho2(1, bufIndex.NB);
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

void RechT2(int cle, T2 *R, bool *trouve, T2 **PS, T2 **QS)
{
    *trouve = false;
    *PS = R;
    *QS = NULL;
    while (*PS != NULL && !(*trouve))
    {
        if (cle == (*PS)->val.cle)
        {
            *trouve = true;
        }
        else
        {
            *QS = *PS;
            if (cle < (*PS)->val.cle)
            {
                *PS = (*PS)->fg;
            }
            else
            {
                *PS = (*PS)->fd;
            }
        }
    }
}

void Recherche(int cle, bool *trouve, T1 **P, T1 **Q, T2 **PS, T2 **QS)
{
    *trouve = false;
    *P = Index;
    *Q = NULL;
    bool stop = false;
    while (*P != NULL && !(*trouve) && !stop)
    {
        if (cle >= (*P)->val.v1 && cle <= (*P)->val.v2)
        {
            stop = true;
            RechT2(cle, (*P)->val.R, trouve, PS, QS);
        }
        else
        {
            *Q = *P;
            if (cle < (*P)->val.v1)
            {
                *P = (*P)->fg;
            }
            else
            {
                *P = (*P)->fd;
            }
        }
    }
}

void Inserer(Tenreg *e, int nBlocks) {
    bool trouve;
    T1 *P, *Q;
    T2 *PS, *QS;
    e->cle = rand() % 10000 + 1;
    Recherche(e->cle, &trouve, &P, &Q, &PS, &QS);
    if (!trouve) {
        int i = rand() % (nBlocks + 1) + 1;
        int j = rand() % MAXTAB + 1;
        ValT2 v;
        v.cle = e->cle;
        v.nbloc = i;
        v.depl = j;
        T2 *PS = malloc(sizeof(T2));
        PS->val = v;
        PS->fg = NULL;
        PS->fd = NULL;
        if (QS != NULL) {
            if (e->cle < QS->val.cle) {
                QS->fg = PS;
            } else {
                QS->fd = PS;
            }
        } else {
            if (P != NULL) {
                P->val.R = PS;
            } else {
                T1 *P = malloc(sizeof(T1));
                P->val.v1 = e->cle;
                P->val.v2 = e->cle;
                P->val.R = PS;
                if (Q != NULL) {
                    if (e->cle < Q->val.v1) {
                        Q->fg = P;
                    } else {
                        Q->fd = P;
                    }
                } else {
                    Index = P;
                }
            }
        }
    }
}

int main(void)
{

    srand((unsigned)time(NULL));

    const char *fname = "index.tof";
    int nBlocks = 5;
    int maxPerBlk = MAXTAB;
    int maxDataBlk = 10;
    generate_random_TOF_index(fname, nBlocks, maxPerBlk, maxDataBlk);
    printf("Index file '%s' generated (%d blocks, up to %d entries/block)\n", fname, nBlocks, maxPerBlk);
    LoadFromTOF();
    printf("Index content:\n");
    print_index(Index);
    SaveToTOF();
    LoadFromTOF();
    printf("Index content:\n");
    print_index(Index);

    Tenreg e;
    Inserer(&e, nBlocks);

    printf("After insertion of key %d, index content:\n", e.cle);

    print_index(Index);

    SaveToTOF();
    LoadFromTOF();

    printf("Index content after reloading from TOF file:\n");
    print_index(Index);

    free_index(Index);

    return 0;
}