#include "index_gen.h"
#include <stdbool.h>

#include <windows.h>

/********* Constantes et variables *********/

#define RED 12   // Code pour la couleur rouge (Terminal)
#define GRAY 8   // Code pour la couleur grise
#define BLUE 9   // Code pour la couleur bleue
#define GREEN 10 // Code pour la couleur verte
#define WHITE 15 // Code pour la couleur blanche
#define CYAN 11  // Code pour la couleur cyan

/*******************************************/

/************************* Terminal ***************************/

void setColor(int ForgeC)
{ // Choisir la couleur du texte affiché
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hStdOut, ForgeC);
}

void clearScreen()
{ // Rénitialiser l'écran
    system("cls");
}

/**************************************************************/

/* comparator for ValT2 by key */
static int valt2_cmp(const void *a, const void *b)
{
    const ValT2 *x = (const ValT2 *)a;
    const ValT2 *y = (const ValT2 *)b;
    return (x->cle - y->cle);
}

/* Generate a random TOF index file.
   fname     : output filename
   nBlocks   : number of index blocks to create
   maxPerBlk : maximum entries per block (will use 1..maxPerBlk random)
   maxDataBlk: maximum data block number to use for ValT2.nbloc (>=1)
   Uses 1-based indexing for entries inside a block (tab[1]..tab[NB]).
*/
void generate_random_TOF_index(const char *fname, int nBlocks, int maxPerBlk, int maxDataBlk)
{
    if (nBlocks <= 0 || maxPerBlk <= 0 || maxPerBlk > MAXTAB || maxDataBlk <= 0) {
        fprintf(stderr, "generate_random_TOF_index: invalid parameters\n");
        return;
    }

    srand((unsigned)time(NULL));
    TOFIndex *F = NULL;
    TblocIndex buf;
    long total = 0;

    TI_open(&F, (char *)fname, 'N');

    for (int b = 1; b <= nBlocks; ++b) {
        int nb = (rand() % maxPerBlk) + 1; /* 1..maxPerBlk */
        memset(&buf, 0, sizeof(buf));
        buf.NB = nb;

        for (int p = 1; p <= nb; ++p) {
            ValT2 v;
            v.cle = (rand() % (10000 / nBlocks)) + 1 + (b - 1) * (10000 / nBlocks);           /* random key */
            v.nbloc = (rand() % maxDataBlk) + 1;   /* random data block 1..maxDataBlk */
            v.depl = (rand() % MAXTAB) + 1;        /* random offset 1..MAXTAB */
            buf.tab[p] = v;                        /* requires TblocIndex to have tab[] of ValT2 */
        }

        /* sort block by key so tab[1] is smallest and tab[NB] largest */
        qsort(&buf.tab[1], buf.NB, sizeof(buf.tab[0]), valt2_cmp);

        TI_writeBlock(F, b, &buf);
        total += nb;
    }

    TI_setHeader(F, "nBlock", nBlocks);
    TI_setHeader(F, "nIns", total);
    TI_close(F);
}

static void print_t2(T2 *r, bool *first_element)
{
    if (!r)
        return;
    print_t2(r->fg, first_element);
    if (*first_element) {
        printf("   %08ld   | (%04ld, %04d) |\n", (long)r->val.cle, (long)r->val.nbloc, r->val.depl);
        *first_element = false;
    } else {
        printf("|            |            |   %08ld   | (%04ld, %04d) |\n", (long)r->val.cle, (long)r->val.nbloc, r->val.depl);
    }
    print_t2(r->fd, first_element);
}

static void print_t1(T1 *r)
{
    if (!r)
        return;
    bool *first_element = malloc(sizeof(bool));
    *first_element = true;
    print_t1(r->fg);
    printf("|  %08d  |  %08d  |", r->val.v1, r->val.v2);
    print_t2(r->val.R, first_element);
    printf("|____________|____________|______________|______________|\n");
    print_t1(r->fd);
    free(first_element);
}

/* Free helpers */
static void free_t2(T2 *r)
{
    if (!r)
        return;
    free_t2(r->fg);
    free_t2(r->fd);
    free(r);
}

static void free_t1(T1 *r)
{
    if (!r)
        return;
    free_t1(r->fg);
    free_t1(r->fd);
    free_t2(r->val.R);
    free(r);
}

void print_index(T1 *root) { 
    if (!root) {
        printf("<empty index>\n");
        return;
    }
    printf("Index content:\n");
    printf(" _______________________________________________________\n");
    printf("|            |            |              R              |\n");
    printf("|    Val1    |    Val2    |---------------------------- |\n");
    printf("|            |            |     Cles     |  Depl (i,j)  |\n");
    printf("|____________|____________|______________|______________|\n");
    print_t1(root);
}
void free_index(T1 *root) { free_t1(root); }