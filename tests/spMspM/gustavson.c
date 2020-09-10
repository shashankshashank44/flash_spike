#include <stdio.h>
#include <rv32_vec_ins.h>
#ifndef GENERATE_ONLY
#include <generated_macros.h>
#endif
#include <stdlib.h>
#include <string.h>
//#include <context_sr_macros.h>
//#include <setup.h>

bool tooLarge (int nr, int nc)
{
   // we can not handle matrices that require more than 1GB mem
   // 1GB is 2^30 == > 2^28 4-byte elements.

   unsigned long int ne=nr*nc;
   if (ne >= 1073741824) return true;
   return false;
}

void handleError(int nr, int nc)
{
   printf("Size of matrix exceeds simulator capacity!\n");
   exit(0);
}

void loadConstantToReg(char * name, int address, int reg, int*lower_12);

unsigned long read_cycles(void)
{
  unsigned long cycles;
  asm volatile ("rdcycle %0" : "=r" (cycles));
  return cycles;
}

int *c=0;
int n;
double sparsity;

// CSR representation of matrix A.
char a_file_name[1024];
int *a=0;
int * a_rows;
int * a_cols;
int * a_vals;
int a_nnz=0;

// CSR representation of matrix B.
char b_file_name[1024];
int *b=0;
int * b_rows;
int * b_cols;
int * b_vals;
int b_nnz=0;

int t; // just for verification
volatile int last_op;

void freeMem(void)
{
    free(a); 
    free(b); 
    free(c); 
    free(a_rows); 
    free(a_cols); 
    free(a_vals); 
    free(b_rows); 
    free(b_cols); 
    free(b_vals); 
}

void initSparse(int * m, int *rows, int*cols, int* vals, int *g_nnz)
{
    int k=0;
    int nnz=0;
    rows[0]=0;
    *g_nnz=0;
    for (int i=0;i<n;i++)
    {
       nnz=0;
       for (int j=0;j<n;j++)
       {
           if (m[i*n+j] != 0) {
              cols[k] = j;
              vals[k] = m[i*n+j];
              k++;
              nnz++;
              (*g_nnz)++;
           }  
       }
       while (nnz % 8) {
          cols[k] = n;
          vals[k] = 0;
          k++;
          nnz++;
       }
       rows[i+1]=rows[i]+nnz;
    }

    sparsity = (n*n - *g_nnz)/((double)n*n);
}

void execSparseScalar(void)
{
}

void genSparseVector(void)
{
}

void loadConstantToReg(char * name, int address, int reg, int*var_lower_12)
{
    int upper_20 = address;
    int lower_12 = address;
    upper_20 = upper_20 >> 12;
    lower_12 = lower_12 & 0xfff;
    *var_lower_12 = lower_12;
    char str[1024];
    sprintf(&str[0], "ld_r%d_%s_upper_20_macro", reg, name);
    generateAsm(str, LD_IMM(reg, upper_20));
    sprintf(&str[0], "add_r%d_r%d_imm_%s_lower_12_macro", reg, reg, name);
    generateAsm(str, ADD_IMM(reg, reg, lower_12));
    sprintf(&str[0], "sub_r%d_r%d_r12_macro", reg, reg);
    generateAsm(&str[0], SUB_R_R(reg, reg, R12));
}

void genHWHelper(void)
{
}

#ifndef GENERATE_ONLY
void execSparseVector(void)
{
}

void execHWHelper(void)
{
}

void execDenseVector(void)
{
}

#endif

void execDenseScalar(void)
{
    // gustavson's works by pairwise multiplying 
    // and accumulating partial sums of rows.

    for (int i=0;i<n;i++)
    {
        for (int j=0;j<n;j++)
        {

        }
    }
}

void genDenseVector(void)
{
}

void usage(void)
{
    printf("Program requires five args:\n");
    printf("Arg 1: 0 or 1 value -- 0 if only compile, 1 if compile+exec\n");
    printf("Arg 2: 0 - dense matrix, 1 - CSR sparse matrix, 2 - TAMU COO sparse matrix\n");
    printf("Arg 3: matrix data file A as input.\n");
    printf("Arg 4: matrix data file B as input.\n");
    printf("Arg 5: 0 or 1 value -- 0 if doing dense, 1 if doing sparse\n");
    printf("Arg 6: 0/1/2 value -- 0 if using scalar, 1 if using vectors, 2 if using hardware helper (2 is valid only with sparse)\n");
}

typedef enum {DENSE=0, CSR=1, COO=2} matrix_type_t;

int main(int argc, char ** argv)
{
    if (argc <= 6) { usage(); return 0; }
    bool do_compile_exec = false;
    if (strcmp(argv[1],"1") == 0) do_compile_exec = true;

    FILE*a_fp = fopen(argv[3],"r");
    strcpy(a_file_name, argv[3]);
    if (!a_fp) { printf("Could not open %s for reading matrix A.\n", argv[3]); return 0;}

    FILE*b_fp = fopen(argv[4],"r");
    strcpy(b_file_name, argv[4]);
    if (!b_fp) { printf("Could not open %s for reading matrix B.\n", argv[4]); return 0;}

    matrix_type_t matrix_type;
    if (strcmp(argv[2], "0")==0) matrix_type=DENSE;
    else if (strcmp(argv[2], "1")==0) matrix_type=CSR;
    else if (strcmp(argv[2], "2")==0) matrix_type=COO;
    else { printf("Unknown matrix type.\n"); usage(); return 0;}

    if (matrix_type == DENSE) {
        fscanf(a_fp, "%d\n", &n);
        fscanf(b_fp, "%d\n", &t);
        if (n != t) {
            printf("Matrices are incompatible in size. Can't be multiplied.\n");
            exit(0);
        }
        if (tooLarge(n, n)) handleError(n, n);
        a = (int*)malloc(n*n*sizeof(int));
        b = (int*)malloc(n*n*sizeof(int));
        c = (int*)malloc(n*n*sizeof(int));
        for (int i=0;i<n*n;i++) fscanf(a_fp,"%d\n", &a[i]);
        for (int i=0;i<n*n;i++) fscanf(b_fp,"%d\n", &b[i]);
    }
    else if (matrix_type == CSR) {
        printf("Format as yet unsupported\n");
        return 0;
    } else if (matrix_type == COO) {
        // Read TAMU Matrix Market formatted input
        // Assumptions: commented lines start with %
        // Matrix COO indices are index-1 based and not index-0 based
        printf("Format as yet unsupported\n");
        return 0;
    }
    a_rows = (int*) malloc((n+1)*sizeof(int));
    a_cols = (int*) malloc((n+8)*(n+8)*sizeof(int));
    a_vals = (int*) malloc((n+8)*(n+8)*sizeof(int));
    b_rows = (int*) malloc((n+1)*sizeof(int));
    b_cols = (int*) malloc((n+8)*(n+8)*sizeof(int));
    b_vals = (int*) malloc((n+8)*(n+8)*sizeof(int));
    initSparse(a, a_rows, a_cols, a_vals, &a_nnz);
    initSparse(b, b_rows, b_cols, b_vals, &b_nnz);

    bool do_sparse = false;
    if (strcmp(argv[5],"1") == 0) do_sparse = true;

    bool use_vectors = false;
    bool use_hw_helper = false;
    if (strcmp(argv[6],"1") == 0) use_vectors = true;
    if (strcmp(argv[6],"2") == 0) use_hw_helper = true;

    if (!do_sparse && use_hw_helper) {
        printf("Invalid options: HW Helper option is only valid with sparse computation.\n");
        exit(0);
    }

    execDenseScalar();

#define COMPARE (n*n-1)
    last_op = c[COMPARE];
    printf("last_op is %d\n", last_op);
    c[COMPARE] = last_op -1;
    volatile unsigned long s, e;
    //save_regs();
#ifdef GENERATE_ONLY
    return 0;
#else
    if (!do_compile_exec) { freeMem(); return 0;}
    s = read_cycles();
    if (do_sparse)
    {
        if (use_vectors)
        {
            execSparseVector();
        }
        else if (use_hw_helper)
        {
            execHWHelper();
        }
        else
            execSparseScalar();
    }
    else {
        if (use_vectors)
        {
            execDenseVector();
        }
        else
        {
            execDenseScalar();
        }
    }
    //restore_regs();
    e = read_cycles();
    printf("Matrix A: %s; %d by %d; Matrix B: %s; %d by %d; %ld cycles\n", a_file_name, n, n, b_file_name, n, n, e-s);
    
    // check contents of y.
    printf("Last expected: %d\n", last_op);
    printf("Last output: %d\n", c[COMPARE]);
    if (last_op != c[COMPARE]) printf("Failed!\n");
    else printf("Passed!\n");

    freeMem();
    
    return 0;
#endif
}
