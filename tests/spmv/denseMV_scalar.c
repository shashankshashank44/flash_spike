#include <stdio.h>
#include <rv32_vec_ins.h>
#ifndef GENERATE_ONLY
#include <generated_macros.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <iostream>
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

void loadConstantToReg(char * name, unsigned long address, int reg, int*lower_12);

unsigned long long read_cycles(void)
#ifdef RISCV_BUILD
{
  unsigned long cycles;
  asm volatile ("rdcycle %0" : "=r" (cycles));
  return (unsigned long long)cycles;
}
#else
{
    unsigned int lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}
#endif

int v_size=8;
int vector_size = v_size;

int *m=0;
int *v=0;
int *y=0;
volatile int last_op;
int n;
int n_lower_12;
int nnz_lower_12;
int g_nnz=0;
int w_nnz=0;
char g_file_name[1024];
double sparsity;
int expand;

int num_r, num_c, num_nz;

int m_lower_12;
int v_lower_12;
int y_lower_12;

// CSR representation of matrix M.
int * rows;
int * cols;
int * vals;
int bv_lower_12;
int bv_upper_20;
int rows_lower_12;
int rows_upper_20;
int cols_lower_12;
int cols_upper_20;
int vals_lower_12;
int vals_upper_20;
int v_upper_20;
int rl_rows_lower_12;                                                                                                                                                        
int rl_rows_upper_20;                                                                                                                                                        
int rl_cols_lower_12;                                                                                                                                                        
int rl_cols_upper_20;                                                                                                                                                        
int rl_vals_lower_12;                                                                                                                                                        
int rl_vals_upper_20;
int b_vals_lower_12;                                                                                                                                                        
int b_vals_upper_20;


// Run-length representation of matrix M
int * rl_rows; // number of run lengths in each row
int * rl_cols; //holds pairs of (start column, number of columns)
int * rl_vals; //holds non-zero values
float average_run_length;

char * bitmap;
int  * b_vals;
// we do not create another values vector
// for M as CSR vals is the same format.

unsigned long HELPER_BASE = (3*1024*1024*1024);
int helper_base_lower_12;
int helper_base_upper_20;
volatile int *HHT_BASE = (int*)HELPER_BASE;

unsigned long PREFETCH_BASE = (3*1024*1024*1024) + (1024*1024);
int prefetch_base_lower_12;
int prefetch_base_upper_20;
//volatile int *PREFETCH_BASE = (int*)PREFETCH_BASE;

int t; // just for verification
int t_lower_12;
int t_upper_20;

typedef void (*PFN)(void);
PFN exec_fn;
typedef enum {S_DENSE=0, S_CSR=1, S_BV=2, S_RL=3} storage_type_t;
typedef enum {C_DENSE=0, C_CSR=1, C_BV=2, C_RL=3, C_HHT_DENSE=4, C_HHT_SPARSE=5} compute_type_t;
storage_type_t storage_type;
compute_type_t compute_type;
PFN assignExecuteFunction(storage_type_t storage_format, compute_type_t compute_format, int vector_size);

void execDenseDenseScalar(void);
void execDenseDenseVector2(void);
void execDenseDenseVector4(void);
void execDenseDenseVector8(void);
void execHelperDenseCSRScalar(void);
void execHelperDenseCSRVector2(void);
void execHelperDenseCSRVector4(void);
void execHelperDenseCSRVector8(void);
void execHelperDenseBVScalar(void);
void execHelperDenseBVVector2(void);
void execHelperDenseBVVector4(void);
void execHelperDenseBVVector8(void);
void execHelperDenseRLScalar(void);
void execHelperDenseRLVector2(void);
void execHelperDenseRLVector4(void);
void execHelperDenseRLVector8(void);
void execHelperSparseCSRScalar(void);
void execHelperSparseCSRVector2(void);
void execHelperSparseCSRVector4(void);
void execHelperSparseCSRVector8(void);
void execHelperSparseCSRVector8StoreCompress(void);
void execHelperSparseBVScalar(void);
void execHelperSparseBVVector2(void);
void execHelperSparseBVVector4(void);
void execHelperSparseBVVector8(void);
void execHelperSparseRLScalar(void);
void execHelperSparseRLVector2(void);
void execHelperSparseRLVector4(void);
void execHelperSparseRLVector8(void);
void execCSRCSRScalar(void);
void execCSRCSRVector2(void);
void execCSRCSRVector4(void);
void execCSRCSRVector8(void);
void execBVBVScalar(void);
void execBVBVVector2(void);
void execBVBVVector4(void);
void execBVBVVector8(void);
void execRLRLScalar(void);
void execRLRLVector2(void);
void execRLRLVector4(void);
void execRLRLVector8(void);

void initCSR()
{
    rows = (int*) malloc((n+1)*sizeof(int));
    cols = (int*) malloc((n+8)*(n+8)*sizeof(int));
    vals = (int*) malloc((n+8)*(n+8)*sizeof(int));
    printf("rows at 0x%x\n", &rows[0]);
    printf("Cols at 0x%x\n", &cols[0]);
    printf("vals at 0x%x\n", &vals[0]);

    int k=0;
    int nnz=0;
    rows[0]=0;
    g_nnz=0;
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
              g_nnz++;
			  w_nnz++;
           }  
       }
		if(expand != 4){
		   while (nnz % 8) {
			  cols[k] = n;
			  vals[k] = 0;
			  k++;
			  nnz++;
		   }
		}
       	rows[i+1]=rows[i]+nnz;
    }

    sparsity = (n*n - g_nnz)/((double)n*n);
}

void initBitVector(void)
{
    int bitmap_count = (n+1)*(n+8)/(8*sizeof(char));
    bitmap = (char*) malloc (bitmap_count*sizeof(char));
    for (int i=0; i<bitmap_count; i++) bitmap[i] = 0;
    b_vals = (int*) malloc((n+8)*(n+8)*sizeof(int));

    int w = 0;
    int l = 0;
    for (int i=0;i<n;i++)
    {
       for (int j=0;j<n;j+=8)
       {
           for (int k=0;k<8;k++)
           {
                if (m[i*n+j+k] != 0) {
                    bitmap[w] |= (1 << k);
                    b_vals[l++] = m[i*n+j+k];
                }
           }
           w++;
       }
    }
}

void initRunLength()
{
    rl_rows = (int *) malloc ((n+8)*sizeof(int));
    rl_vals = (int*) malloc((n+8)*(n+8)*sizeof(int));
    rl_cols = (int*) malloc(2*(n+8)*(n+8)*sizeof(int));

    int l=0;
    int h=0;
    int k=0;
    bool in_run = false;
    int rl=0;
    int start_col;
    for (int i=0;i<n;i++)
    {
       in_run = false;
       rl = 0;
       k = 0;
       for (int j=0;j<n;j++)
       {
            if (m[i*n+j] != 0) {
                rl_vals[l++] = m[i*n+j];

                if (!in_run) start_col = j;
                rl++;
                in_run = true; 
            } else {
                if (in_run)
                {
                    // found a run
                    rl_cols[h++]=rl;
                    rl_cols[h++]=start_col;
                    rl = 0;
                    in_run = false;
                    k++;
                }
            }
        }
        if (in_run)
        {
            // found a run at the end
            rl_cols[h++]=rl;
            rl_cols[h++]=start_col;
            rl = 0;
            in_run = false;
            k++;
        }
        rl_rows[i] = k;
    }

    int num_run_lengths = 0;
    int total_lengths = 0;
    k = 0;
    for (int i=0;i<n;i++) 
    {
        num_run_lengths += rl_rows[i];
        for (int j=0;j<rl_rows[i]; j++)
        {
            total_lengths += rl_cols[k];
            k+=2;
        }
    }
    //printf("Total number of RLs is %d\n", num_run_lengths);
    //printf("Total lengths is %d\n", total_lengths);
    average_run_length = (double) total_lengths/(double) num_run_lengths;

    printf("Average Run Length of Data is %f\n", average_run_length);
}

void initSparse(void)
{
    initCSR();
    initBitVector();
    initRunLength();
}

#if 0
void execSparseScalar(void)
{
    int s=0;
    int k=0;
    for (int i=0;i<n;i++)
    {
        int nnz = rows[i+1]-rows[i];
        s=0;
        if (nnz)
        {
           int x,z;

           // prologue of SW pipeline
           x = v[cols[k]];
           z = vals[k];
           
           for (int j=1;j<nnz;j++)
           {
               //s+=vals[k+j]*v[cols[k+j]];
               s += x*z;
               x = v[cols[k+j]];
               z = vals[k+j];
           }
           s += x*z; // epilogue of SW pipeline
           k+=nnz;
        }
        y[i]=s;
    }
}
#endif

void execCSRScalar(void)
{
    int s=0;
    int k=0;
    for (int i=0;i<n;i++)
    {
        int nnz = rows[i+1]-rows[i];
        s=0;
#if 0
        for (int j=0;j<nnz;j++)
        {
            s+=vals[k+j]*v[cols[k+j]];
            //s+=A_sparse_vals[k+j]*w_dense[j];
            //printf("Val %d from %d\n", vals[k+j], k+j);
            //printf("V %d from %d\n", v[cols[k+j]], cols[k+j]);
        }
#else
        int j;
        for (j=0;j<nnz;j+=8)
        {
            s+=vals[k+j]*v[cols[k+j]];
            s+=vals[k+j+1]*v[cols[k+j+1]];
            s+=vals[k+j+2]*v[cols[k+j+2]];
            s+=vals[k+j+3]*v[cols[k+j+3]];
            s+=vals[k+j+4]*v[cols[k+j+4]];
            s+=vals[k+j+5]*v[cols[k+j+5]];
            s+=vals[k+j+6]*v[cols[k+j+6]];
            s+=vals[k+j+7]*v[cols[k+j+7]];
        }

        int r = nnz - j;
        for (int h=0; h<r; h++)
        {
            s+=vals[k+j+h]*v[cols[k+j+h]];
        }
        

#endif
        k+=nnz;
        y[i]=s;
    }
}

void execBVScalar(void)
{
    int k=0;
    int w=0;
    int r = n%8;
    int s = 0;
    if (r) r = (8-r)+n;
    else r = n;

    for (int i=0;i<n; i++)
    {
        s = 0;
        for (int j=0;j<r; j+= 8*sizeof(char))
        {
            char bits = bitmap[w++];
            // got 8 bits of data
#if 0
            for (int m=0;m<8*sizeof(char);m++) {
                if (bits & 0x1) {
                    s += b_vals[k++] * v[j+m];
                    //printf("Val %d from %d\n", vals[k-1], k-1);
                    //printf("V %d from %d\n", v[j+m], j+m);
                }
                bits >>= 1;
            }
#else
            if (bits & 0x1) s += b_vals[k++] * v[j];    
            if (bits & 0x2) s += b_vals[k++] * v[j+1];    
            if (bits & 0x4) s += b_vals[k++] * v[j+2];    
            if (bits & 0x8) s += b_vals[k++] * v[j+3];    
            if (bits & 0x10) s += b_vals[k++] * v[j+4];    
            if (bits & 0x20) s += b_vals[k++] * v[j+5];    
            if (bits & 0x40) s += b_vals[k++] * v[j+6];    
            if (bits & 0x80) s += b_vals[k++] * v[j+7];    
#endif
        }
        //printf("Wrote %d to %d\n", s, i);
        y[i] = s;
    }
}

void execRLScalar(void)
{
    int s,c,k,l,num_cols,start_col,v_idx;
    k = 0;
    v_idx = 0;
    for (int i=0;i < n; i++)
    {
        s=0;
        c = rl_rows[i];
        for (int j=0;j<c; j++)
        {
            num_cols = rl_cols[k++];
            start_col = rl_cols[k++];
            for (l=0;l<num_cols;l++)
            {
                s += rl_vals[v_idx++]*v[start_col+l];
            }
        }
        y[i] = s;
    }
}

#if 0
void execSparseVector(void)
{
    int s=0;
    int k=0;
    int vm;
    for (int i=0;i<n;i++)
    {
        int nnz = rows[i+1]-rows[i];
        int num_iters = nnz/8;
        s=0;
        // set vmask = 8 1s.
        vm = 0xffffffff;
        for (int j=0;j<num_iters;j++)
        {
            if (j==(num_iters - 1)){
                // set vmask = nnz % 8 1s.
                // for eg: if nnz % 8 = 3, then set vmask to 111
                int r = nnz % 8;
                if (r) {
                    vm = (1 << (r*4)) - 1;
                }
            }
            // R10: holds k+j*8
            // load vals[k+j*8] through vals[k+j*8+7] with vm=0
            // load v[cols[k+j*8]] through v[cols[k+j*8+7]] with vm=0
            // multiply pairwise with vm=0
            // cumulate to s.
        }
        k+=nnz;
        y[i]=s;
    }
}
#endif

void genSparseVector(void)
{
    int n_upper_20 = (int)(&n);
    n_lower_12 = (int)(&n);
    n_upper_20 = n_upper_20 >> 12;
    n_lower_12 = n_lower_12 & 0xfff;
    generateAsm("ld_r12_imm_0xfffff_macro", LD_IMM(R12, 0xfffff));
    generateAsm("ld_r23_n_upper_20_macro", LD_IMM(R23, n_upper_20));
    generateAsm("add_r23_r23_imm_n_lower_12_macro", ADD_IMM(R23, R23, n_lower_12));
    generateAsm("sub_r23_r23_r12_macro", SUB_R_R(R23, R23, R12));
    generateAsm("ldw_r23_r23_imm_0_macro", LDW(R23, R23, 0));
    generateAsm("set_r18_0_macro", SET_0(R18));
    generateAsm("set_r19_0_macro", SET_0(R19));
    generateAsm("set_r20_0_macro", SET_0(R20));
    generateAsm("set_r21_0_macro", SET_0(R21));
    generateAsm("set_r22_0_macro", SET_0(R22));
    generateAsm("incr_r18_macro", INCR(R18));
    generateAsm("blt_r18_r23_imm_neg_4_macro", BLT(R18, R23, -4));
    generateAsm("blt_r18_r23_imm_neg_20_macro", BLT(R18, R23, -20));
    generateAsm("blt_r18_r23_imm_neg_40_macro", BLT(R18, R23, -40));
    generateAsm("blt_r18_r23_imm_neg_52_macro", BLT(R18, R23, -52));
    generateAsm("blt_r18_r23_imm_neg_60_macro", BLT(R18, R23, -60));
    generateAsm("blt_r18_r23_imm_neg_64_macro", BLT(R18, R23, -64));
    generateAsm("blt_r18_r23_imm_neg_68_macro", BLT(R18, R23, -68));
    generateAsm("blt_r18_r23_imm_neg_80_macro", BLT(R18, R23, -80));
    generateAsm("blt_r18_r23_imm_neg_84_macro", BLT(R18, R23, -84));
    generateAsm("blt_r18_r23_imm_neg_76_macro", BLT(R18, R23, -76));
    generateAsm("blt_r18_r23_imm_neg_92_macro", BLT(R18, R23, -92));
    generateAsm("add_r6_r0_imm_256_macro", ADD_IMM(R6,R0,256));
    generateAsm("add_r6_r0_imm_64_macro", ADD_IMM(R6,R0,64));
    generateAsm("add_r6_r0_imm_128_macro", ADD_IMM(R6,R0,128));
    generateAsm("vsetvli_r7_r6_e32_macro", VSETVLI(R7, R6, (E32 << VSEW_OFFSET)));
    t_upper_20 = (int)(&t);
    t_lower_12 = (int)(&t);
    t_upper_20 = t_upper_20 >> 12;
    t_lower_12 = t_lower_12 & 0xfff;
    generateAsm("ld_r24_t_upper_20_macro", LD_IMM(R24, t_upper_20));
    generateAsm("add_r24_r24_imm_t_lower_12_macro", ADD_IMM(R24, R24, t_lower_12));
    generateAsm("sub_r24_r24_r12_macro", SUB_R_R(R24, R24, R12));
    generateAsm("stw_r24_r18_imm_0_macro", STW(R24, R18, 0));
    generateAsm("stw_r24_r25_imm_0_macro", STW(R24, R25, 0));

	loadConstantToReg("nnz", (unsigned long)w_nnz, R26, &nnz_lower_12);
	
	loadConstantToReg("bv", (unsigned long)bitmap, R25, &bv_lower_12);

    loadConstantToReg("rows", (unsigned long)rows, R24, &rows_lower_12);

    loadConstantToReg("vals", (unsigned long)vals, R27, &vals_lower_12);

	loadConstantToReg("b_vals", (unsigned long)b_vals, R27, &b_vals_lower_12);
    
	loadConstantToReg("cols", (unsigned long)cols, R28, &cols_lower_12);
    
	loadConstantToReg("rl_rows", (unsigned long)rl_rows, R24, &rl_rows_lower_12);

    loadConstantToReg("rl_vals", (unsigned long)rl_vals, R27, &rl_vals_lower_12);

    loadConstantToReg("rl_cols", (unsigned long)rl_cols, R28, &rl_cols_lower_12);

    loadConstantToReg("v", (unsigned long)v, R29, &v_lower_12);
    loadConstantToReg("y", (unsigned long)y, R31, &y_lower_12);

    generateAsm("incr_r24_by_4_macro", INCR4(R24));
    generateAsm("ldw_r25_r24_imm_0_macro", LDW(R25, R24, 0));
    generateAsm("ldw_r22_r24_imm_0_macro", LDW(R22, R24, 0));
    generateAsm("ldw_r26_r24_imm_0_macro", LDW(R26, R24, 0));
    generateAsm("sub_r25_r26_r25_macro", SUB_R_R(R25, R26, R25));
    generateAsm("srl_r26_r25_imm_3_macro", SRL_IMM(R26, R25, 3));
    generateAsm("srl_r26_r23_imm_3_macro", SRL_IMM(R26, R23, 3));
	generateAsm("srl_r26_r25_imm_2_macro", SRL_IMM(R26, R25, 2));
    generateAsm("srl_r26_r25_imm_1_macro", SRL_IMM(R26, R25, 1));
    generateAsm("incr_r19_macro", INCR(R19));
    generateAsm("blt_r19_r26_imm_neg_4_macro", BLT(R19, R26, -4));
    generateAsm("blt_r19_r26_imm_neg_12_macro", BLT(R19, R26, -12));
    generateAsm("blt_r19_r26_imm_neg_20_macro", BLT(R19, R26, -20));
    generateAsm("blt_r19_r26_imm_neg_24_macro", BLT(R19, R26, -24));
    generateAsm("blt_r19_r26_imm_neg_28_macro", BLT(R19, R26, -28));
    generateAsm("blt_r19_r26_imm_neg_32_macro", BLT(R19, R26, -32));
    generateAsm("blt_r19_r26_imm_neg_36_macro", BLT(R19, R26, -36));
    generateAsm("beq_r19_r26_imm_pos_12_macro", BEQ(R19, R26, 12));
    generateAsm("beq_r19_r26_imm_pos_24_macro", BEQ(R19, R26, 24));
    generateAsm("beq_r19_r26_imm_pos_32_macro", BEQ(R19, R26, 32));
    generateAsm("beq_r19_r26_imm_pos_36_macro", BEQ(R19, R26, 36));
    generateAsm("beq_r19_r26_imm_pos_40_macro", BEQ(R19, R26, 40));
    generateAsm("beq_r19_r26_imm_pos_48_macro", BEQ(R19, R26, 48));
    generateAsm("nop", NOP);
    generateAsm("vlwv_v27_r27_macro", VLWV(V27,R27));
    generateAsm("vlwv_v28_r28_macro", VLWV(V28,R28));
	generateAsm("vlwv_v27_r29_macro", VLWV(V27,R29));
	generateAsm("incr32_r29_macro",INCR32(R29));
    generateAsm("incr32_r27_macro",INCR32(R27));
    generateAsm("incr8_r27_macro",INCR8(R27));
    generateAsm("incr16_r27_macro",INCR16(R27));
    generateAsm("incr4_r31_macro",INCR4(R31));
    generateAsm("incr32_r28_macro",INCR32(R28));
    generateAsm("incr16_r28_macro",INCR16(R28));
    generateAsm("incr8_r28_macro",INCR8(R28));
	generateAsm("add_r29_r28_imm_0_macro",ADD_IMM(R29, R28, 0));
    generateAsm("add_r28_r29_imm_0_macro",ADD_IMM(R28, R29, 0));
    generateAsm("vsll_vi_v28_v28_imm_4_macro", VSLL_VI(V28, V28, 2, VM));
    generateAsm("vswv_r24_v28_macro", VSWV(R24, V28));
    generateAsm("vswv_r31_v30_macro", VSWV(R31, V30));
    generateAsm("vliwv_v29_r29_v28_macro", VLIWV(V29, R29, V28));
    generateAsm("vmul_vv_v29_v27_v29_vm_macro", VMUL_VV(V29, V27, V29, VM));
    generateAsm("vredsum_vs_v30_v30_v29_macro", VREDSUM_VS(V30, V30, V29, VM));
    generateAsm("reset_v30_r0_macro", VMV_VX(V30,R0));
	generateAsm("reset_v29_r0_macro", VMV_VX(V29,R0));
    generateAsm("vlwv_v29_r30_macro", VLWV(V29,R30));
}

void loadConstantToReg(char * name, unsigned long address, int reg, int*var_lower_12)
{
    unsigned long upper_20 = address;
    unsigned long lower_12 = address;
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

/*void genHWHelper(void)
{
    // this is going to generate only the delta
    // instructions needed over and above whatever
    // genSparseVector(.) generates
    loadConstantToReg("helper_base", HELPER_BASE, R30, &helper_base_lower_12);

    // store n at *R30
    // n is stored in R23
    generateAsm("stw_r30_r23_imm_0_macro", STW(R30, R23, 0));

    // store rows address at *(R30+4)
    // rows is stored in R24
    generateAsm("stw_r30_r24_imm_4_macro", STW(R30, R24, 4));

    // store cols address at *(R30+8)
    // cols is stored in R28
    generateAsm("stw_r30_r28_imm_8_macro", STW(R30, R28, 8));

    // store v address at *(R30+12)
    // v is stored in R29
    generateAsm("stw_r30_r29_imm_12_macro", STW(R30, R29, 12));

    // store buffer id to *(R30+16)
    // in our case, buffer id is 0
    generateAsm("stw_r30_r0_imm_16_macro", STW(R30, R0, 16));

    // store start bit to *(R30+24)
    // start bit is simply a 1 written to LSB
    // first we gen instruction to store 1 into R22
    generateAsm("add_r22_r0_imm_1_macro", ADD_IMM(R22, R0, 1));
    generateAsm("stw_r30_r22_imm_24_macro", STW(R30, R22, 24));

    // add 1KB to R30 - to move the HELPER to point to
    // base address of buffer
    generateAsm("add_r30_r30_imm_1024_macro", ADD_IMM(R30, R30, 1024));
}*/

void genHWHelper_2(void)
{
    // this is going to generate only the delta
    // instructions needed over and above whatever
    // genSparseVector(.) generates
    loadConstantToReg("helper_base", HELPER_BASE, R30, &helper_base_lower_12);

    // store EC
    // EC is stored in R23
    generateAsm("stw_r30_r0_imm_0_macro", STW(R30, R0, 0));
    
	// EC as 1 is stored in R23
    generateAsm("stw_r30_r22_imm_0_macro", STW(R30, R22, 0));

    // store storage at *R30
    // storage type is stored in R23
    generateAsm("stw_r30_r0_imm_4_macro", STW(R30, R0, 4));

    // store n at *R30
    // n is stored in R23
    generateAsm("stw_r30_r23_imm_8_macro", STW(R30, R23, 8));

    // store rows address at *(R30+4)
    // rows is stored in R24
    generateAsm("stw_r30_r24_imm_12_macro", STW(R30, R24, 12));

    // store cols address at *(R30+8)
    // cols is stored in R28
    generateAsm("stw_r30_r28_imm_16_macro", STW(R30, R28, 16));

    // store v address at *(R30+12)
    // v is stored in R29
    generateAsm("stw_r30_r29_imm_20_macro", STW(R30, R29, 20));

    // store vals
    // vals is stored in R25
    generateAsm("stw_r30_r27_imm_24_macro", STW(R30, R27, 24));

    // store BV
    // Bv is stored in R25
    generateAsm("stw_r30_r25_imm_28_macro", STW(R30, R25, 28));
	
	// store buffer id to *(R30+16)
    // in our case, buffer id is 0
    generateAsm("stw_r30_r0_imm_32_macro", STW(R30, R0, 32));

    // store NNZ
    generateAsm("stw_r30_r26_imm_36_macro", STW(R30, R26, 36));

    // store start bit to *(R30+24)
    // start bit is simply a 1 written to LSB
    // first we gen instruction to store 1 into R22
    generateAsm("add_r22_r0_imm_1_macro", ADD_IMM(R22, R0, 1));
    generateAsm("add_r22_r0_imm_2_macro", ADD_IMM(R22, R0, 2));
    generateAsm("stw_r30_r22_imm_48_macro", STW(R30, R22, 48));
    generateAsm("stw_r30_r22_imm_4_macro", STW(R30, R22, 4));

    // add 1KB to R30 - to move the HELPER to point to
    // base address of buffer
    generateAsm("add_r30_r30_imm_1024_macro", ADD_IMM(R30, R30, 1024));
    

	generateAsm("add_r28_r29_imm_0_macro", ADD_IMM(R28, R29, 0));
}

void genprefetch()
{
	loadConstantToReg("prefetch_base", PREFETCH_BASE, R31, &prefetch_base_lower_12);	
	
	//cols
	generateAsm("stw_r31_r28_imm_0_macro", STW(R31, R28, 0));
	
	//vals
	generateAsm("stw_r31_r27_imm_4_macro", STW(R31, R27, 4));

	//V
	generateAsm("stw_r31_r29_imm_8_macro", STW(R31, R29, 8));

}

#ifndef GENERATE_ONLY
void execCSRVector_V2(void)
{
    // initialize locals i, j, s, k, vm;
    // i maps to R18
    // j maps to R19
    // s maps to R20
    // k maps to R21
    // vm maps to R22
    // n maps to R23
    // rows address maps to R24
    // rows[i] maps to R25
    // rows[i+1] maps to R26
    // nnz maps to R25 - dont need old R25
    // num_iters maps to R26
    // address of vals maps to R27
    
    // i,j, s, k, vm to 0
    set_r18_0_macro; 
    set_r19_0_macro; 
    set_r20_0_macro; 
    set_r21_0_macro; 
    set_r22_0_macro; 

    // load address of n to R23
    ld_r23_n_upper_20_macro;
    add_r23_r23_imm_n_lower_12_macro;
    ld_r12_imm_0xfffff_macro;
    if (n_lower_12 & 0x800) sub_r23_r23_r12_macro;

    // load value of n to R23 (do not need address of n anymore)
    ldw_r23_r23_imm_0_macro;

    // VL: 64 bits
    add_r6_r0_imm_64_macro;

    // SEW: 32 bits
    vsetvli_r7_r6_e32_macro;

    // outer loop on n.
    // for (int i=0;i<n;i++)
    // do the increment of i and then compare

    // load R24 with address of rows
    ld_r24_rows_upper_20_macro;
    add_r24_r24_imm_rows_lower_12_macro;
    if (rows_lower_12 & 0x800) sub_r24_r24_r12_macro;

    // load R27 with address of vals
    ld_r27_vals_upper_20_macro;
    add_r27_r27_imm_vals_lower_12_macro;
    if (vals_lower_12 & 0x800) sub_r27_r27_r12_macro;

    // load R28 with address of cols
    ld_r28_cols_upper_20_macro;
    add_r28_r28_imm_cols_lower_12_macro;
    if (cols_lower_12 & 0x800) sub_r28_r28_r12_macro;

    // load R29 with address of v[.] vector
    ld_r29_v_upper_20_macro;
    add_r29_r29_imm_v_lower_12_macro;
    if (v_lower_12 & 0x800) sub_r29_r29_r12_macro;

    // load R31 with address of y[.] vector
    ld_r31_y_upper_20_macro;
    add_r31_r31_imm_y_lower_12_macro;
    if (y_lower_12 & 0x800) sub_r31_r31_r12_macro;

outer_loop:
    // load rows[i] - R25
    ldw_r25_r24_imm_0_macro;
    // increment R24 by 4
    incr_r24_by_4_macro;
    // load rows[i+1] - R26
    ldw_r26_r24_imm_0_macro;

    // compute nnz - R26 - R25 into R25
    sub_r25_r26_r25_macro;

    // compute num iterations
    // divide nnz by vector width (8)
    // this is just a shift right by 3
    // followed by a check if nnz % 8 leaves a reminder
    // for the last extra iteration
    
    srl_r26_r25_imm_1_macro;
    //nop;
    
    // for the inner loop on number of iterations
    // we need to set j to 0.
    set_r19_0_macro; 
    //nop;

    // reset V30 that holds the sum to be output to Y[.]
    reset_v30_r0_macro;

    // check if j == number of iterations
    // jump to 'post_inner_loop'
    beq_r19_r26_imm_pos_48_macro;
    //nop;

    // set s to 0
    set_r20_0_macro; 

inner_loop:
    // read vals[k] through vals[k+vector_size-1] as a vector load
    vlwv_v27_r27_macro;

    // increment vals address by vector size * 4 bytes
    incr8_r27_macro; 

    // read cols[k] through cols[k+vector_size-1] as a vector load
    vlwv_v28_r28_macro;

    // use indexed loads v[cols[k]] through v[cols[k+vector_size-1]]
    // as another vector indexed load
    // note that the indexes must be address offsets.
    // what we have here are indices of columns and not address offsets.
    // as the offset is 4x column index (since data type is int),
    // we just scale the cols by 4x.
    vsll_vi_v28_v28_imm_4_macro;
    vliwv_v29_r29_v28_macro; // v[.] loaded to V29
    vmul_vv_v29_v27_v29_vm_macro; // pairwise multiply of vals[.] and v[.] into V29
    vredsum_vs_v30_v30_v29_macro; // cumulate into v30

    // increment cols address by vector size * 4 bytes
    incr8_r28_macro; 

    // increment j
    incr_r19_macro; 
    //nop;

    // branch back to start of inner loop if j < number of iterations
    blt_r19_r26_imm_neg_36_macro;
    //nop;

post_inner_loop:

    // store Y[.] 
    vswv_r31_v30_macro;

    // move Y address forward by 4
    incr4_r31_macro;

    // reset V30 that holds the sum to be output to Y[.]
    reset_v30_r0_macro;

    incr_r18_macro; 

    // branch back to start of outer loop if i < n
    blt_r18_r23_imm_neg_92_macro;
}

void execCSRVector_V4(void)
{
    // initialize locals i, j, s, k, vm;
    // i maps to R18
    // j maps to R19
    // s maps to R20
    // k maps to R21
    // vm maps to R22
    // n maps to R23
    // rows address maps to R24
    // rows[i] maps to R25
    // rows[i+1] maps to R26
    // nnz maps to R25 - dont need old R25
    // num_iters maps to R26
    // address of vals maps to R27
    
    // i,j, s, k, vm to 0
    set_r18_0_macro; 
    set_r19_0_macro; 
    set_r20_0_macro; 
    set_r21_0_macro; 
    set_r22_0_macro; 

    // load address of n to R23
    ld_r23_n_upper_20_macro;
    add_r23_r23_imm_n_lower_12_macro;
    ld_r12_imm_0xfffff_macro;
    if (n_lower_12 & 0x800) sub_r23_r23_r12_macro;

    // load value of n to R23 (do not need address of n anymore)
    ldw_r23_r23_imm_0_macro;

    // VL: 128 bits
    add_r6_r0_imm_128_macro;

    // SEW: 32 bits
    vsetvli_r7_r6_e32_macro;

    // outer loop on n.
    // for (int i=0;i<n;i++)
    // do the increment of i and then compare

    // load R24 with address of rows
    ld_r24_rows_upper_20_macro;
    add_r24_r24_imm_rows_lower_12_macro;
    if (rows_lower_12 & 0x800) sub_r24_r24_r12_macro;

    // load R27 with address of vals
    ld_r27_vals_upper_20_macro;
    add_r27_r27_imm_vals_lower_12_macro;
    if (vals_lower_12 & 0x800) sub_r27_r27_r12_macro;

    // load R28 with address of cols
    ld_r28_cols_upper_20_macro;
    add_r28_r28_imm_cols_lower_12_macro;
    if (cols_lower_12 & 0x800) sub_r28_r28_r12_macro;

    // load R29 with address of v[.] vector
    ld_r29_v_upper_20_macro;
    add_r29_r29_imm_v_lower_12_macro;
    if (v_lower_12 & 0x800) sub_r29_r29_r12_macro;

    // load R31 with address of y[.] vector
    ld_r31_y_upper_20_macro;
    add_r31_r31_imm_y_lower_12_macro;
    if (y_lower_12 & 0x800) sub_r31_r31_r12_macro;

outer_loop:
    // load rows[i] - R25
    ldw_r25_r24_imm_0_macro;
    // increment R24 by 4
    incr_r24_by_4_macro;
    // load rows[i+1] - R26
    ldw_r26_r24_imm_0_macro;

    // compute nnz - R26 - R25 into R25
    sub_r25_r26_r25_macro;

    // compute num iterations
    // divide nnz by vector width (8)
    // this is just a shift right by 3
    // followed by a check if nnz % 8 leaves a reminder
    // for the last extra iteration
    
    srl_r26_r25_imm_2_macro;
    //nop;
    
    // for the inner loop on number of iterations
    // we need to set j to 0.
    set_r19_0_macro; 
    //nop;

    // reset V30 that holds the sum to be output to Y[.]
    reset_v30_r0_macro;

    // check if j == number of iterations
    // jump to 'post_inner_loop'
    beq_r19_r26_imm_pos_48_macro;
    //nop;

    // set s to 0
    set_r20_0_macro; 

inner_loop:
    // read vals[k] through vals[k+vector_size-1] as a vector load
    vlwv_v27_r27_macro;

    // increment vals address by vector size * 4 bytes
    incr16_r27_macro; 

    // read cols[k] through cols[k+vector_size-1] as a vector load
    vlwv_v28_r28_macro;

    // use indexed loads v[cols[k]] through v[cols[k+vector_size-1]]
    // as another vector indexed load
    // note that the indexes must be address offsets.
    // what we have here are indices of columns and not address offsets.
    // as the offset is 4x column index (since data type is int),
    // we just scale the cols by 4x.
    vsll_vi_v28_v28_imm_4_macro;
    vliwv_v29_r29_v28_macro; // v[.] loaded to V29
    vmul_vv_v29_v27_v29_vm_macro; // pairwise multiply of vals[.] and v[.] into V29
    vredsum_vs_v30_v30_v29_macro; // cumulate into v30

    // increment cols address by vector size * 4 bytes
    incr16_r28_macro; 

    // increment j
    incr_r19_macro; 
    //nop;

    // branch back to start of inner loop if j < number of iterations
    blt_r19_r26_imm_neg_36_macro;
    //nop;

post_inner_loop:

    // store Y[.] 
    vswv_r31_v30_macro;

    // move Y address forward by 4
    incr4_r31_macro;

    // reset V30 that holds the sum to be output to Y[.]
    reset_v30_r0_macro;

    incr_r18_macro; 

    // branch back to start of outer loop if i < n
    blt_r18_r23_imm_neg_92_macro;
}

void execCSRVector_V8(void)
{
    // initialize locals i, j, s, k, vm;
    // i maps to R18
    // j maps to R19
    // s maps to R20
    // k maps to R21
    // vm maps to R22
    // n maps to R23
    // rows address maps to R24
    // rows[i] maps to R25
    // rows[i+1] maps to R26
    // nnz maps to R25 - dont need old R25
    // num_iters maps to R26
    // address of vals maps to R27
    
    // i,j, s, k, vm to 0
    set_r18_0_macro; 
    set_r19_0_macro; 
    set_r20_0_macro; 
    set_r21_0_macro; 
    set_r22_0_macro; 

    // load address of n to R23
    ld_r23_n_upper_20_macro;
    add_r23_r23_imm_n_lower_12_macro;
    ld_r12_imm_0xfffff_macro;
    if (n_lower_12 & 0x800) sub_r23_r23_r12_macro;

    // load value of n to R23 (do not need address of n anymore)
    ldw_r23_r23_imm_0_macro;

    // VL: 256 bits
    add_r6_r0_imm_256_macro;

    // SEW: 32 bits
    vsetvli_r7_r6_e32_macro;

    // outer loop on n.
    // for (int i=0;i<n;i++)
    // do the increment of i and then compare

    // load R24 with address of rows
    ld_r24_rows_upper_20_macro;
    add_r24_r24_imm_rows_lower_12_macro;
    if (rows_lower_12 & 0x800) sub_r24_r24_r12_macro;

    // load R27 with address of vals
    ld_r27_vals_upper_20_macro;
    add_r27_r27_imm_vals_lower_12_macro;
    if (vals_lower_12 & 0x800) sub_r27_r27_r12_macro;

    // load R28 with address of cols
    ld_r28_cols_upper_20_macro;
    add_r28_r28_imm_cols_lower_12_macro;
    if (cols_lower_12 & 0x800) sub_r28_r28_r12_macro;

    // load R29 with address of v[.] vector
    ld_r29_v_upper_20_macro;
    add_r29_r29_imm_v_lower_12_macro;
    if (v_lower_12 & 0x800) sub_r29_r29_r12_macro;

	// load R31 with address of y[.] vector
    ld_r31_prefetch_base_upper_20_macro;
    add_r31_r31_imm_prefetch_base_lower_12_macro;
    if (prefetch_base_lower_12 & 0x800) sub_r31_r31_r12_macro;	

	//These addresses wil be used if prefetcher is available
	//col
	stw_r31_r28_imm_0_macro;
	//val
	stw_r31_r27_imm_4_macro;
	//V
	stw_r31_r29_imm_8_macro;

    // load R31 with address of y[.] vector
    ld_r31_y_upper_20_macro;
    add_r31_r31_imm_y_lower_12_macro;
    if (y_lower_12 & 0x800) sub_r31_r31_r12_macro;

	//printf("CSR*****************\n");



outer_loop:
    // load rows[i] - R25
    ldw_r25_r24_imm_0_macro;
    // increment R24 by 4
    incr_r24_by_4_macro;
    // load rows[i+1] - R26
    ldw_r26_r24_imm_0_macro;

    // compute nnz - R26 - R25 into R25
    sub_r25_r26_r25_macro;

    // compute num iterations
    // divide nnz by vector width (8)
    // this is just a shift right by 3
    // followed by a check if nnz % 8 leaves a reminder
    // for the last extra iteration
    
    srl_r26_r25_imm_3_macro;
    //nop;
    
    // for the inner loop on number of iterations
    // we need to set j to 0.
    set_r19_0_macro; 
    //nop;

    // reset V30 that holds the sum to be output to Y[.]
    reset_v30_r0_macro;

    // check if j == number of iterations
    // jump to 'post_inner_loop'
    beq_r19_r26_imm_pos_48_macro;
    //nop;

    // set s to 0
    set_r20_0_macro; 

inner_loop:
    // read vals[k] through vals[k+7] as a vector load
    vlwv_v27_r27_macro;

    // increment vals address by 32 bytes
    incr32_r27_macro; 

    // read cols[k] through cols[k+7] as a vector load
    vlwv_v28_r28_macro;

    // use indexed loads v[cols[k]] through v[cols[k+7]]
    // as another vector indexed load
    // note that the indexes must be address offsets.
    // what we have here are indices of columns and not address offsets.
    // as the offset is 4x column index (since data type is int),
    // we just scale the cols by 4x.
    vsll_vi_v28_v28_imm_4_macro;
    vliwv_v29_r29_v28_macro; // v[.] loaded to V29
    vmul_vv_v29_v27_v29_vm_macro; // pairwise multiply of vals[.] and v[.] into V29
    vredsum_vs_v30_v30_v29_macro; // cumulate into v30

    // increment cols address by 32 bytes
    incr32_r28_macro; 

    // increment j
    incr_r19_macro; 
    //nop;

    // branch back to start of inner loop if j < number of iterations
    blt_r19_r26_imm_neg_36_macro;
    //nop;

post_inner_loop:

    // store Y[.] 
    vswv_r31_v30_macro;

    // move Y address forward by 4
    incr4_r31_macro;

    // reset V30 that holds the sum to be output to Y[.]
    reset_v30_r0_macro;

    incr_r18_macro; 

    // branch back to start of outer loop if i < n
    blt_r18_r23_imm_neg_92_macro;
    //nop;

    // for verification - store R18 to some unused variable t.
    // use R24 to hold address of t.
    ld_r24_t_upper_20_macro;
    add_r24_r24_imm_t_lower_12_macro;
    if (t_lower_12 & 0x800) sub_r24_r24_r12_macro;
    vswv_r24_v28_macro;
    //stw_r24_r25_imm_0_macro;
}

void HHTInit(int n, int m_rows_base, int m_cols_base, int v_base)
{
    // store n 
    *(HHT_BASE) = n;

    // store rows - address of rows[]
    *(HHT_BASE + 1) = m_rows_base;

    // store cols - address of cols[]
    *(HHT_BASE + 2) = m_cols_base;

    // store v  - address of v[]
    *(HHT_BASE + 3) = v_base;

    // store 0  -- buffer id to use
    *(HHT_BASE + 4) = 0;

    // store 1 -- start bit 
    *(HHT_BASE + 6) = 1;
}

void execHWHelperScalar(void)
{
    int s=0;
    int k=0;
    HHTInit(n, (int)rows, (int)cols, (int)v);
    volatile int * BUFFER = HHT_BASE + 1024/sizeof(int);
    for (int i=0;i<n;i++)
    {
        int nnz = rows[i+1]-rows[i];
        s=0;
        for (int j=0;j<nnz;j++)
        {
            //s+=vals[k+j]*v[cols[k+j]];
            s+=vals[k+j]*(*BUFFER);
        }
        k+=nnz;
        y[i]=s;
    }
}

/*void execHWHelper_V2(void)
{
    // initialize locals i, j, s, k, vm;
    // i maps to R18
    // j maps to R19
    // s maps to R20
    // k maps to R21
    // vm maps to R22
    // n maps to R23
    // rows address maps to R24
    // rows[i] maps to R25
    // rows[i+1] maps to R26
    // nnz maps to R25 - dont need old R25
    // num_iters maps to R26
    // address of vals maps to R27
    
    // i,j, s, k, vm to 0
    set_r18_0_macro; 
    set_r19_0_macro; 
    set_r20_0_macro; 
    set_r21_0_macro; 
    set_r22_0_macro; 

    // load address of n to R23
    ld_r23_n_upper_20_macro;
    add_r23_r23_imm_n_lower_12_macro;
    ld_r12_imm_0xfffff_macro;
    if (n_lower_12 & 0x800) sub_r23_r23_r12_macro;

    // load value of n to R23 (do not need address of n anymore)
    ldw_r23_r23_imm_0_macro;

    // VL: 64 bits
    add_r6_r0_imm_64_macro;

    // SEW: 32 bits
    vsetvli_r7_r6_e32_macro;

    // outer loop on n.
    // for (int i=0;i<n;i++)
    // do the increment of i and then compare

    // load R24 with address of rows
    ld_r24_rows_upper_20_macro;
    add_r24_r24_imm_rows_lower_12_macro;
    if (rows_lower_12 & 0x800) sub_r24_r24_r12_macro;

    // load R27 with address of vals
    ld_r27_vals_upper_20_macro;
    add_r27_r27_imm_vals_lower_12_macro;
    if (vals_lower_12 & 0x800) sub_r27_r27_r12_macro;

    // load R28 with address of cols
    ld_r28_cols_upper_20_macro;
    add_r28_r28_imm_cols_lower_12_macro;
    if (cols_lower_12 & 0x800) sub_r28_r28_r12_macro;

    // load R29 with address of v[.] vector
    ld_r29_v_upper_20_macro;
    add_r29_r29_imm_v_lower_12_macro;
    if (v_lower_12 & 0x800) sub_r29_r29_r12_macro;

    // load R31 with address of y[.] vector
    ld_r31_y_upper_20_macro;
    add_r31_r31_imm_y_lower_12_macro;
    if (y_lower_12 & 0x800) sub_r31_r31_r12_macro;

hw_helper_init:
    // program the HW helper with the following:
    // HW helper is memory mapped to locations [HELPER_BASE, HELPER_BASE+8KB]
    // the base addresses of these writes are also specified below
    // HELPER_BASE: n: number of rows
    // HELPER_BASE+4: rows: address of rows[.] vector
    // HELPER_BASE+8: cols: address of cols[.] vector
    // HELPER_BASE+12: v: address of v[.] vector where v is 
    // the vector corresponding to the operation M*v, M is the sparse
    // matrix.
    // HELPER_BASE+16: buffer id to be used to fill out values from v[.]
    // HELPER_BASE+20: reserved
    // HELPER_BASE+24: start bit. Writing a 1 to this location will
    // prompt the buffer controller to start processing.
    
    // load the value of HELPER_BASE into R30
    ld_r30_helper_base_upper_20_macro;
    add_r30_r30_imm_helper_base_lower_12_macro;
    if (helper_base_lower_12 & 0x800) sub_r30_r30_r12_macro;

    // store n 
    stw_r30_r23_imm_0_macro;

    // store rows - address of rows[]
    stw_r30_r24_imm_4_macro;

    // store cols - address of cols[]
    stw_r30_r28_imm_8_macro;

    // store v  - address of v[]
    stw_r30_r29_imm_12_macro;

    // store 0  -- buffer id to use
    stw_r30_r0_imm_16_macro;

    // store 1 -- start bit 
    add_r22_r0_imm_1_macro;
    stw_r30_r22_imm_24_macro;

    // now move R30 by 1024 bytes to point to
    // the buffer base
    add_r30_r30_imm_1024_macro;

outer_loop:
    // load rows[i] - R25
    ldw_r25_r24_imm_0_macro;
    // increment R24 by 4
    incr_r24_by_4_macro;
    // load rows[i+1] - R26
    ldw_r26_r24_imm_0_macro;

    // compute nnz - R26 - R25 into R25
    sub_r25_r26_r25_macro;

    // compute num iterations
    // divide nnz by vector width (8)
    // this is just a shift right 
    
    srl_r26_r25_imm_1_macro;
    //nop;
    
    // for the inner loop on number of iterations
    // we need to set j to 0.
    set_r19_0_macro; 
    //nop;

    // reset V30 that holds the sum to be output to Y[.]
    reset_v30_r0_macro;

    // check if j == number of iterations
    // jump to 'post_inner_loop'
    beq_r19_r26_imm_pos_36_macro;
    //nop;

    // set s to 0
    set_r20_0_macro; 

inner_loop:
    // read vals[k] through vals[k+vector_size-1] as a vector load
    vlwv_v27_r27_macro;

    // increment vals address by vector_size*4 bytes
    incr8_r27_macro; 

    // special buffered load
    vlwv_v29_r30_macro; // v[.] loaded to V29 via buffer load
    vmul_vv_v29_v27_v29_vm_macro; // pairwise multiply of vals[.] and v[.] into V29 
    vredsum_vs_v30_v30_v29_macro; // cumulate into v30

    // increment j
    incr_r19_macro; 
    //nop;

    // branch back to start of inner loop if j < number of iterations
    blt_r19_r26_imm_neg_24_macro;
    //nop;

post_inner_loop:

    // store Y[.] 
    vswv_r31_v30_macro;

    // move Y address forward by 4
    incr4_r31_macro;

    // reset V30 that holds the sum to be output to Y[.]
    reset_v30_r0_macro;

    incr_r18_macro; 

    // branch back to start of outer loop if i < n
    blt_r18_r23_imm_neg_80_macro;
    //nop;

    // for verification - store R18 to some unused variable t.
    // use R24 to hold address of t.
    ld_r24_t_upper_20_macro;
    add_r24_r24_imm_t_lower_12_macro;
    if (t_lower_12 & 0x800) sub_r24_r24_r12_macro;
    vswv_r24_v28_macro;
}

void execHWHelper_V4(void)
{
    // initialize locals i, j, s, k, vm;
    // i maps to R18
    // j maps to R19
    // s maps to R20
    // k maps to R21
    // vm maps to R22
    // n maps to R23
    // rows address maps to R24
    // rows[i] maps to R25
    // rows[i+1] maps to R26
    // nnz maps to R25 - dont need old R25
    // num_iters maps to R26
    // address of vals maps to R27
    
    // i,j, s, k, vm to 0
    set_r18_0_macro; 
    set_r19_0_macro; 
    set_r20_0_macro; 
    set_r21_0_macro; 
    set_r22_0_macro; 

    // load address of n to R23
    ld_r23_n_upper_20_macro;
    add_r23_r23_imm_n_lower_12_macro;
    ld_r12_imm_0xfffff_macro;
    if (n_lower_12 & 0x800) sub_r23_r23_r12_macro;

    // load value of n to R23 (do not need address of n anymore)
    ldw_r23_r23_imm_0_macro;

    // VL: 128 bits
    add_r6_r0_imm_128_macro;

    // SEW: 32 bits
    vsetvli_r7_r6_e32_macro;

    // outer loop on n.
    // for (int i=0;i<n;i++)
    // do the increment of i and then compare

    // load R24 with address of rows
    ld_r24_rows_upper_20_macro;
    add_r24_r24_imm_rows_lower_12_macro;
    if (rows_lower_12 & 0x800) sub_r24_r24_r12_macro;

    // load R27 with address of vals
    ld_r27_vals_upper_20_macro;
    add_r27_r27_imm_vals_lower_12_macro;
    if (vals_lower_12 & 0x800) sub_r27_r27_r12_macro;

    // load R28 with address of cols
    ld_r28_cols_upper_20_macro;
    add_r28_r28_imm_cols_lower_12_macro;
    if (cols_lower_12 & 0x800) sub_r28_r28_r12_macro;

    // load R29 with address of v[.] vector
    ld_r29_v_upper_20_macro;
    add_r29_r29_imm_v_lower_12_macro;
    if (v_lower_12 & 0x800) sub_r29_r29_r12_macro;

    // load R31 with address of y[.] vector
    ld_r31_y_upper_20_macro;
    add_r31_r31_imm_y_lower_12_macro;
    if (y_lower_12 & 0x800) sub_r31_r31_r12_macro;

hw_helper_init:
    // program the HW helper with the following:
    // HW helper is memory mapped to locations [HELPER_BASE, HELPER_BASE+8KB]
    // the base addresses of these writes are also specified below
    // HELPER_BASE: n: number of rows
    // HELPER_BASE+4: rows: address of rows[.] vector
    // HELPER_BASE+8: cols: address of cols[.] vector
    // HELPER_BASE+12: v: address of v[.] vector where v is 
    // the vector corresponding to the operation M*v, M is the sparse
    // matrix.
    // HELPER_BASE+16: buffer id to be used to fill out values from v[.]
    // HELPER_BASE+20: reserved
    // HELPER_BASE+24: start bit. Writing a 1 to this location will
    // prompt the buffer controller to start processing.
    
    // load the value of HELPER_BASE into R30
    ld_r30_helper_base_upper_20_macro;
    add_r30_r30_imm_helper_base_lower_12_macro;
    if (helper_base_lower_12 & 0x800) sub_r30_r30_r12_macro;

    // store n 
    stw_r30_r23_imm_0_macro;

    // store rows - address of rows[]
    stw_r30_r24_imm_4_macro;

    // store cols - address of cols[]
    stw_r30_r28_imm_8_macro;

    // store v  - address of v[]
    stw_r30_r29_imm_12_macro;

    // store 0  -- buffer id to use
    stw_r30_r0_imm_16_macro;

    // store 1 -- start bit 
    add_r22_r0_imm_1_macro;
    stw_r30_r22_imm_24_macro;

    // now move R30 by 1024 bytes to point to
    // the buffer base
    add_r30_r30_imm_1024_macro;

outer_loop:
    // load rows[i] - R25
    ldw_r25_r24_imm_0_macro;
    // increment R24 by 4
    incr_r24_by_4_macro;
    // load rows[i+1] - R26
    ldw_r26_r24_imm_0_macro;

    // compute nnz - R26 - R25 into R25
    sub_r25_r26_r25_macro;

    // compute num iterations
    // divide nnz by vector width
    
    srl_r26_r25_imm_2_macro;
    //nop;
    
    // for the inner loop on number of iterations
    // we need to set j to 0.
    set_r19_0_macro; 
    //nop;

    // reset V30 that holds the sum to be output to Y[.]
    reset_v30_r0_macro;

    // check if j == number of iterations
    // jump to 'post_inner_loop'
    beq_r19_r26_imm_pos_36_macro;
    //nop;

    // set s to 0
    set_r20_0_macro; 

inner_loop:
    // read vals[k] through vals[k+vector_size-1] as a vector load
    vlwv_v27_r27_macro;

    // increment vals address by vector_size*4 bytes
    incr16_r27_macro; 

    // special buffered load
    vlwv_v29_r30_macro; // v[.] loaded to V29 via buffer load
    vmul_vv_v29_v27_v29_vm_macro; // pairwise multiply of vals[.] and v[.] into V29 
    vredsum_vs_v30_v30_v29_macro; // cumulate into v30

    // increment j
    incr_r19_macro; 
    //nop;

    // branch back to start of inner loop if j < number of iterations
    blt_r19_r26_imm_neg_24_macro;
    //nop;

post_inner_loop:

    // store Y[.] 
    vswv_r31_v30_macro;

    // move Y address forward by 4
    incr4_r31_macro;

    // reset V30 that holds the sum to be output to Y[.]
    reset_v30_r0_macro;

    incr_r18_macro; 

    // branch back to start of outer loop if i < n
    blt_r18_r23_imm_neg_80_macro;
    //nop;

    // for verification - store R18 to some unused variable t.
    // use R24 to hold address of t.
    ld_r24_t_upper_20_macro;
    add_r24_r24_imm_t_lower_12_macro;
    if (t_lower_12 & 0x800) sub_r24_r24_r12_macro;
    vswv_r24_v28_macro;
}

void execHWHelper_V8(void)
{
    // initialize locals i, j, s, k, vm;
    // i maps to R18
    // j maps to R19
    // s maps to R20
    // k maps to R21
    // vm maps to R22
    // n maps to R23
    // rows address maps to R24
    // rows[i] maps to R25
    // rows[i+1] maps to R26
    // nnz maps to R25 - dont need old R25
    // num_iters maps to R26
    // address of vals maps to R27
    
    // i,j, s, k, vm to 0
    set_r18_0_macro; 
    set_r19_0_macro; 
    set_r20_0_macro; 
    set_r21_0_macro; 
    set_r22_0_macro; 

    // load address of n to R23
    ld_r23_n_upper_20_macro;
    add_r23_r23_imm_n_lower_12_macro;
    ld_r12_imm_0xfffff_macro;
    if (n_lower_12 & 0x800) sub_r23_r23_r12_macro;

    // load value of n to R23 (do not need address of n anymore)
    ldw_r23_r23_imm_0_macro;

    // VL: 256 bits
    add_r6_r0_imm_256_macro;

    // SEW: 32 bits
    vsetvli_r7_r6_e32_macro;

    // outer loop on n.
    // for (int i=0;i<n;i++)
    // do the increment of i and then compare

    // load R24 with address of rows
    ld_r24_rows_upper_20_macro;
    add_r24_r24_imm_rows_lower_12_macro;
    if (rows_lower_12 & 0x800) sub_r24_r24_r12_macro;

    // load R27 with address of vals
    ld_r27_vals_upper_20_macro;
    add_r27_r27_imm_vals_lower_12_macro;
    if (vals_lower_12 & 0x800) sub_r27_r27_r12_macro;

    // load R28 with address of cols
    ld_r28_cols_upper_20_macro;
    add_r28_r28_imm_cols_lower_12_macro;
    if (cols_lower_12 & 0x800) sub_r28_r28_r12_macro;

    // load R29 with address of v[.] vector
    ld_r29_v_upper_20_macro;
    add_r29_r29_imm_v_lower_12_macro;
    if (v_lower_12 & 0x800) sub_r29_r29_r12_macro;

    // load R31 with address of y[.] vector
    ld_r31_y_upper_20_macro;
    add_r31_r31_imm_y_lower_12_macro;
    if (y_lower_12 & 0x800) sub_r31_r31_r12_macro;

hw_helper_init:
    // program the HW helper with the following:
    // HW helper is memory mapped to locations [HELPER_BASE, HELPER_BASE+8KB]
    // the base addresses of these writes are also specified below
    // HELPER_BASE: n: number of rows
    // HELPER_BASE+4: rows: address of rows[.] vector
    // HELPER_BASE+8: cols: address of cols[.] vector
    // HELPER_BASE+12: v: address of v[.] vector where v is 
    // the vector corresponding to the operation M*v, M is the sparse
    // matrix.
    // HELPER_BASE+16: buffer id to be used to fill out values from v[.]
    // HELPER_BASE+20: reserved
    // HELPER_BASE+24: start bit. Writing a 1 to this location will
    // prompt the buffer controller to start processing.
    
    // load the value of HELPER_BASE into R30
    ld_r30_helper_base_upper_20_macro;
    add_r30_r30_imm_helper_base_lower_12_macro;
    if (helper_base_lower_12 & 0x800) sub_r30_r30_r12_macro;

    // store n 
    stw_r30_r23_imm_0_macro;

    // store rows - address of rows[]
    stw_r30_r24_imm_4_macro;

    // store cols - address of cols[]
    stw_r30_r28_imm_8_macro;

    // store v  - address of v[]
    stw_r30_r29_imm_12_macro;

    // store 0  -- buffer id to use
    stw_r30_r0_imm_16_macro;

    // store 1 -- start bit 
    add_r22_r0_imm_1_macro;
    stw_r30_r22_imm_24_macro;

    // now move R30 by 1024 bytes to point to
    // the buffer base
    add_r30_r30_imm_1024_macro;

outer_loop:
    // load rows[i] - R25
    ldw_r25_r24_imm_0_macro;
    // increment R24 by 4
    incr_r24_by_4_macro;
    // load rows[i+1] - R26
    ldw_r26_r24_imm_0_macro;

    // compute nnz - R26 - R25 into R25
    sub_r25_r26_r25_macro;

    // compute num iterations
    // divide nnz by vector width (8)
    // this is just a shift right by 3
    // followed by a check if nnz % 8 leaves a reminder
    // for the last extra iteration
    
    srl_r26_r25_imm_3_macro;
    //nop;
    
    // for the inner loop on number of iterations
    // we need to set j to 0.
    set_r19_0_macro; 
    //nop;

    // reset V30 that holds the sum to be output to Y[.]
    reset_v30_r0_macro;

    // check if j == number of iterations
    // jump to 'post_inner_loop'
    beq_r19_r26_imm_pos_36_macro;
    //nop;

    // set s to 0
    set_r20_0_macro; 

inner_loop:
    // read vals[k] through vals[k+7] as a vector load
    vlwv_v27_r27_macro;

    // increment vals address by 32 bytes
    incr32_r27_macro; 

    // special buffered load
    vlwv_v29_r30_macro; // v[.] loaded to V29 via buffer load
    vmul_vv_v29_v27_v29_vm_macro; // pairwise multiply of vals[.] and v[.] into V29 
    vredsum_vs_v30_v30_v29_macro; // cumulate into v30

    // increment j
    incr_r19_macro; 
    //nop;

    // branch back to start of inner loop if j < number of iterations
    blt_r19_r26_imm_neg_24_macro;
    //nop;

post_inner_loop:

    // store Y[.] 
    vswv_r31_v30_macro;

    // move Y address forward by 4
    incr4_r31_macro;

    // reset V30 that holds the sum to be output to Y[.]
    reset_v30_r0_macro;

    incr_r18_macro; 

    // branch back to start of outer loop if i < n
    blt_r18_r23_imm_neg_80_macro;
    //nop;

    // for verification - store R18 to some unused variable t.
    // use R24 to hold address of t.
    ld_r24_t_upper_20_macro;
    add_r24_r24_imm_t_lower_12_macro;
    if (t_lower_12 & 0x800) sub_r24_r24_r12_macro;
    vswv_r24_v28_macro;
}*/
#endif

void execDenseScalar(void)
{
    int s;
    for (int i=0;i<n;i++)
    {
        s = 0;
        /*
        for (int j=0;j<n;j++)
        {
            s += m[i*n+j]*v[j];
        }
        */
        for (int j=0;j<n;j+=8)
        {
            s += m[i*n+j]*v[j];
            s += m[i*n+j+1]*v[j+1];
            s += m[i*n+j+2]*v[j+2];
            s += m[i*n+j+3]*v[j+3];
            s += m[i*n+j+4]*v[j+4];
            s += m[i*n+j+5]*v[j+5];
            s += m[i*n+j+6]*v[j+6];
            s += m[i*n+j+7]*v[j+7];
        }
        y[i] = s;
    }
}

void genDenseVector(void)
{
    generateAsm("add_r6_r0_imm_256_macro", ADD_IMM(R6,R0,256));
    generateAsm("vsetvli_r7_r6_e32_macro", VSETVLI(R7, R6, (E32 << VSEW_OFFSET)));

    int m_upper_20 = (int)(&m[0]);
    m_lower_12 = (int)(&m[0]);
    m_upper_20 = m_upper_20 >> 12;
    m_lower_12 = m_lower_12 & 0xfff;
    generateAsm("ld_r10_m_upper_20_macro", LD_IMM(R10, m_upper_20));
    generateAsm("add_r10_r10_imm_m_lower_12_macro", ADD_IMM(R10, R10, m_lower_12));

    int v_upper_20 = (int)(&v[0]);
    v_lower_12 = (int)(&v[0]);
    v_upper_20 = v_upper_20 >> 12;
    v_lower_12 = v_lower_12 & 0xfff;
    generateAsm("ld_r13_v_upper_20_macro", LD_IMM(R13, v_upper_20));
    generateAsm("add_r13_r13_imm_v_lower_12_macro", ADD_IMM(R13, R13, v_lower_12));

    int y_upper_20 = (int)(&y[0]);
    y_lower_12 = (int)(&y[0]);
    y_upper_20 = y_upper_20 >> 12;
    y_lower_12 = y_lower_12 & 0xfff;
    generateAsm("ld_r28_y_upper_20_macro", LD_IMM(R28, y_upper_20));
    generateAsm("add_r28_r28_imm_y_lower_12_macro", ADD_IMM(R28, R28, y_lower_12));

    generateAsm("ld_r12_imm_0xfffff_macro", LD_IMM(R12, 0xfffff));
    generateAsm("sub_r10_r10_r12_macro", SUB_R_R(R10,R10,R12));
    generateAsm("sub_r11_r11_r12_macro", SUB_R_R(R11,R11,R12));
    generateAsm("sub_r13_r13_r12_macro", SUB_R_R(R13,R13,R12));
    generateAsm("sub_r28_r28_r12_macro", SUB_R_R(R28,R28,R12));

    generateAsm("vlwv_v10_r10_macro", VLWV(V10,R10));
    generateAsm("vlwv_v13_r13_macro", VLWV(V13,R13));
    generateAsm("vadd_vv_v11_v10_v13_vm_macro", VADD_VV(V11,V10,V13,VM));
    generateAsm("vmul_vv_v11_v10_v13_vm_macro", VMUL_VV(V11,V10,V13,VM));
    generateAsm("vswv_r11_v11_macro", VSWV(R11,V11));
    generateAsm("vswv_r28_v28_macro", VSWV(R28,V28));
    generateAsm("add_10_10_imm_256_macro", ADD_IMM(R10,R10,0b000000100000));
    generateAsm("add_11_11_imm_256_macro", ADD_IMM(R11,R11,0b000000100000));
    generateAsm("add_10_10_imm_128_macro", ADD_IMM(R10,R10,0b000000010000));
    generateAsm("add_11_11_imm_128_macro", ADD_IMM(R11,R11,0b000000010000));
    generateAsm("add_10_10_imm_64_macro", ADD_IMM(R10,R10,0b000000001000));
    generateAsm("add_11_11_imm_64_macro", ADD_IMM(R11,R11,0b000000001000));
    generateAsm("add_11_11_imm_32_macro", ADD_IMM(R11,R11,0b000000000100));
    generateAsm("add_13_13_imm_256_macro", ADD_IMM(R13,R13,0b000000100000));
    generateAsm("add_13_13_imm_128_macro", ADD_IMM(R13,R13,0b000000010000));
    generateAsm("add_13_13_imm_64_macro", ADD_IMM(R13,R13,0b000000001000));
    generateAsm("add_28_28_imm_256_macro", ADD_IMM(R28,R28,0b000000100000));
    generateAsm("add_28_28_imm_32_macro", ADD_IMM(R28,R28,0b000000000100));
    // use V28 to hold cumulation
    generateAsm("vredsum_vs_v28_v28_v11_macro", VREDSUM_VS(V28, V28, V11, VM));

    // generate a copy instruction to save contents of R13
    generateAsm("cp_r29_r13_macro", ADD_IMM(R29, R13, 0));
    // generate a copy instruction to restore contents to R13
    generateAsm("cp_r13_r29_macro", ADD_IMM(R13, R29, 0));

    // generate instruction to clear cumulator V28[0]
    generateAsm("reset_v28_r0_macro", VMV_VX(V28,R0));

    // generate instruction to save V28[0] into x register R30.
    generateAsm("move_r30_v28_macro", VMV_XV(R30,V28));

    // generate instruction to save R30 to MEM[R11].
    generateAsm("sw_r11_r30_macro", STW(R11, R30, 0));
}

#ifndef GENERATE_ONLY
void execDenseVector_V2(void)
{
    // VL: 64 bits
    add_r6_r0_imm_64_macro;

    // SEW: 32 bits
    vsetvli_r7_r6_e32_macro;

    // load address of M into r10
    ld_r10_m_upper_20_macro;
    add_r10_r10_imm_m_lower_12_macro;
    ld_r12_imm_0xfffff_macro;
    if (m_lower_12 & 0x800) sub_r10_r10_r12_macro;

    // load addres of V into r13
    ld_r13_v_upper_20_macro;
    add_r13_r13_imm_v_lower_12_macro;
    if (v_lower_12 & 0x800) sub_r13_r13_r12_macro;

    // save contents of r13 into r29 for quick inner loop
    cp_r29_r13_macro;

    // load addres of Y into r28
    ld_r28_y_upper_20_macro;
    add_r28_r28_imm_y_lower_12_macro;
    if (y_lower_12 & 0x800) sub_r28_r28_r12_macro;

    for(int i=0; i<n; i++)
    {
        // reload address of vector V into R13.
        //ld_r13_v_upper_20_macro;
        //add_r13_r13_imm_v_lower_12_macro;
        //if (v_lower_12 & 0x800) sub_r13_r13_r12_macro;
        cp_r13_r29_macro;        

        for (int j=0;j<n; j+= 2)
        {
            vlwv_v10_r10_macro; // load next vector from M[.][]
            vlwv_v13_r13_macro; // load next vector from V[.]
            vmul_vv_v11_v10_v13_vm_macro; // pairwise multiply
            vredsum_vs_v28_v28_v11_macro; // cumulate into v28
            add_10_10_imm_64_macro;  // move to next M[][] vector
            add_13_13_imm_64_macro;  // move to next V[] vector
        }
        // V28[0] holds Y[i]. Store it to Y[i].
        // we move v28[0] to R30
        // then store R30 to Y[i].
        // move_r30_v28_macro;     
        // sw_r11_r30_macro;    

        // increment address by 4
        // add_11_11_imm_32_macro; // mov to next output Y[].

        // store V28 to MEM[R28]
        vswv_r28_v28_macro;

        add_28_28_imm_32_macro;

        // clear v28[0]
        reset_v28_r0_macro;
    }
}

void execDenseVector_V4(void)
{
    // VL: 128 bits
    add_r6_r0_imm_128_macro;

    // SEW: 32 bits
    vsetvli_r7_r6_e32_macro;

    // load address of M into r10
    ld_r10_m_upper_20_macro;
    add_r10_r10_imm_m_lower_12_macro;
    ld_r12_imm_0xfffff_macro;
    if (m_lower_12 & 0x800) sub_r10_r10_r12_macro;

    // load addres of V into r13
    ld_r13_v_upper_20_macro;
    add_r13_r13_imm_v_lower_12_macro;
    if (v_lower_12 & 0x800) sub_r13_r13_r12_macro;

    // save contents of r13 into r29 for quick inner loop
    cp_r29_r13_macro;

    // load addres of Y into r28
    ld_r28_y_upper_20_macro;
    add_r28_r28_imm_y_lower_12_macro;
    if (y_lower_12 & 0x800) sub_r28_r28_r12_macro;

    for(int i=0; i<n; i++)
    {
        // reload address of vector V into R13.
        //ld_r13_v_upper_20_macro;
        //add_r13_r13_imm_v_lower_12_macro;
        //if (v_lower_12 & 0x800) sub_r13_r13_r12_macro;
        cp_r13_r29_macro;        

        for (int j=0;j<n; j+= 4)
        {
            vlwv_v10_r10_macro; // load next vector from M[.][]
            vlwv_v13_r13_macro; // load next vector from V[.]
            vmul_vv_v11_v10_v13_vm_macro; // pairwise multiply
            vredsum_vs_v28_v28_v11_macro; // cumulate into v28
            add_10_10_imm_128_macro;  // move to next M[][] vector
            add_13_13_imm_128_macro;  // move to next V[] vector
        }
        // V28[0] holds Y[i]. Store it to Y[i].
        // we move v28[0] to R30
        // then store R30 to Y[i].
        // move_r30_v28_macro;     
        // sw_r11_r30_macro;    

        // increment address by 4
        // add_11_11_imm_32_macro; // mov to next output Y[].

        // store V28 to MEM[R28]
        vswv_r28_v28_macro;

        add_28_28_imm_32_macro;

        // clear v28[0]
        reset_v28_r0_macro;
    }
}

void execDenseVector_V8(void)
{
    // VL: 256 bits
    add_r6_r0_imm_256_macro;

    // SEW: 32 bits
    vsetvli_r7_r6_e32_macro;

    // load address of M into r10
    ld_r10_m_upper_20_macro;
    add_r10_r10_imm_m_lower_12_macro;
    ld_r12_imm_0xfffff_macro;
    if (m_lower_12 & 0x800) sub_r10_r10_r12_macro;

    // load addres of V into r13
    ld_r13_v_upper_20_macro;
    add_r13_r13_imm_v_lower_12_macro;
    if (v_lower_12 & 0x800) sub_r13_r13_r12_macro;

    // save contents of r13 into r29 for quick inner loop
    cp_r29_r13_macro;

    // load addres of Y into r28
    ld_r28_y_upper_20_macro;
    add_r28_r28_imm_y_lower_12_macro;
    if (y_lower_12 & 0x800) sub_r28_r28_r12_macro;

    for(int i=0; i<n; i++)
    {
        // reload address of vector V into R13.
        //ld_r13_v_upper_20_macro;
        //add_r13_r13_imm_v_lower_12_macro;
        //if (v_lower_12 & 0x800) sub_r13_r13_r12_macro;
        cp_r13_r29_macro;        

        for (int j=0;j<n; j+= 8)
        {
            vlwv_v10_r10_macro; // load next vector from M[.][]
            vlwv_v13_r13_macro; // load next vector from V[.]
            vmul_vv_v11_v10_v13_vm_macro; // pairwise multiply
            vredsum_vs_v28_v28_v11_macro; // cumulate into v28
            add_10_10_imm_256_macro;  // move to next M[][] vector
            add_13_13_imm_256_macro;  // move to next V[] vector
        }
        // V28[0] holds Y[i]. Store it to Y[i].
        // we move v28[0] to R30
        // then store R30 to Y[i].
        // move_r30_v28_macro;     
        // sw_r11_r30_macro;    

        // increment address by 4
        // add_11_11_imm_32_macro; // mov to next output Y[].

        // store V28 to MEM[R28]
        vswv_r28_v28_macro;

        add_28_28_imm_32_macro;

        // clear v28[0]
        reset_v28_r0_macro;
    }
}

void execDenseDenseScalar(void) 
{
    execDenseScalar();
}

void execDenseDenseVector2(void) 
{
    execDenseVector_V2();
}

void execDenseDenseVector4(void) 
{
    execDenseVector_V4();
}

void execDenseDenseVector8(void) 
{
    execDenseVector_V8();
}

void execHelperDenseCSRScalar(void) 
{
	 // load address of n to R23
    ld_r23_n_upper_20_macro;
    add_r23_r23_imm_n_lower_12_macro;
    ld_r12_imm_0xfffff_macro;
    if (n_lower_12 & 0x800) sub_r23_r23_r12_macro;

    // load value of n to R23 (do not need address of n anymore)
    ldw_r23_r23_imm_0_macro;

    // VL: 64 bits
    add_r6_r0_imm_256_macro;

    // SEW: 32 bits
    vsetvli_r7_r6_e32_macro;

    //load R26 with nnz
    ld_r26_nnz_upper_20_macro;
    add_r26_r26_imm_nnz_lower_12_macro;
    if (nnz_lower_12 & 0x800) sub_r26_r26_r12_macro;

    //load R25 with bitvector base
    ld_r25_bv_upper_20_macro;
    add_r25_r25_imm_bv_lower_12_macro;
    if (bv_lower_12 & 0x800) sub_r25_r25_r12_macro;

    // load R24 with address of rows
    ld_r24_rows_upper_20_macro;
    add_r24_r24_imm_rows_lower_12_macro;
    if (rows_lower_12 & 0x800) sub_r24_r24_r12_macro;

    // load R27 with address of vals
    ld_r27_vals_upper_20_macro;
    add_r27_r27_imm_vals_lower_12_macro;
    if (vals_lower_12 & 0x800) sub_r27_r27_r12_macro;
	
	 // load R28 with address of cols
    ld_r28_cols_upper_20_macro;
    add_r28_r28_imm_cols_lower_12_macro;
    if (cols_lower_12 & 0x800) sub_r28_r28_r12_macro;

    // load R29 with address of v[.] vector
    ld_r29_v_upper_20_macro;
    add_r29_r29_imm_v_lower_12_macro;
    if (v_lower_12 & 0x800) sub_r29_r29_r12_macro;

    // load R31 with address of y[.] vector
    //ld_r31_y_upper_20_macro;
    //add_r31_r31_imm_y_lower_12_macro;
    //if (y_lower_12 & 0x800) sub_r31_r31_r12_macro;

hw_helper_init:
    // program the HW helper with the following:
    // HW helper is memory mapped to locations [HELPER_BASE, HELPER_BASE+8KB]
    // the base addresses of these writes are also specified below
    // HELPER_BASE: n: number of rows
    // HELPER_BASE+4: rows: address of rows[.] vector
    // HELPER_BASE+8: cols: address of cols[.] vector
    // HELPER_BASE+12: v: address of v[.] vector where v is
    // the vector corresponding to the operation M*v, M is the sparse
    // matrix.
    // HELPER_BASE+16: buffer id to be used to fill out values from v[.]
    // HELPER_BASE+20: reserved
    // HELPER_BASE+24: start bit. Writing a 1 to this location will
    // prompt the buffer controller to start processing.

    // load the value of HELPER_BASE into R30
    ld_r30_helper_base_upper_20_macro;
    add_r30_r30_imm_helper_base_lower_12_macro;
    if (helper_base_lower_12 & 0x800) sub_r30_r30_r12_macro;

    //Ec
    stw_r30_r0_imm_0_macro;

    //storage format
    stw_r30_r0_imm_4_macro;
	
	// store n
    stw_r30_r23_imm_8_macro;

    // store rows - address of rows[]
    stw_r30_r24_imm_12_macro;

    // store cols - address of cols[]
    stw_r30_r28_imm_16_macro;

    // store v  - address of v[]
    stw_r30_r29_imm_20_macro;

    // store vals  - address of vals[]
    stw_r30_r27_imm_24_macro;

    // store bitvector  - address of bitvector[]
    stw_r30_r25_imm_28_macro;

    // store 0  -- buffer id to use
    stw_r30_r0_imm_32_macro;

    // store nnz
    stw_r30_r26_imm_36_macro;

    // store 1 -- start bit
    add_r22_r0_imm_1_macro;
    stw_r30_r22_imm_48_macro;

    // now move R30 by 1024 bytes to point to
    // the buffer base
    add_r30_r30_imm_1024_macro;

    //copy r29 (v[]) to another register (r28) as backup
    add_r28_r29_imm_0_macro;

    int s;
    volatile int * BUFFER = HHT_BASE + 1024/sizeof(int);

    for (int i=0;i < n; i++)
	{
        s = 0;
        for (int j=0;j<n; j+=8)
        {
			s += (*BUFFER)*v[j];
            s += (*BUFFER)*v[j+1];
            s += (*BUFFER)*v[j+2];
            s += (*BUFFER)*v[j+3];
            s += (*BUFFER)*v[j+4];
            s += (*BUFFER)*v[j+5];
            s += (*BUFFER)*v[j+6];
            s += (*BUFFER)*v[j+7];
            //index++;
        }
        y[i] = s;
    }

}

void execHelperDenseCSRVector2(void) 
{
}

void execHelperDenseCSRVector4(void) 
{
}

void execHelperDenseCSRVector8(void) 
{
    // initialize locals i, j, s, k, vm;
    // i maps to R18
    // j maps to R19
    // s maps to R20
    // k maps to R21
    // vm maps to R22
    // n maps to R23
    // rows address maps to R24
    // rows[i] maps to R25
    // rows[i+1] maps to R26
    // nnz maps to R25 - dont need old R25
    // num_iters maps to R26
    // address of vals maps to R27
    
    // i,j, s, k, vm to 0
    set_r18_0_macro; 
    set_r19_0_macro; 
    set_r20_0_macro; 
    set_r21_0_macro; 
    set_r22_0_macro; 

    // load address of n to R23
    ld_r23_n_upper_20_macro;
    add_r23_r23_imm_n_lower_12_macro;
    ld_r12_imm_0xfffff_macro;
    if (n_lower_12 & 0x800) sub_r23_r23_r12_macro;

    // load value of n to R23 (do not need address of n anymore)
    ldw_r23_r23_imm_0_macro;

    // VL: 64 bits
    add_r6_r0_imm_256_macro;

    // SEW: 32 bits
    vsetvli_r7_r6_e32_macro;

    // outer loop on n.
    // for (int i=0;i<n;i++)
    // do the increment of i and then compare

	//load R26 with nnz
    ld_r26_nnz_upper_20_macro;
    add_r26_r26_imm_nnz_lower_12_macro;
    if (nnz_lower_12 & 0x800) sub_r26_r26_r12_macro;

	//load R25 with bitvector base
    ld_r25_bv_upper_20_macro;
    add_r25_r25_imm_bv_lower_12_macro;
    if (bv_lower_12 & 0x800) sub_r25_r25_r12_macro;

    // load R24 with address of rows
    ld_r24_rows_upper_20_macro;
    add_r24_r24_imm_rows_lower_12_macro;
    if (rows_lower_12 & 0x800) sub_r24_r24_r12_macro;

    // load R27 with address of vals
    ld_r27_vals_upper_20_macro;
    add_r27_r27_imm_vals_lower_12_macro;
    if (vals_lower_12 & 0x800) sub_r27_r27_r12_macro;

    // load R28 with address of cols
    ld_r28_cols_upper_20_macro;
    add_r28_r28_imm_cols_lower_12_macro;
    if (cols_lower_12 & 0x800) sub_r28_r28_r12_macro;

    // load R29 with address of v[.] vector
    ld_r29_v_upper_20_macro;
    add_r29_r29_imm_v_lower_12_macro;
    if (v_lower_12 & 0x800) sub_r29_r29_r12_macro;

    // load R31 with address of y[.] vector
    ld_r31_y_upper_20_macro;
    add_r31_r31_imm_y_lower_12_macro;
    if (y_lower_12 & 0x800) sub_r31_r31_r12_macro;

hw_helper_init:
    // program the HW helper with the following:
    // HW helper is memory mapped to locations [HELPER_BASE, HELPER_BASE+8KB]
    // the base addresses of these writes are also specified below
    // HELPER_BASE: n: number of rows
    // HELPER_BASE+4: rows: address of rows[.] vector
    // HELPER_BASE+8: cols: address of cols[.] vector
    // HELPER_BASE+12: v: address of v[.] vector where v is 
    // the vector corresponding to the operation M*v, M is the sparse
    // matrix.
    // HELPER_BASE+16: buffer id to be used to fill out values from v[.]
    // HELPER_BASE+20: reserved
    // HELPER_BASE+24: start bit. Writing a 1 to this location will
    // prompt the buffer controller to start processing.
    
    // load the value of HELPER_BASE into R30
    ld_r30_helper_base_upper_20_macro;
    add_r30_r30_imm_helper_base_lower_12_macro;
    if (helper_base_lower_12 & 0x800) sub_r30_r30_r12_macro;

	//Ec
	stw_r30_r0_imm_0_macro;	

	//storage format
	stw_r30_r0_imm_4_macro;	

    // store n 
    stw_r30_r23_imm_8_macro;

    // store rows - address of rows[]
    stw_r30_r24_imm_12_macro;

    // store cols - address of cols[]
    stw_r30_r28_imm_16_macro;

    // store v  - address of v[]
    stw_r30_r29_imm_20_macro;
    
	// store vals  - address of vals[]
    stw_r30_r27_imm_24_macro;
	
	// store bitvector  - address of bitvector[]
    stw_r30_r25_imm_28_macro;

    // store 0  -- buffer id to use
    stw_r30_r0_imm_32_macro;
    
	// store nnz
    stw_r30_r26_imm_36_macro;

    // store 1 -- start bit 
    add_r22_r0_imm_1_macro;
    stw_r30_r22_imm_48_macro;

    // now move R30 by 1024 bytes to point to
    // the buffer base
    add_r30_r30_imm_1024_macro;
	
	//copy r29 (v[]) to another register (r28) as backup		
	add_r28_r29_imm_0_macro;	

outer_loop:
    // load rows[i] - R25
    ldw_r25_r24_imm_0_macro;
    // increment R24 by 4
    incr_r24_by_4_macro;
    // load rows[i+1] - R26
    ldw_r26_r24_imm_0_macro;

    // compute nnz - R26 - R25 into R25
    //sub_r25_r26_r25_macro;

    // compute num iterations
    // divide nnz by vector width (8)
    // this is just a shift right 
	
	//for expansion, n/8
    srl_r26_r23_imm_3_macro;
    //nop;
    
    // for the inner loop on number of iterations
    // we need to set j to 0.
    set_r19_0_macro; 
    //nop;

    // reset V30 that holds the sum to be output to Y[.]
    reset_v30_r0_macro;

    // check if j == number of iterations
    // jump to 'post_inner_loop'
    beq_r19_r26_imm_pos_40_macro;
    //nop;

    // set s to 0
    set_r20_0_macro; 

inner_loop:
    // read vals[k] through vals[k+vector_size-1] as a vector load
    vlwv_v27_r29_macro;

    // increment vals address by vector_size*32 bytes
    incr32_r29_macro; 

    // special buffered load
    reset_v29_r0_macro;
    vlwv_v29_r30_macro; // v[.] loaded to V29 via buffer load
    vmul_vv_v29_v27_v29_vm_macro; // pairwise multiply of vals[.] and v[.] into V29 
    vredsum_vs_v30_v30_v29_macro; // cumulate into v30

    // increment j
    incr_r19_macro; 
    //nop;

    // branch back to start of inner loop if j < number of iterations
    blt_r19_r26_imm_neg_28_macro;
    //nop;

post_inner_loop:

    // store Y[.] 
    vswv_r31_v30_macro;

    // move Y address forward by 4
    incr4_r31_macro;

    // reset V30 that holds the sum to be output to Y[.]
    reset_v30_r0_macro;

    incr_r18_macro; 

	//sub vector base to 0
	add_r29_r28_imm_0_macro;

    // branch back to start of outer loop if i < n
    blt_r18_r23_imm_neg_84_macro;
    //nop;

    // for verification - store R18 to some unused variable t.
    // use R24 to hold address of t.
    ld_r24_t_upper_20_macro;
    add_r24_r24_imm_t_lower_12_macro;
    if (t_lower_12 & 0x800) sub_r24_r24_r12_macro;
    vswv_r24_v28_macro;

}

void execHelperDenseBVScalar(void) 
{
// load address of n to R23
    ld_r23_n_upper_20_macro;
    add_r23_r23_imm_n_lower_12_macro;
    ld_r12_imm_0xfffff_macro;
    if (n_lower_12 & 0x800) sub_r23_r23_r12_macro;

    // load value of n to R23 (do not need address of n anymore)
    ldw_r23_r23_imm_0_macro;

    // VL: 64 bits
    add_r6_r0_imm_256_macro;

    // SEW: 32 bits
    vsetvli_r7_r6_e32_macro;

    // outer loop on n.
    // for (int i=0;i<n;i++)
    // do the increment of i and then compare

	//load R26 with nnz
    ld_r26_nnz_upper_20_macro;
    add_r26_r26_imm_nnz_lower_12_macro;
    if (nnz_lower_12 & 0x800) sub_r26_r26_r12_macro;

	//load R25 with bitvector base
    ld_r25_bv_upper_20_macro;
    add_r25_r25_imm_bv_lower_12_macro;
    if (bv_lower_12 & 0x800) sub_r25_r25_r12_macro;

    // load R24 with address of rows
    ld_r24_rl_rows_upper_20_macro;
    add_r24_r24_imm_rl_rows_lower_12_macro;
    if (rl_rows_lower_12 & 0x800) sub_r24_r24_r12_macro;

    // load R27 with address of vals
    ld_r27_b_vals_upper_20_macro;
    add_r27_r27_imm_b_vals_lower_12_macro;
    if (b_vals_lower_12 & 0x800) sub_r27_r27_r12_macro;

    // load R28 with address of cols
    ld_r28_rl_cols_upper_20_macro;
    add_r28_r28_imm_rl_cols_lower_12_macro;
    if (rl_cols_lower_12 & 0x800) sub_r28_r28_r12_macro;

    // load R29 with address of v[.] vector
    ld_r29_v_upper_20_macro;
    add_r29_r29_imm_v_lower_12_macro;
    if (v_lower_12 & 0x800) sub_r29_r29_r12_macro;

    // load R31 with address of y[.] vector
    //ld_r31_y_upper_20_macro;
    //add_r31_r31_imm_y_lower_12_macro;
    //if (y_lower_12 & 0x800) sub_r31_r31_r12_macro;

hw_helper_init:
    // program the HW helper with the following:
    // HW helper is memory mapped to locations [HELPER_BASE, HELPER_BASE+8KB]
    // the base addresses of these writes are also specified below
    // HELPER_BASE: n: number of rows
    // HELPER_BASE+4: rows: address of rows[.] vector
    // HELPER_BASE+8: cols: address of cols[.] vector
    // HELPER_BASE+12: v: address of v[.] vector where v is 
    // the vector corresponding to the operation M*v, M is the sparse
    // matrix.
    // HELPER_BASE+16: buffer id to be used to fill out values from v[.]
    // HELPER_BASE+20: reserved
    // HELPER_BASE+24: start bit. Writing a 1 to this location will
    // prompt the buffer controller to start processing.
    
    // load the value of HELPER_BASE into R30
    ld_r30_helper_base_upper_20_macro;
    add_r30_r30_imm_helper_base_lower_12_macro;
    if (helper_base_lower_12 & 0x800) sub_r30_r30_r12_macro;

	//Ec
	stw_r30_r0_imm_0_macro;	

	//storage format
    add_r22_r0_imm_1_macro;
	stw_r30_r22_imm_4_macro;	

    // store n 
    stw_r30_r23_imm_8_macro;

    // store rows - address of rows[]
    stw_r30_r24_imm_12_macro;

    // store cols - address of cols[]
    stw_r30_r28_imm_16_macro;

    // store v  - address of v[]
    stw_r30_r29_imm_20_macro;
    
	// store vals  - address of vals[]
    stw_r30_r27_imm_24_macro;
	
	// store bitvector  - address of bitvector[]
    stw_r30_r25_imm_28_macro;

    // store 0  -- buffer id to use
    stw_r30_r0_imm_32_macro;
    
	// store nnz
    stw_r30_r26_imm_36_macro;

    // store 1 -- start bit 
    stw_r30_r22_imm_48_macro;

    // now move R30 by 1024 bytes to point to
    // the buffer base
    add_r30_r30_imm_1024_macro;
	
	//copy r29 (v[]) to another register (r28) as backup		
	add_r28_r29_imm_0_macro;	

	int s;
	volatile int * BUFFER = HHT_BASE + 1024/sizeof(int);
    
	for (int i=0;i < n; i++)
    {
		s = 0;
        for (int j=0;j<n; j+=8)
        {
			s += (*BUFFER)*v[j];
            s += (*BUFFER)*v[j+1];
            s += (*BUFFER)*v[j+2];
            s += (*BUFFER)*v[j+3];
            s += (*BUFFER)*v[j+4];
            s += (*BUFFER)*v[j+5];
            s += (*BUFFER)*v[j+6];
            s += (*BUFFER)*v[j+7];
        	//index++;
		}
        y[i] = s;
    }

}

void execHelperDenseBVVector2(void) 
{
}

void execHelperDenseBVVector4(void) 
{
}

void execHelperDenseBVVector8(void) 
{
	//RL
	// initialize locals i, j, s, k, vm;
    // i maps to R18
    // j maps to R19
    // s maps to R20
    // k maps to R21
    // vm maps to R22
    // n maps to R23
    // rows address maps to R24
    // rows[i] maps to R25
    // rows[i+1] maps to R26
    // nnz maps to R25 - dont need old R25
    // num_iters maps to R26
    // address of vals maps to R27
    
    // i,j, s, k, vm to 0
    set_r18_0_macro; 
    set_r19_0_macro; 
    set_r20_0_macro; 
    set_r21_0_macro; 
    set_r22_0_macro; 

    // load address of n to R23
    ld_r23_n_upper_20_macro;
    add_r23_r23_imm_n_lower_12_macro;
    ld_r12_imm_0xfffff_macro;
    if (n_lower_12 & 0x800) sub_r23_r23_r12_macro;

    // load value of n to R23 (do not need address of n anymore)
    ldw_r23_r23_imm_0_macro;

    // VL: 64 bits
    add_r6_r0_imm_256_macro;

    // SEW: 32 bits
    vsetvli_r7_r6_e32_macro;

    // outer loop on n.
    // for (int i=0;i<n;i++)
    // do the increment of i and then compare

	//load R26 with nnz
    ld_r26_nnz_upper_20_macro;
    add_r26_r26_imm_nnz_lower_12_macro;
    if (nnz_lower_12 & 0x800) sub_r26_r26_r12_macro;

	//load R25 with bitvector base
    ld_r25_bv_upper_20_macro;
    add_r25_r25_imm_bv_lower_12_macro;
    if (bv_lower_12 & 0x800) sub_r25_r25_r12_macro;

    // load R24 with address of rows
    ld_r24_rl_rows_upper_20_macro;
    add_r24_r24_imm_rl_rows_lower_12_macro;
    if (rl_rows_lower_12 & 0x800) sub_r24_r24_r12_macro;

    // load R27 with address of vals
    ld_r27_b_vals_upper_20_macro;
    add_r27_r27_imm_b_vals_lower_12_macro;
    if (b_vals_lower_12 & 0x800) sub_r27_r27_r12_macro;

    // load R28 with address of cols
    ld_r28_rl_cols_upper_20_macro;
    add_r28_r28_imm_rl_cols_lower_12_macro;
    if (rl_cols_lower_12 & 0x800) sub_r28_r28_r12_macro;

    // load R29 with address of v[.] vector
    ld_r29_v_upper_20_macro;
    add_r29_r29_imm_v_lower_12_macro;
    if (v_lower_12 & 0x800) sub_r29_r29_r12_macro;

    // load R31 with address of y[.] vector
    ld_r31_y_upper_20_macro;
    add_r31_r31_imm_y_lower_12_macro;
    if (y_lower_12 & 0x800) sub_r31_r31_r12_macro;

hw_helper_init:
    // program the HW helper with the following:
    // HW helper is memory mapped to locations [HELPER_BASE, HELPER_BASE+8KB]
    // the base addresses of these writes are also specified below
    // HELPER_BASE: n: number of rows
    // HELPER_BASE+4: rows: address of rows[.] vector
    // HELPER_BASE+8: cols: address of cols[.] vector
    // HELPER_BASE+12: v: address of v[.] vector where v is 
    // the vector corresponding to the operation M*v, M is the sparse
    // matrix.
    // HELPER_BASE+16: buffer id to be used to fill out values from v[.]
    // HELPER_BASE+20: reserved
    // HELPER_BASE+24: start bit. Writing a 1 to this location will
    // prompt the buffer controller to start processing.
    
    // load the value of HELPER_BASE into R30
    ld_r30_helper_base_upper_20_macro;
    add_r30_r30_imm_helper_base_lower_12_macro;
    if (helper_base_lower_12 & 0x800) sub_r30_r30_r12_macro;

	//Ec
	stw_r30_r0_imm_0_macro;	

	//storage format
    add_r22_r0_imm_1_macro;
	stw_r30_r22_imm_4_macro;	

    // store n 
    stw_r30_r23_imm_8_macro;

    // store rows - address of rows[]
    stw_r30_r24_imm_12_macro;

    // store cols - address of cols[]
    stw_r30_r28_imm_16_macro;

    // store v  - address of v[]
    stw_r30_r29_imm_20_macro;
    
	// store vals  - address of vals[]
    stw_r30_r27_imm_24_macro;
	
	// store bitvector  - address of bitvector[]
    stw_r30_r25_imm_28_macro;

    // store 0  -- buffer id to use
    stw_r30_r0_imm_32_macro;
    
	// store nnz
    stw_r30_r26_imm_36_macro;

    // store 1 -- start bit 
    add_r22_r0_imm_1_macro;
    stw_r30_r22_imm_48_macro;

    // now move R30 by 1024 bytes to point to
    // the buffer base
    add_r30_r30_imm_1024_macro;
	
	//copy r29 (v[]) to another register (r28) as backup		
	add_r28_r29_imm_0_macro;	

outer_loop:

    // load rl_rows[i] - R25
    ldw_r22_r24_imm_0_macro;

    // compute num iterations
    // divide nnz by vector width (8)
    // this is just a shift right 
	
	//for expansion, n/8
    srl_r26_r23_imm_3_macro;
    //nop;
    
    // for the inner loop on number of iterations
    // we need to set j to 0.
    set_r19_0_macro; 
    //nop;

    // reset V30 that holds the sum to be output to Y[.]
    reset_v30_r0_macro;

    // check if j == number of iterations
    // jump to 'post_inner_loop'
    beq_r19_r26_imm_pos_40_macro;
    //nop;

    // set s to 0
    set_r20_0_macro; 

inner_loop:
    // read vals[k] through vals[k+vector_size-1] as a vector load
    vlwv_v27_r29_macro;

    // increment vals address by vector_size*32 bytes
    incr32_r29_macro; 

    // special buffered load
    reset_v29_r0_macro;
    vlwv_v29_r30_macro; // v[.] loaded to V29 via buffer load
    vmul_vv_v29_v27_v29_vm_macro; // pairwise multiply of vals[.] and v[.] into V29 
    vredsum_vs_v30_v30_v29_macro; // cumulate into v30

    // increment j
    incr_r19_macro; 
    //nop;

    // branch back to start of inner loop if j < number of iterations
    blt_r19_r26_imm_neg_28_macro;
    //nop;

post_inner_loop:

    // store Y[.] 
    vswv_r31_v30_macro;

    // move Y address forward by 4
    incr4_r31_macro;

    // reset V30 that holds the sum to be output to Y[.]
    reset_v30_r0_macro;

    incr_r18_macro; 

	//sub vector base to 0
	add_r29_r28_imm_0_macro;

    // branch back to start of outer loop if i < n
    blt_r18_r23_imm_neg_76_macro;
    //nop;

    // for verification - store R18 to some unused variable t.
    // use R24 to hold address of t.
    ld_r24_t_upper_20_macro;
    add_r24_r24_imm_t_lower_12_macro;
    if (t_lower_12 & 0x800) sub_r24_r24_r12_macro;
    vswv_r24_v28_macro;
}

void execHelperDenseRLScalar(void) 
{
    // load address of n to R23
    ld_r23_n_upper_20_macro;
    add_r23_r23_imm_n_lower_12_macro;
    ld_r12_imm_0xfffff_macro;
    if (n_lower_12 & 0x800) sub_r23_r23_r12_macro;

    // load value of n to R23 (do not need address of n anymore)
    ldw_r23_r23_imm_0_macro;

    // VL: 64 bits
    add_r6_r0_imm_256_macro;

    // SEW: 32 bits
    vsetvli_r7_r6_e32_macro;

    // outer loop on n.
    // for (int i=0;i<n;i++)
    // do the increment of i and then compare

	//load R26 with nnz
    ld_r26_nnz_upper_20_macro;
    add_r26_r26_imm_nnz_lower_12_macro;
    if (nnz_lower_12 & 0x800) sub_r26_r26_r12_macro;

	//load R25 with bitvector base
    ld_r25_bv_upper_20_macro;
    add_r25_r25_imm_bv_lower_12_macro;
    if (bv_lower_12 & 0x800) sub_r25_r25_r12_macro;

    // load R24 with address of rows
    ld_r24_rl_rows_upper_20_macro;
    add_r24_r24_imm_rl_rows_lower_12_macro;
    if (rl_rows_lower_12 & 0x800) sub_r24_r24_r12_macro;

    // load R27 with address of vals
    ld_r27_rl_vals_upper_20_macro;
    add_r27_r27_imm_rl_vals_lower_12_macro;
    if (rl_vals_lower_12 & 0x800) sub_r27_r27_r12_macro;

    // load R28 with address of cols
    ld_r28_rl_cols_upper_20_macro;
    add_r28_r28_imm_rl_cols_lower_12_macro;
    if (rl_cols_lower_12 & 0x800) sub_r28_r28_r12_macro;

    // load R29 with address of v[.] vector
    ld_r29_v_upper_20_macro;
    add_r29_r29_imm_v_lower_12_macro;
    if (v_lower_12 & 0x800) sub_r29_r29_r12_macro;

    // load R31 with address of y[.] vector
    //ld_r31_y_upper_20_macro;
    //add_r31_r31_imm_y_lower_12_macro;
    //if (y_lower_12 & 0x800) sub_r31_r31_r12_macro;

hw_helper_init:
    // program the HW helper with the following:
    // HW helper is memory mapped to locations [HELPER_BASE, HELPER_BASE+8KB]
    // the base addresses of these writes are also specified below
    // HELPER_BASE: n: number of rows
    // HELPER_BASE+4: rows: address of rows[.] vector
    // HELPER_BASE+8: cols: address of cols[.] vector
    // HELPER_BASE+12: v: address of v[.] vector where v is 
    // the vector corresponding to the operation M*v, M is the sparse
    // matrix.
    // HELPER_BASE+16: buffer id to be used to fill out values from v[.]
    // HELPER_BASE+20: reserved
    // HELPER_BASE+24: start bit. Writing a 1 to this location will
    // prompt the buffer controller to start processing.
    
    // load the value of HELPER_BASE into R30
    ld_r30_helper_base_upper_20_macro;
    add_r30_r30_imm_helper_base_lower_12_macro;
    if (helper_base_lower_12 & 0x800) sub_r30_r30_r12_macro;

	//Ec
	stw_r30_r0_imm_0_macro;	

	//storage format
    add_r22_r0_imm_2_macro;
	stw_r30_r22_imm_4_macro;	

    // store n 
    stw_r30_r23_imm_8_macro;

    // store rows - address of rows[]
    stw_r30_r24_imm_12_macro;

    // store cols - address of cols[]
    stw_r30_r28_imm_16_macro;

    // store v  - address of v[]
    stw_r30_r29_imm_20_macro;
    
	// store vals  - address of vals[]
    stw_r30_r27_imm_24_macro;
	
	// store bitvector  - address of bitvector[]
    stw_r30_r25_imm_28_macro;

    // store 0  -- buffer id to use
    stw_r30_r0_imm_32_macro;
    
	// store nnz
    stw_r30_r26_imm_36_macro;

    // store 1 -- start bit 
    add_r22_r0_imm_1_macro;
    stw_r30_r22_imm_48_macro;

    // now move R30 by 1024 bytes to point to
    // the buffer base
    add_r30_r30_imm_1024_macro;
	
	//copy r29 (v[]) to another register (r28) as backup		
	add_r28_r29_imm_0_macro;	

	int s;
	volatile int * BUFFER = HHT_BASE + 1024/sizeof(int);
    
	for (int i=0;i < n; i++)
    {
		s = 0;
        for (int j=0;j<n; j+=8)
        {
    		s += (*BUFFER)*v[j];
            s += (*BUFFER)*v[j+1];
            s += (*BUFFER)*v[j+2];
            s += (*BUFFER)*v[j+3];
            s += (*BUFFER)*v[j+4];
            s += (*BUFFER)*v[j+5];
            s += (*BUFFER)*v[j+6];
            s += (*BUFFER)*v[j+7];
	    	//index++;
		}
        y[i] = s;
    }
    // for verification - store R18 to some unused variable t.
    // use R24 to hold address of t.
    //ld_r24_t_upper_20_macro;
    //add_r24_r24_imm_t_lower_12_macro;
    //if (t_lower_12 & 0x800) sub_r24_r24_r12_macro;
    //vswv_r24_v28_macro;
}

void execHelperDenseRLVector2(void) 
{
}

void execHelperDenseRLVector4(void) 
{
}

void execHelperDenseRLVector8(void) 
{
	//RL
	// initialize locals i, j, s, k, vm;
    // i maps to R18
    // j maps to R19
    // s maps to R20
    // k maps to R21
    // vm maps to R22
    // n maps to R23
    // rows address maps to R24
    // rows[i] maps to R25
    // rows[i+1] maps to R26
    // nnz maps to R25 - dont need old R25
    // num_iters maps to R26
    // address of vals maps to R27
    
    // i,j, s, k, vm to 0
    set_r18_0_macro; 
    set_r19_0_macro; 
    set_r20_0_macro; 
    set_r21_0_macro; 
    set_r22_0_macro; 

    // load address of n to R23
    ld_r23_n_upper_20_macro;
    add_r23_r23_imm_n_lower_12_macro;
    ld_r12_imm_0xfffff_macro;
    if (n_lower_12 & 0x800) sub_r23_r23_r12_macro;

    // load value of n to R23 (do not need address of n anymore)
    ldw_r23_r23_imm_0_macro;

    // VL: 64 bits
    add_r6_r0_imm_256_macro;

    // SEW: 32 bits
    vsetvli_r7_r6_e32_macro;

    // outer loop on n.
    // for (int i=0;i<n;i++)
    // do the increment of i and then compare

	//load R26 with nnz
    ld_r26_nnz_upper_20_macro;
    add_r26_r26_imm_nnz_lower_12_macro;
    if (nnz_lower_12 & 0x800) sub_r26_r26_r12_macro;

	//load R25 with bitvector base
    ld_r25_bv_upper_20_macro;
    add_r25_r25_imm_bv_lower_12_macro;
    if (bv_lower_12 & 0x800) sub_r25_r25_r12_macro;

    // load R24 with address of rows
    ld_r24_rl_rows_upper_20_macro;
    add_r24_r24_imm_rl_rows_lower_12_macro;
    if (rl_rows_lower_12 & 0x800) sub_r24_r24_r12_macro;

    // load R27 with address of vals
    ld_r27_rl_vals_upper_20_macro;
    add_r27_r27_imm_rl_vals_lower_12_macro;
    if (rl_vals_lower_12 & 0x800) sub_r27_r27_r12_macro;

    // load R28 with address of cols
    ld_r28_rl_cols_upper_20_macro;
    add_r28_r28_imm_rl_cols_lower_12_macro;
    if (rl_cols_lower_12 & 0x800) sub_r28_r28_r12_macro;

    // load R29 with address of v[.] vector
    ld_r29_v_upper_20_macro;
    add_r29_r29_imm_v_lower_12_macro;
    if (v_lower_12 & 0x800) sub_r29_r29_r12_macro;

    // load R31 with address of y[.] vector
    ld_r31_y_upper_20_macro;
    add_r31_r31_imm_y_lower_12_macro;
    if (y_lower_12 & 0x800) sub_r31_r31_r12_macro;

hw_helper_init:
    // program the HW helper with the following:
    // HW helper is memory mapped to locations [HELPER_BASE, HELPER_BASE+8KB]
    // the base addresses of these writes are also specified below
    // HELPER_BASE: n: number of rows
    // HELPER_BASE+4: rows: address of rows[.] vector
    // HELPER_BASE+8: cols: address of cols[.] vector
    // HELPER_BASE+12: v: address of v[.] vector where v is 
    // the vector corresponding to the operation M*v, M is the sparse
    // matrix.
    // HELPER_BASE+16: buffer id to be used to fill out values from v[.]
    // HELPER_BASE+20: reserved
    // HELPER_BASE+24: start bit. Writing a 1 to this location will
    // prompt the buffer controller to start processing.
    
    // load the value of HELPER_BASE into R30
    ld_r30_helper_base_upper_20_macro;
    add_r30_r30_imm_helper_base_lower_12_macro;
    if (helper_base_lower_12 & 0x800) sub_r30_r30_r12_macro;

	//Ec
	stw_r30_r0_imm_0_macro;	

	//storage format
    add_r22_r0_imm_2_macro;
	stw_r30_r22_imm_4_macro;	

    // store n 
    stw_r30_r23_imm_8_macro;

    // store rows - address of rows[]
    stw_r30_r24_imm_12_macro;

    // store cols - address of cols[]
    stw_r30_r28_imm_16_macro;

    // store v  - address of v[]
    stw_r30_r29_imm_20_macro;
    
	// store vals  - address of vals[]
    stw_r30_r27_imm_24_macro;
	
	// store bitvector  - address of bitvector[]
    stw_r30_r25_imm_28_macro;

    // store 0  -- buffer id to use
    stw_r30_r0_imm_32_macro;
    
	// store nnz
    stw_r30_r26_imm_36_macro;

    // store 1 -- start bit 
    add_r22_r0_imm_1_macro;
    stw_r30_r22_imm_48_macro;

    // now move R30 by 1024 bytes to point to
    // the buffer base
    add_r30_r30_imm_1024_macro;
	
	//copy r29 (v[]) to another register (r28) as backup		
	add_r28_r29_imm_0_macro;	

outer_loop:

    // load rl_rows[i] - R25
    ldw_r22_r24_imm_0_macro;

    // compute num iterations
    // divide nnz by vector width (8)
    // this is just a shift right 
	
	//for expansion, n/8
    srl_r26_r23_imm_3_macro;
    //nop;
    
    // for the inner loop on number of iterations
    // we need to set j to 0.
    set_r19_0_macro; 
    //nop;

    // reset V30 that holds the sum to be output to Y[.]
    reset_v30_r0_macro;

    // check if j == number of iterations
    // jump to 'post_inner_loop'
    beq_r19_r26_imm_pos_40_macro;
    //nop;

    // set s to 0
    set_r20_0_macro; 

inner_loop:
    // read vals[k] through vals[k+vector_size-1] as a vector load
    vlwv_v27_r29_macro;

    // increment vals address by vector_size*32 bytes
    incr32_r29_macro; 

    // special buffered load
    reset_v29_r0_macro;
    vlwv_v29_r30_macro; // v[.] loaded to V29 via buffer load
    vmul_vv_v29_v27_v29_vm_macro; // pairwise multiply of vals[.] and v[.] into V29 
    vredsum_vs_v30_v30_v29_macro; // cumulate into v30

    // increment j
    incr_r19_macro; 
    //nop;

    // branch back to start of inner loop if j < number of iterations
    blt_r19_r26_imm_neg_28_macro;
    //nop;

post_inner_loop:

    // store Y[.] 
    vswv_r31_v30_macro;

    // move Y address forward by 4
    incr4_r31_macro;

    // reset V30 that holds the sum to be output to Y[.]
    reset_v30_r0_macro;

    incr_r18_macro; 

	//sub vector base to 0
	add_r29_r28_imm_0_macro;

    // branch back to start of outer loop if i < n
    blt_r18_r23_imm_neg_76_macro;
    //nop;

    // for verification - store R18 to some unused variable t.
    // use R24 to hold address of t.
    ld_r24_t_upper_20_macro;
    add_r24_r24_imm_t_lower_12_macro;
    if (t_lower_12 & 0x800) sub_r24_r24_r12_macro;
    vswv_r24_v28_macro;
		
}

void execHelperSparseCSRScalar(void) 
{
}

void execHelperSparseCSRVector2(void) 
{
}

void execHelperSparseCSRVector4(void) 
{
}

void execHelperSparseCSRVector8(void) 
{
   // initialize locals i, j, s, k, vm;
    // i maps to R18
    // j maps to R19
    // s maps to R20
    // k maps to R21
    // vm maps to R22
    // n maps to R23
    // rows address maps to R24
    // rows[i] maps to R25
    // rows[i+1] maps to R26
    // nnz maps to R25 - dont need old R25
    // num_iters maps to R26
    // address of vals maps to R27
    
    // i,j, s, k, vm to 0
    set_r18_0_macro; 
    set_r19_0_macro; 
    set_r20_0_macro; 
    set_r21_0_macro; 
    set_r22_0_macro; 

    // load address of n to R23
    ld_r23_n_upper_20_macro;
    add_r23_r23_imm_n_lower_12_macro;
    ld_r12_imm_0xfffff_macro;
    if (n_lower_12 & 0x800) sub_r23_r23_r12_macro;

    // load value of n to R23 (do not need address of n anymore)
    ldw_r23_r23_imm_0_macro;

    // VL: 256 bits
    add_r6_r0_imm_256_macro;

    // SEW: 32 bits
    vsetvli_r7_r6_e32_macro;

    // outer loop on n.
    // for (int i=0;i<n;i++)
    // do the increment of i and then compare

	//load R26 with nnz
    ld_r26_nnz_upper_20_macro;
    add_r26_r26_imm_nnz_lower_12_macro;
    if (nnz_lower_12 & 0x800) sub_r26_r26_r12_macro;

    //load R25 with bitvector base
    ld_r25_bv_upper_20_macro;
    add_r25_r25_imm_bv_lower_12_macro;
    if (bv_lower_12 & 0x800) sub_r25_r25_r12_macro;

    // load R24 with address of rows
    ld_r24_rows_upper_20_macro;
    add_r24_r24_imm_rows_lower_12_macro;
    if (rows_lower_12 & 0x800) sub_r24_r24_r12_macro;

    // load R27 with address of vals
    ld_r27_vals_upper_20_macro;
    add_r27_r27_imm_vals_lower_12_macro;
    if (vals_lower_12 & 0x800) sub_r27_r27_r12_macro;

    // load R28 with address of cols
    ld_r28_cols_upper_20_macro;
    add_r28_r28_imm_cols_lower_12_macro;
    if (cols_lower_12 & 0x800) sub_r28_r28_r12_macro;

    // load R29 with address of v[.] vector
    ld_r29_v_upper_20_macro;
    add_r29_r29_imm_v_lower_12_macro;
    if (v_lower_12 & 0x800) sub_r29_r29_r12_macro;

    // load R31 with address of y[.] vector
    ld_r31_y_upper_20_macro;
    add_r31_r31_imm_y_lower_12_macro;
    if (y_lower_12 & 0x800) sub_r31_r31_r12_macro;


hw_helper_init:
    // program the HW helper with the following:
    // HW helper is memory mapped to locations [HELPER_BASE, HELPER_BASE+8KB]
    // the base addresses of these writes are also specified below
    // HELPER_BASE: n: number of rows
    // HELPER_BASE+4: rows: address of rows[.] vector
    // HELPER_BASE+8: cols: address of cols[.] vector
    // HELPER_BASE+12: v: address of v[.] vector where v is 
    // the vector corresponding to the operation M*v, M is the sparse
    // matrix.
    // HELPER_BASE+16: buffer id to be used to fill out values from v[.]
    // HELPER_BASE+20: reserved
    // HELPER_BASE+24: start bit. Writing a 1 to this location will
    // prompt the buffer controller to start processing.
    
    // load the value of HELPER_BASE into R30
    ld_r30_helper_base_upper_20_macro;
    add_r30_r30_imm_helper_base_lower_12_macro;
    if (helper_base_lower_12 & 0x800) sub_r30_r30_r12_macro;

	//Ec
    add_r22_r0_imm_1_macro;
    stw_r30_r22_imm_0_macro;			//should 1 for CSR no_expand helper

    //storage format
    stw_r30_r0_imm_4_macro;			//should be 1 for CSR

    // store n
    stw_r30_r23_imm_8_macro;

    // store rows - address of rows[]
    stw_r30_r24_imm_12_macro;

    // store cols - address of cols[]
    stw_r30_r28_imm_16_macro;

    // store v  - address of v[]
    stw_r30_r29_imm_20_macro;

    // store vals  - address of vals[]
    stw_r30_r27_imm_24_macro;

    // store bitvector  - address of bitvector[]
    stw_r30_r25_imm_28_macro;

    // store 0  -- buffer id to use
    stw_r30_r0_imm_32_macro;

    // store nnz
    stw_r30_r26_imm_36_macro;

    // store 1 -- start bit
    stw_r30_r22_imm_48_macro;

    // now move R30 by 1024 bytes to point to
    // the buffer base
    add_r30_r30_imm_1024_macro;

    //copy r29 (v[]) to another register (r28) as backup
    add_r28_r29_imm_0_macro;


outer_loop:
    // load rows[i] - R25
    ldw_r25_r24_imm_0_macro;
    // increment R24 by 4
    incr_r24_by_4_macro;
    // load rows[i+1] - R26
    ldw_r26_r24_imm_0_macro;

    // compute nnz - R26 - R25 into R25
    sub_r25_r26_r25_macro;

    // compute num iterations
    // divide nnz by vector width (8)
    // this is just a shift right by 3
    // followed by a check if nnz % 8 leaves a reminder
    // for the last extra iteration
    
    srl_r26_r25_imm_3_macro;
    //nop;
    
    // for the inner loop on number of iterations
    // we need to set j to 0.
    set_r19_0_macro; 
    //nop;

    // reset V30 that holds the sum to be output to Y[.]
    reset_v30_r0_macro;

    // check if j == number of iterations
    // jump to 'post_inner_loop'
    beq_r19_r26_imm_pos_36_macro;
    //nop;

    // set s to 0
    set_r20_0_macro; 

inner_loop:
    // read vals[k] through vals[k+7] as a vector load
    vlwv_v27_r27_macro;

    // increment vals address by 32 bytes
    incr32_r27_macro; 

    // special buffered load
    vlwv_v29_r30_macro; // v[.] loaded to V29 via buffer load
    vmul_vv_v29_v27_v29_vm_macro; // pairwise multiply of vals[.] and v[.] into V29
    vredsum_vs_v30_v30_v29_macro; // cumulate into v30

    // increment j
    incr_r19_macro; 
    //nop;

    // branch back to start of inner loop if j < number of iterations
    blt_r19_r26_imm_neg_24_macro;
    //nop;

post_inner_loop:

    // store Y[.] 
    vswv_r31_v30_macro;

    // move Y address forward by 4
    incr4_r31_macro;

    // reset V30 that holds the sum to be output to Y[.]
    reset_v30_r0_macro;

    incr_r18_macro; 

    // branch back to start of outer loop if i < n
    blt_r18_r23_imm_neg_80_macro;
    //nop;

    // for verification - store R18 to some unused variable t.
    // use R24 to hold address of t.
    ld_r24_t_upper_20_macro;
    add_r24_r24_imm_t_lower_12_macro;
    if (t_lower_12 & 0x800) sub_r24_r24_r12_macro;
    vswv_r24_v28_macro;

}

void execHelperSparseCSRVector8StoreCompress(void) 
{
   // initialize locals i, j, s, k, vm;
    // i maps to R18
    // j maps to R19
    // s maps to R20
    // k maps to R21
    // vm maps to R22
    // n maps to R23
    // rows address maps to R24
    // rows[i] maps to R25
    // rows[i+1] maps to R26
    // nnz maps to R25 - dont need old R25
    // num_iters maps to R26
    // address of vals maps to R27
    
    // i,j, s, k, vm to 0
    set_r18_0_macro; 
    set_r19_0_macro; 
    set_r20_0_macro; 
    set_r21_0_macro; 
    set_r22_0_macro; 

    // load address of n to R23
    ld_r23_n_upper_20_macro;
    add_r23_r23_imm_n_lower_12_macro;
    ld_r12_imm_0xfffff_macro;
    if (n_lower_12 & 0x800) sub_r23_r23_r12_macro;

    // load value of n to R23 (do not need address of n anymore)
    ldw_r23_r23_imm_0_macro;

    // VL: 256 bits
    add_r6_r0_imm_256_macro;

    // SEW: 32 bits
    vsetvli_r7_r6_e32_macro;

    // outer loop on n.
    // for (int i=0;i<n;i++)
    // do the increment of i and then compare

	//load R26 with nnz
    ld_r26_nnz_upper_20_macro;
    add_r26_r26_imm_nnz_lower_12_macro;
    if (nnz_lower_12 & 0x800) sub_r26_r26_r12_macro;

    //load R25 with bitvector base
    ld_r25_bv_upper_20_macro;
    add_r25_r25_imm_bv_lower_12_macro;
    if (bv_lower_12 & 0x800) sub_r25_r25_r12_macro;

    // load R24 with address of rows
    ld_r24_rows_upper_20_macro;
    add_r24_r24_imm_rows_lower_12_macro;
    if (rows_lower_12 & 0x800) sub_r24_r24_r12_macro;

    // load R27 with address of vals
    ld_r27_vals_upper_20_macro;
    add_r27_r27_imm_vals_lower_12_macro;
    if (vals_lower_12 & 0x800) sub_r27_r27_r12_macro;

    // load R28 with address of cols
    ld_r28_cols_upper_20_macro;
    add_r28_r28_imm_cols_lower_12_macro;
    if (cols_lower_12 & 0x800) sub_r28_r28_r12_macro;

    // load R29 with address of v[.] vector
    ld_r29_v_upper_20_macro;
    add_r29_r29_imm_v_lower_12_macro;
    if (v_lower_12 & 0x800) sub_r29_r29_r12_macro;

    // load R31 with address of y[.] vector
    ld_r31_y_upper_20_macro;
    add_r31_r31_imm_y_lower_12_macro;
    if (y_lower_12 & 0x800) sub_r31_r31_r12_macro;


hw_helper_init:
    // program the HW helper with the following:
    // HW helper is memory mapped to locations [HELPER_BASE, HELPER_BASE+8KB]
    // the base addresses of these writes are also specified below
    // HELPER_BASE: n: number of rows
    // HELPER_BASE+4: rows: address of rows[.] vector
    // HELPER_BASE+8: cols: address of cols[.] vector
    // HELPER_BASE+12: v: address of v[.] vector where v is 
    // the vector corresponding to the operation M*v, M is the sparse
    // matrix.
    // HELPER_BASE+16: buffer id to be used to fill out values from v[.]
    // HELPER_BASE+20: reserved
    // HELPER_BASE+24: start bit. Writing a 1 to this location will
    // prompt the buffer controller to start processing.
    
    // load the value of HELPER_BASE into R30
    ld_r30_helper_base_upper_20_macro;
    add_r30_r30_imm_helper_base_lower_12_macro;
    if (helper_base_lower_12 & 0x800) sub_r30_r30_r12_macro;

	//Ec
    add_r22_r0_imm_1_macro;
    stw_r30_r22_imm_0_macro;			//should 1 for CSR no_expand helper

    //storage format
    stw_r30_r0_imm_4_macro;			//should be 1 for CSR

    // store n
    stw_r30_r23_imm_8_macro;

    // store rows - address of rows[]
    stw_r30_r24_imm_12_macro;

    // store cols - address of cols[]
    stw_r30_r28_imm_16_macro;

    // store v  - address of v[]
    stw_r30_r29_imm_20_macro;

    // store vals  - address of vals[]
    stw_r30_r27_imm_24_macro;

    // store bitvector  - address of bitvector[]
    stw_r30_r25_imm_28_macro;

    // store 0  -- buffer id to use
    stw_r30_r0_imm_32_macro;

    // store nnz
    stw_r30_r26_imm_36_macro;

    // store 1 -- start bit
    stw_r30_r22_imm_48_macro;

    // now move R30 by 1024 bytes to point to
    // the buffer base
    add_r30_r30_imm_1024_macro;

    //copy r29 (v[]) to another register (r28) as backup
    add_r28_r29_imm_0_macro;


outer_loop:
    // load rows[i] - R25
    ldw_r25_r24_imm_0_macro;
    // increment R24 by 4
    incr_r24_by_4_macro;
    // load rows[i+1] - R26
    ldw_r26_r24_imm_0_macro;

    // compute nnz - R26 - R25 into R25
    sub_r25_r26_r25_macro;

    // compute num iterations
    // divide nnz by vector width (8)
    // this is just a shift right by 3
    // followed by a check if nnz % 8 leaves a reminder
    // for the last extra iteration
    
    srl_r26_r25_imm_3_macro;
    //nop;
    
    // for the inner loop on number of iterations
    // we need to set j to 0.
    set_r19_0_macro; 
    //nop;

    // reset V30 that holds the sum to be output to Y[.]
    reset_v30_r0_macro;

    // check if j == number of iterations
    // jump to 'post_inner_loop'
    beq_r19_r26_imm_pos_36_macro;
    //nop;

    // set s to 0
    set_r20_0_macro; 

inner_loop:
    // read vals[k] through vals[k+7] as a vector load
    vlwv_v27_r27_macro;

    // increment vals address by 32 bytes
    incr32_r27_macro; 

    // special buffered load
    vlwv_v29_r30_macro; // v[.] loaded to V29 via buffer load
    vmul_vv_v29_v27_v29_vm_macro; // pairwise multiply of vals[.] and v[.] into V29
    vredsum_vs_v30_v30_v29_macro; // cumulate into v30

    // increment j
    incr_r19_macro; 
    //nop;

    // branch back to start of inner loop if j < number of iterations
    blt_r19_r26_imm_neg_24_macro;
    //nop;

post_inner_loop:

    // store Y[.] 
    //vswv_r31_v30_macro;

	//store at a specific memory mapped address;
	//vswv_r22_v30_macro;	//r22 should have BC store address; BC gets triggered when tried to store at that location, then BC compresses and stores

    // move Y address forward by 4
    //incr4_r31_macro;

    // reset V30 that holds the sum to be output to Y[.]
    reset_v30_r0_macro;

    incr_r18_macro; 

    // branch back to start of outer loop if i < n
    blt_r18_r23_imm_neg_80_macro;
    //nop;

    // for verification - store R18 to some unused variable t.
    // use R24 to hold address of t.
    ld_r24_t_upper_20_macro;
    add_r24_r24_imm_t_lower_12_macro;
    if (t_lower_12 & 0x800) sub_r24_r24_r12_macro;
    vswv_r24_v28_macro;

}

void execHelperSparseBVScalar(void) 
{
}

void execHelperSparseBVVector2(void) 
{
}

void execHelperSparseBVVector4(void) 
{
}

void execHelperSparseBVVector8(void) 
{
}

void execHelperSparseRLScalar(void) 
{
}

void execHelperSparseRLVector2(void) 
{
}

void execHelperSparseRLVector4(void) 
{
}

void execHelperSparseRLVector8(void) 
{
}

void execCSRCSRScalar(void) 
{
    execCSRScalar();
}

void execCSRCSRVector2(void) 
{
    execCSRVector_V2();
}

void execCSRCSRVector4(void) 
{
    execCSRVector_V4();
}

void execCSRCSRVector8(void) 
{
    execCSRVector_V8();
}

void execBVBVScalar(void) 
{
    execBVScalar(); 
}

void execBVBVVector2(void) 
{
}

void execBVBVVector4(void) 
{
}

void execBVBVVector8(void) 
{
}

void execRLRLScalar(void) 
{
    execRLScalar(); 
}

void execRLRLVector2(void) 
{
}

void execRLRLVector4(void) 
{
}

void execRLRLVector8(void) 
{
}

#endif

void usage(void)
{
    printf("Program requires five args:\n");
    printf("Arg 1: 0 or 1 value -- 0 if only compile, 1 if compile+exec\n");
    printf("Arg 2: 0 - dense synth matrix, 1 - CSR sparse matrix, 2 - TAMU COO sparse matrix, 3 - DNN weight matrix\n");
    printf("Arg 3: matrix data file as input.\n");
    printf("Arg 4: 0/1/2 value -- storage format: 0 -- dense, 1 -- CSR, 2 -- Bit vector, 3 -- Run-length\n");
    printf("Arg 5: 0/1/2/3/4 value -- compute format: 0 -- dense, 1 -- CSR, 2 -- Bit vector, 3 -- Run-length, 4 -- Helper Dense, 5 -- Helper Sparse\n");
    printf("Arg 6: 1/2/4/8 -- vector size (1 is same as scalar)\n");
}

typedef enum {DENSE=0, CSR=1, COO=2, DENSE_DNN=3} matrix_type_t;

int main(int argc, char ** argv)
{
    if (argc <= 5) { usage(); return 0; }
    bool do_compile_exec = false;
    if (strcmp(argv[1],"1") == 0) do_compile_exec = true;

    FILE*fp = fopen(argv[3],"r");
    strcpy(g_file_name, argv[3]);
    if (!fp) { printf("Could not open %s for reading.\n", argv[3]); return 0;}
    matrix_type_t matrix_type;
    if (strcmp(argv[2], "0")==0) matrix_type=DENSE;
    else if (strcmp(argv[2], "1")==0) matrix_type=CSR;
    else if (strcmp(argv[2], "2")==0) matrix_type=COO;
    else if (strcmp(argv[2], "3")==0) matrix_type=DENSE_DNN;
    else { printf("Unknown matrix type.\n"); usage(); return 0;}

    if (matrix_type == DENSE) {
        fscanf(fp, "%d\n", &n);
        if (tooLarge(n, n)) handleError(n, n);
        m = (int*)malloc(n*n*sizeof(int));
        v = (int*)malloc((n+8)*sizeof(int));
        y = (int*)malloc((n+8)*sizeof(int));
        for (int i=0;i<n+8;i++) v[i] = 0;
        for (int i=0;i<n+8;i++) y[i] = 0;
        for (int i=0;i<n*n;i++) fscanf(fp,"%d\n", &m[i]);
        for (int i=0;i<n;i++) fscanf(fp,"%d\n", &v[i]);
    } else if (matrix_type == DENSE_DNN) {
	    int p, q;
        fscanf(fp, "%d\n", &p);
        fscanf(fp, "%d\n", &q);
	    num_r=p; 
	    num_c=q;
	    if (num_r < num_c) num_r = num_c;
	    else num_c = num_r;

        if (num_r % v_size) num_r += (v_size - (num_r % v_size));
        if (num_c % v_size) num_c += (v_size - (num_c % v_size));
        if (tooLarge(num_r, num_c)) handleError(num_r, num_c);
        m = (int*)malloc(num_r*num_c*sizeof(int));
        v = (int*)malloc((num_c+8)*sizeof(int));
        y = (int*)malloc((num_c+8)*sizeof(int));
        for (int i=0;i<num_r;i++) 
        for (int j=0;j<num_c;j++)
            m[i*num_c + j]=0;
        for (int i=0;i<num_c+8;i++) v[i] = 0;
        for (int i=0;i<num_c+8;i++) y[i] = 0;
            n=num_r;

	    for (int i=0;i<p*q; i++) {
             fscanf(fp, "%d\n", &m[i]);
	    }
	    for (int i=0;i<num_c; i++) {
	         v[i] = i;
	    }
    }
    else if (matrix_type == CSR) {
        printf("Format as yet unsupported\n");
        return 0;
    } else if (matrix_type == COO) {
        // Read TAMU Matrix Market formatted input
        // Assumptions: commented lines start with %
        // Matrix COO indices are index-1 based and not index-0 based
        char line[1024];
        int matrix_sz_found=0;
        while (fgets(&line[0], 1024, fp))
        {
            if (line[0] == '%') continue;
            if (matrix_sz_found) {
                int r, c, v_i;
                float v_f;
                sscanf(line, "%d %d %f", &r, &c, &v_f);
                if ((v_f < 0.0001) && (v_f > -0.9999)) v_f = 1.0;
                v_i = (int) v_f;
                m[(r-1)*num_c+(c-1)]=v_i;
            } else {
                sscanf(line, "%d %d %d\n", &num_r, &num_c, &num_nz);
                if (num_r % v_size) num_r += (v_size - (num_r % v_size));
                if (num_c % v_size) num_c += (v_size - (num_c % v_size));
                if (num_r != num_c) {
                    printf("Non-square matrices are not yet supported\n");
                    return 0;
                }
                matrix_sz_found = 1;
                if (tooLarge(num_r, num_c)) handleError(num_r, num_c);
                m = (int*)malloc(num_r*num_c*sizeof(int));
                v = (int*)malloc((num_c+8)*sizeof(int));
                y = (int*)malloc((num_c+8)*sizeof(int));
                for (int i=0;i<num_r;i++) 
                for (int j=0;j<num_c;j++)
                    m[i*num_c + j]=0;
                for (int i=0;i<num_c+8;i++) v[i] = 0;
                for (int i=0;i<num_c+8;i++) y[i] = 0;
                n=num_r;
                printf("Matrix dimensions %d by %d\n", num_r, num_c);
            }
        } 

        // TAMU format does not specify the vector v.
        // so we generate a vector in our code.
        for (int i=0;i<num_c;i++) v[i]=i+1;
    }
    initSparse();

    storage_type = strtoul(argv[4],NULL,10);
    compute_type = strtoul(argv[5],NULL,10);
   	expand = compute_type; 		//if 4, HHT_expand
    if (argc > 6) {
        vector_size = strtoul(argv[6],NULL,10);
        if (vector_size != 1 && vector_size != 2 && vector_size != 4 && vector_size != 8) {
            printf("Illegal vector size. Must be 1/2/4/8.\n");
            exit(0);
        }
        printf("Vector size: %d\n", vector_size);
    }

#ifdef RISCV_BUILD
    genDenseVector();
    genSparseVector();
    genHWHelper_2();
	genprefetch();      //This function generates assembly instructions to store addresses of matrix and vector for prefetching
#endif
    execDenseScalar();

#define COMPARE ((1))
    last_op = y[COMPARE];
    printf("last_op is %d\n", last_op);
    y[COMPARE] = last_op -1;
    volatile unsigned long long s, e;
    //uint64_t s, e;
    //save_regs();
#ifdef GENERATE_ONLY
    return 0;
#else
    if (!do_compile_exec) { free(m); free(v); return 0;}
    exec_fn = assignExecuteFunction(storage_type, compute_type, vector_size);
    s = read_cycles();
    //restore_regs();
    exec_fn();
    e = read_cycles();
	std::cout<<"cycles:"<<e<<":"<<s<<";e-s:"<<e-s<<"\n";	
    printf("Input: %s; Matrix Dim: %d by %d; NNZ: %d; Sparsity: %g; Cycles: %lld\n", g_file_name, n, n, g_nnz, sparsity, e-s);
    
    // check contents of y.
    printf("Last expected: %d\n", last_op);
    printf("Last output: %d\n", y[COMPARE]);
    if (last_op != y[COMPARE]) printf("Failed!\n");
    else printf("Passed!\n");

    free(m);
    free(v);
    
    return 0;
#endif
}

#ifndef GENERATE_ONLY
PFN assignExecuteFunction(storage_type_t storage_format, compute_type_t compute_format, int vector_size)
{
    switch (compute_format)
    {
    case C_DENSE:
        switch (storage_format)
        {
        case S_DENSE:
            switch (vector_size)
            {
            case 1: return execDenseDenseScalar;
            case 2: return execDenseDenseVector2;
            case 4: return execDenseDenseVector4;
            case 8: return execDenseDenseVector8;
            }
            break;
        case S_CSR:
            switch (vector_size)
            {
            case 1: return execHelperDenseCSRScalar;
            case 2: return execHelperDenseCSRVector2;
            case 4: return execHelperDenseCSRVector4;
            case 8: return execHelperDenseCSRVector8;
            }
            break;
        case S_BV:
            switch (vector_size)
            {
            case 1: return execHelperDenseBVScalar;
            case 2: return execHelperDenseBVVector2;
            case 4: return execHelperDenseBVVector4;
            case 8: return execHelperDenseBVVector8;
            }
            break;
        case S_RL:
            switch (vector_size)
            {
            case 1: return execHelperDenseRLScalar;
            case 2: return execHelperDenseRLVector2;
            case 4: return execHelperDenseRLVector4;
            case 8: return execHelperDenseRLVector8;
            }
            break;
        default:
            printf("Illegal storage format %d\n", storage_format);
        }
        break;
    case C_CSR:
        switch (storage_format)
        {
        case S_DENSE:
            printf("Illegal .. can not compute CSR with Dense storage!\n");
            exit(0);
            break;
        case S_CSR:
            switch (vector_size)
            {
            case 1: return execCSRCSRScalar;
            case 2: return execCSRCSRVector2;
            case 4: return execCSRCSRVector4;
            case 8: return execCSRCSRVector8;
            }
            break;
        case S_BV:
            printf("Illegal .. can not compute CSR with Bit vector storage!\n");
            exit(0);
            break;
        case S_RL:
            printf("Illegal .. can not compute CSR with RL storage!\n");
            exit(0);
            break;
        default:
            printf("Illegal storage format %d\n", storage_format);
        }
        break;
    case C_BV:
        switch (storage_format)
        {
        case S_DENSE:
            printf("Illegal .. can not compute Bit-Vector with Dense storage!\n");
            exit(0);
            break;
        case S_CSR:
            printf("Illegal .. can not compute Bit-vector with CSR storage!\n");
            exit(0);
            break;
        case S_BV:
            switch (vector_size)
            {
            case 1: return execBVBVScalar;
            case 2: return execBVBVVector2;
            case 4: return execBVBVVector4;
            case 8: return execBVBVVector8;
            }
            break;
        case S_RL:
            printf("Illegal .. can not compute Bit-vector with RL storage!\n");
            exit(0);
            break;
        default:
            printf("Illegal storage format %d\n", storage_format);
        }
        break;
    case C_RL:
        switch (storage_format)
        {
        case S_DENSE:
            printf("Illegal .. can not compute Run-Length with Dense storage!\n");
            exit(0);
            break;
        case S_CSR:
            printf("Illegal .. can not compute Run-Length with CSR storage!\n");
            exit(0);
            break;
        case S_BV:
            printf("Illegal .. can not compute Run-Length with Bit-Vector storage!\n");
            exit(0);
            break;
        case S_RL:
            switch (vector_size)
            {
            case 1: return execRLRLScalar;
            case 2: return execRLRLVector2;
            case 4: return execRLRLVector4;
            case 8: return execRLRLVector8;
            }
            break;
        default:
            printf("Illegal storage format %d\n", storage_format);
        }
        break;
    case C_HHT_DENSE:
        switch (storage_format)
        {
        case S_DENSE:
            printf("Illegal .. can not do HHT_Dense with Dense storage!\n");
            exit(0);
            break;
        case S_CSR:
            switch (vector_size)
            {
            case 1: return execHelperDenseCSRScalar;
            case 2: return execHelperDenseCSRVector2;
            case 4: return execHelperDenseCSRVector4;
            case 8: return execHelperDenseCSRVector8;
            }
            break;
        case S_BV:
            switch (vector_size)
            {
            case 1: return execHelperDenseBVScalar;
            case 2: return execHelperDenseBVVector2;
            case 4: return execHelperDenseBVVector4;
            case 8: return execHelperDenseBVVector8;
            }
            break;
        case S_RL:
            switch (vector_size)
            {
            case 1: return execHelperDenseRLScalar;
            case 2: return execHelperDenseRLVector2;
            case 4: return execHelperDenseRLVector4;
            case 8: return execHelperDenseRLVector8;
            }
            break;
        default:
            printf("Illegal storage format %d\n", storage_format);
        }
        break;
    case C_HHT_SPARSE:
        switch (storage_format)
        {
        case S_DENSE:
            printf("Illegal .. can not do HHT_Sparse with Dense storage!\n");
            exit(0);
            break;
        case S_CSR:
            switch (vector_size)
            {
            case 1: return execHelperSparseCSRScalar;
            case 2: return execHelperSparseCSRVector2;
            case 4: return execHelperSparseCSRVector4;
            case 8: return execHelperSparseCSRVector8;
            }
            break;
        case S_BV:
            switch (vector_size)
            {
            case 1: return execHelperSparseBVScalar;
            case 2: return execHelperSparseBVVector2;
            case 4: return execHelperSparseBVVector4;
            case 8: return execHelperSparseBVVector8;
            }
            break;
        case S_RL:
            switch (vector_size)
            {
            case 1: return execHelperSparseRLScalar;
            case 2: return execHelperSparseRLVector2;
            case 4: return execHelperSparseRLVector4;
            case 8: return execHelperSparseRLVector8;
            }
            break;
        default:
            printf("Illegal storage format %d\n", storage_format);
        }
        break;
    default:
        printf("Illegal compute format %d\n", compute_format);
        break;
    }
    return 0;
}
#endif
