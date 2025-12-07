// ...existing code...
#include "gen_data.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

/* Internal helpers (T1 = main index by key; T2 = occurrences tree) */

/* Create a new T2 node */
static T2 *t2_new_node(ValT2 v)
{
    T2 *n = malloc(sizeof(T2));
    if (!n)
    {
        perror("t2_new_node");
        exit(EXIT_FAILURE);
    }
    n->val = v;
    n->fg = n->fd = NULL;
    return n;
}

/* Insert occurrence into T2 BST using (nbloc,depl) as comparison */
static T2 *t2_insert(T2 *root, ValT2 v)
{
    if (root == NULL)
        return t2_new_node(v);
    if (v.nbloc < root->val.nbloc || (v.nbloc == root->val.nbloc && v.depl < root->val.depl))
        root->fg = t2_insert(root->fg, v);
    else
        root->fd = t2_insert(root->fd, v);
    return root;
}

/* Create new T1 node */
static T1 *t1_new_node(int key, ValT2 occ)
{
    T1 *n = malloc(sizeof(T1));
    if (!n)
    {
        perror("t1_new_node");
        exit(EXIT_FAILURE);
    }
    n->val.v1 = key;
    n->val.v2 = key; /* set v2 to same key so v1 <= v2 */
    n->val.R = NULL;
    n->fg = n->fd = NULL;
    n->val.R = t2_insert(n->val.R, occ);
    return n;
}

/* Insert into T1 BST by key (val.v1). If exists, add occurrence into its T2 */
static T1 *t1_insert(T1 *root, int key, ValT2 occ)
{
    if (root == NULL)
        return t1_new_node(key, occ);
    if (key < root->val.v1)
        root->fg = t1_insert(root->fg, key, occ);
    else if (key > root->val.v1)
        root->fd = t1_insert(root->fd, key, occ);
    else
    {
        /* same key: add occurrence to secondary tree */
        root->val.R = t2_insert(root->val.R, occ);
    }
    return root;
}

/* Print functions */
static void print_t2(T2 *r)
{
    if (!r)
        return;
    print_t2(r->fg);
    printf("    (bloc=%ld, depl=%d)\n", (long)r->val.nbloc, r->val.depl);
    print_t2(r->fd);
}

static void print_t1(T1 *r)
{
    if (!r)
        return;
    print_t1(r->fg);
    printf("Val1=%d\n", r->val.v1);
    printf("Val2=%d\n", r->val.v2);
    print_t2(r->val.R);
    print_t1(r->fd);
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

/* Public print/free wrappers */
void print_index(T1 *root) { print_t1(root); }
void free_index(T1 *root) { free_t1(root); }

/* Generate a random TnOF file. Uses 1..NB indexing for records in a block. */
// ...existing code...

/* comparator for Tenreg by key */
static int tenreg_cmp(const void *a, const void *b)
{
    const Tenreg *x = a;
    const Tenreg *y = b;
    return (x->cle - y->cle);
}

/* Generate a random TnOF file. Uses 1..NB indexing for records in a block. */
void generate_random_TnOF(const char *fname, int nBlocks, int maxPerBlock)
{
    if (nBlocks <= 0 || maxPerBlock <= 0 || maxPerBlock > MAXTAB)
    {
        fprintf(stderr, "generate_random_TnOF: invalid parameters\n");
        return;
    }
    srand((unsigned)time(NULL));
    TnOFData *F = NULL;
    TblocData buf;
    long total = 0;

    TnOF_open(&F, (char *)fname, 'N');

    for (int i = 1; i <= nBlocks; ++i)
    {
        int nb = (rand() % maxPerBlock) + 1; /* 1..maxPerBlock */
        memset(&buf, 0, sizeof(buf));
        buf.NB = nb;
        for (int p = 1; p <= nb; ++p)
        {
            Tenreg r;
            r.cle = (rand() % 1000) + 1;
            r.field = (rand() % 10000);
            buf.tab[p] = r;
        }

        /* sort records in the block by key so first/last are min/max */
        qsort(&buf.tab[1], nb, sizeof(buf.tab[0]), tenreg_cmp);

        TnOF_writeBlock(F, i, &buf);
        total += nb;
    }

    TnOF_setHeader(F, "nBlock", nBlocks);
    TnOF_setHeader(F, "nIns", total);
    TnOF_close(F);
    printf("Generated %ld records in %d blocks into '%s'\n", total, nBlocks, fname);
}
// ...existing code...

/* Build in-memory index (T1) from a TnOF file.
   Assumes blocks use 1..NB indexing for records. */
T1 *build_index_from_TnOF_file(const char *fname)
{
    TnOFData *F = NULL;
    TblocData buf;
    T1 *root = NULL;

    TnOF_open(&F, (char *)fname, 'E');
    long nBlocks = TnOF_getHeader(F, "nBlock");

    /* temporary array for sorting a block (size MAXTAB+1 to allow 1-based convention) */
    Tenreg tmp[MAXTAB + 1];

    for (long b = 1; b <= nBlocks; ++b)
    {
        TnOF_readBlock(F, b, &buf);
        int nb = buf.NB;
        if (nb < 0 || nb > MAXTAB)
            nb = 0;
        if (nb == 0)
            continue;

        /* copy block into tmp using indices 1..nb if possible */
        /* this code assumes your blocks are written in 1..NB convention (common in your project) */
        for (int p = 1; p <= nb; ++p)
            tmp[p] = buf.tab[p];

        /* sort the block by key so tmp[1] is min and tmp[nb] is max */
        qsort(&tmp[1], nb, sizeof(tmp[0]), tenreg_cmp);

        /* insert each record into the in-memory index */
        for (int p = 1; p <= nb; ++p)
        {
            Tenreg r = tmp[p];
            ValT2 occ;
            occ.cle = r.cle;
            occ.nbloc = b;
            occ.depl = p; /* position inside the block after sorting */
            root = t1_insert(root, r.cle, occ);
        }
    }

    TnOF_close(F);
    return root;
}
// ...existing code...