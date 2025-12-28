#include <stdio.h>
#include <stdbool.h>
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/********* Constantes et variables *********/

#define RED 12   // Code for the red color (Terminal)
#define GRAY 8   // Code for the gray color (Terminal)
#define BLUE 9   // Code for the blue color (Terminal)
#define GREEN 10 // Code for the green color (Terminal)
#define WHITE 15 // Code for the white color (Terminal)
#define CYAN 11  // Code for the cyan color (Terminal)
#define MAXTAB 10 // maximum block capacity (in number of records)

/*******************************************/

/************************* Terminal ***************************/

// Choose the color of the displayed text
void setColor(int ForgeC)
{ 
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hStdOut, ForgeC);
}

// Clear the screen
void clearScreen()
{
    system("cls");
}

/**************************************************************/



// type of values of T2
typedef struct t_T2VAL
{
    int cle;
    int nbloc, depl;
} ValT2;

// type of nodes of T2
typedef struct t_T2
{
    ValT2 val;
    struct t_T2 *fg, *fd;
} T2;

// type of values of T1
typedef struct t_T1VAL
{
    int v1, v2;
    T2 *R;
} ValT1;

// type of nodes of T1
typedef struct t_T1
{
    ValT1 val;
    struct t_T1 *fg, *fd;
} T1;

// type of records (adapt to the using context)
typedef struct t_tenreg
{
    int cle;
    int field;
} Tenreg;

// Index block structure (Ordered table of ValT2 - TOF index)
typedef struct t_TblocIndex
{
    ValT2 tab[MAXTAB + 1];
    int NB;
} TblocIndex;

// the file header (characteristics)
typedef struct t_hdr
{
    long nBlock; // number of blocks in the file (this is also the number of the last block)
    long nIns;   // number of records in the file
    long nDel;   // number of records deleted (logically) from file
} THeaderIndex;

// TOF file structure
typedef struct t_TOFIndex
{
    FILE *f;        // C stream implementing the file
    THeaderIndex h; // the header in main memory
} TOFIndex;

typedef struct node
{
    int val[4];
} Node;

// open a TOF file (In our case the Index) / mode ='N' for a New file and mode ='E' for an Existing file
// returns a pointer to a newly allocated variable of type 'TOFIndex'
void TI_open(TOFIndex **F, char *fname, char mode)
{
    *F = malloc(sizeof(TOFIndex));
    if (mode == 'E' || mode == 'e')
    {
        // opening an existing TOF file ...
        (*F)->f = fopen(fname, "rb+");
        if ((*F)->f == NULL)
        {
            perror("TOF_open");
            exit(EXIT_FAILURE);
        }
        // loading header part in main memory (in (*F)->h)
        fread(&((*F)->h), sizeof(THeaderIndex), 1, (*F)->f);
    }
    else
    { // mode == 'N' || mode == 'n'
        // creating a new TOF file ...
        (*F)->f = fopen(fname, "wb+");
        if ((*F)->f == NULL)
        {
            perror("TOF_open");
            exit(EXIT_FAILURE);
        }
        // initializing the header part in main memory (in (*F)->h)
        (*F)->h.nBlock = 0;
        (*F)->h.nIns = 0;
        (*F)->h.nDel = 0;
        fwrite(&((*F)->h), sizeof(THeaderIndex), 1, (*F)->f);
    }
}

// close a TOF file (In our case the Index) :
// the header is first saved at the beginning of the file and the TOFIndex variable is freed
void TI_close(TOFIndex *F)
{
    // saving header part in secondary memory (at the begining of the stream F->f)
    fseek(F->f, 0L, SEEK_SET);
    fwrite(&F->h, sizeof(THeaderIndex), 1, F->f);
    fclose(F->f);
    free(F);
}

// reading data block number i into variable buf
void TI_readBlock(TOFIndex *F, long i, TblocIndex *buf)
{
    fseek(F->f, sizeof(THeaderIndex) + (i - 1) * sizeof(TblocIndex), SEEK_SET);
    fread(buf, sizeof(TblocIndex), 1, F->f);
}

// writing the contents of the variable buf in data block number i
void TI_writeBlock(TOFIndex *F, long i, TblocIndex *buf)
{
    fseek(F->f, sizeof(THeaderIndex) + (i - 1) * sizeof(TblocIndex), SEEK_SET);
    fwrite(buf, sizeof(TblocIndex), 1, F->f);
}

// header modification
void TI_setHeader(TOFIndex *F, char *hname, long val)
{
    if (strcmp(hname, "nBlock") == 0)
    {
        F->h.nBlock = val;
        return;
    }
    if (strcmp(hname, "nIns") == 0)
    {
        F->h.nIns = val;
        return;
    }
    if (strcmp(hname, "nDel") == 0)
    {
        F->h.nDel = val;
        return;
    }
    fprintf(stderr, "setHeader : Unknown headerName: \"%s\"\n", hname);
}

// header value
long TI_getHeader(TOFIndex *F, char *hname)
{
    if (strcmp(hname, "nBlock") == 0)
        return F->h.nBlock;
    if (strcmp(hname, "nIns") == 0)
        return F->h.nIns;
    if (strcmp(hname, "nDel") == 0)
        return F->h.nDel;
    fprintf(stderr, "getHeader : Unknown headerName: \"%s\"\n", hname);
}

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
    if (nBlocks <= 0 || maxPerBlk <= 0 || maxPerBlk > MAXTAB || maxDataBlk <= 0)
    {
        fprintf(stderr, "generate_random_TOF_index: invalid parameters\n");
        return;
    }

    srand((unsigned)time(NULL));
    TOFIndex *F = NULL;
    TblocIndex buf;
    long total = 0;

    TI_open(&F, (char *)fname, 'N');

    for (int b = 1; b <= nBlocks; ++b)
    {
        int nb = (rand() % maxPerBlk) + 1; /* 1..maxPerBlk */
        memset(&buf, 0, sizeof(buf));
        buf.NB = nb;

        for (int p = 1; p <= nb; ++p)
        {
            ValT2 v;
            v.cle = (rand() % (10000 / nBlocks)) + 1 + (b - 1) * (10000 / nBlocks); /* random key */
            v.nbloc = (rand() % maxDataBlk) + 1;                                    /* random data block 1..maxDataBlk */
            v.depl = (rand() % MAXTAB) + 1;                                         /* random offset 1..MAXTAB */
            buf.tab[p] = v;                                                         /* requires TblocIndex to have tab[] of ValT2 */
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
    if (*first_element)
    {
        printf("   %08ld   | (%04ld, %04d) |\n", (long)r->val.cle, (long)r->val.nbloc, r->val.depl);
        *first_element = false;
    }
    else
    {
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

void print_index(T1 *root)
{
    if (!root)
    {
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

/************** Global variables ****************/

T1 *Index;
// TnOFData *DataFile;
// TblocData bufData;
TOFIndex *IndexFile;
TblocIndex bufIndex;

/***********************************************/

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


// Search for a key in T2.
//
// Parameters:
//  - `cle`    : (in)  key to search for (int)
//  - `R`      : (in)  root of the T2 tree to search (T2 *)
//  - `trouve` : (out) set to true if an exact node with key == cle is found (bool *)
//  -- if *trouve == true, *PS points to the node containing key
//  -- if *trouve == false, *PS is NULL (search ran off a leaf)
//  - `PS`     : (out) on return, points to the node where search stopped:
//  - `QS`     : (out) on return, points to the parent of *PS (NULL if *PS was root or tree empty)
//
// Behavior / notes:
//  - Traverses the BST using the usual invariant: left->val.cle < node->val.cle < right->val.cle.
//  - On each step, *QS holds the previous node (parent), *PS holds the current node.
//  - If the key is found, *trouve is set true and *PS points to that node.
//  - If the key isn't found, loop exits when *PS becomes NULL and *trouve remains false,
//    *QS is the last non-NULL node visited (the would-be parent for an insertion).
//  - Caller must pass addresses for `trouve`, `PS`, `QS`. After return these outputs indicate
//    whether the key exists and where to insert if it does not.
//
void RechT2(int cle, T2 *R, bool *trouve, T2 **PS, T2 **QS)
{
    *trouve = false;
    *PS = R;
    *QS = NULL;

    /* Loop while current node exists and key not found */
    while (*PS != NULL && !(*trouve))
    {
        if (cle == (*PS)->val.cle)
        {
            /* Found exact key in current node */
            *trouve = true;
        }
        else    /* Move down the tree: remember current as parent, then choose child */
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


// Find cle in T1/T2 index.
//
// Parameters:
//  - `cle`   : key to search.
//  - `trouve`: out == true if found.
//  - `P`     : out == points to T1 node examined (or where to insert).
//  - `Q`     : out == parent of *P.
//  - `PS`    : out == points to T2 node where T2 search stopped (or found).
//  - `QS`    : out == parent of *PS.
// Behavior: traverse T1 to find node whose range [v1..v2] contains cle,
// if found, call RechT2 on that node's val.R to locate the exact occurrence.
//
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
            /* Found node R where to search key */
            stop = true;
            RechT2(cle, (*P)->val.R, trouve, PS, QS);
        }
        else /* Move down T1 tree: remember current as parent, then choose child */
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
    P = NULL;
    Q = NULL;
    PS = NULL;
    QS = NULL;
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

// function to create one node and initialize all its values to -1 ( empty value )
Node *createNode()
{
    Node *newNode = (Node *)malloc(sizeof(Node));
    for (int i = 0; i < 4; i++)
    {
        newNode->val[i] = 0;
    }
    return newNode;
}

// After filling the node with 4 value we need to sort them in assending way
void sortNodeValues(Node *n)
{
    if (n == NULL)
        return;

    for (int i = 0; i < 4 - 1; i++)
    {
        for (int j = 0; j < 4 - i - 1; j++)
        {
            if (n->val[j] > n->val[j + 1])
            { // each time we compare every node's value with all the others value
                int temp = n->val[j];
                n->val[j] = n->val[j + 1];
                n->val[j + 1] = temp;
            }
        }
    }
}

// procedure to tell the user to fill the node with 4 values he wants
void fillNodeValues(Node *n)
{
    if (n == NULL)
        return;

    for (int i = 0; i < 4; i++)
    {
        printf("Enter value %d for the node P : ", i + 1);
        scanf("%d", &n->val[i]);
        printf("\n");
    }
    sortNodeValues(n); // after filling the node we sort its values
}

// procedure to print the values of our Node
void printNodeValues(Node *n, char p2)
{
    const int EMPTY = -1;
    if (n == NULL)
        return;
    printf("                            ___________________________\n");
    printf("                           |                           |\n");
    printf("                           |"); setColor(CYAN); printf("       Node %c Values       ", p2); setColor(WHITE); printf("|\n");
    printf("                           |___________________________|\n");
    printf("                           |      |      |      |      |\n");
    printf("                           |");
    for (int i = 0; i < 4; i++)
    {
        if (n->val[i] == EMPTY)
        {
            setColor(RED);
            printf(" %3d  ", n->val[i]); // if the value is -1 we print it in red color
            setColor(WHITE);
            printf("|");
        }
        else
        {
            setColor(GREEN);
            printf(" %3d  ", n->val[i]); // else we print it in Green color
            setColor(WHITE);
            printf("|");
        }
    }
    setColor(WHITE);
    printf("\n");
    printf("                           |______|______|______|______|\n"); // the box around the values is not colored
}


void helper_table(int tab[4], Node *p, int value)
{ // the role of this table he's like buffer or something temporary so we put all the values of our node inside it
    for (int i = 0; i < 5; i++)
    {
        tab[i] = p->val[i];
    }
    tab[4] = value; // we also add the 5th value that the user wants to add that's why this table has 5 cases
    for (int i = 0; i < 5 - 1; i++)
    {
        for (int j = 0; j < 5 - i - 1; j++)
        { // now we need to sort it to make the work easier for us to deside where the values will go after the split operation
            if (tab[j] > tab[j + 1])
            {
                int temp = tab[j];
                tab[j] = tab[j + 1];
                tab[j + 1] = temp;
            }
        }
    }
}

void split(Node *p, int mid)
{                      // the main procedure
    fillNodeValues(p); // we fill the node P with 4 values
    printf("Node P values after key insertion and ordering:\n");
    printNodeValues(p, 'P');
    int tab[5];
    int value_added;
    printf("please enter the value to be added : ");
    scanf("%d", &value_added);
    helper_table(tab, p, value_added); // we fill the helper table
    Node *Q = createNode();
    p->val[0] = tab[0]; // as the node P has already 4 values so when we add new one we've to split it
    p->val[1] = tab[1]; // the first 2 values will be in P
    mid = tab[2];       // the midean will be promoted to the parent
    Q->val[0] = tab[3]; // the last 2 values will go to the new node Q
    Q->val[1] = tab[4];
    const int EMPTY = -1; //-1 means empty value
    for (int i = 2; i < 4; i++)
    { // we need to empty the remaining values in both nodes
        p->val[i] = EMPTY;
        Q->val[i] = EMPTY;
    }
    printf("\nMiddle value to be promoted: %d\n", mid);  // we print the midean value
    printf("Values in original node p after split: \n"); // we print the values in both nodes after the split operation
    printNodeValues(p, 'P');
    printf("Values in new node Q after split: \n");
    printNodeValues(Q, 'Q');
    free(Q);
    free(p); // we free our memoery
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
    print_index(Index);
    SaveToTOF();
    LoadFromTOF();
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

    Node *p = createNode();
    int mid = 0;
    split(p, mid);

    return 0;
}