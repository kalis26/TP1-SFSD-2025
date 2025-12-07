typedef struct t_T2VAL
{
    int cle;
    int nbloc, depl;
} ValT2;

typedef struct t_T2
{
    ValT2 val;
    struct t_T2 *fg, *fd;
} T2;

typedef struct t_T1VAL
{
    int v1, v2;
    T2 *R;
} ValT1;

typedef struct t_T1
{
    ValT1 val;
    struct t_T1 *fg, *fd;
} T1;

typedef struct t_tenreg
{
    int cle;
    int field;
} Tenreg;