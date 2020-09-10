#include <stdio.h>
#include <rv32_vec_ins.h>
#include <generated_macros.h>
#include <fstream>
#include <string>
#include <iostream>

using namespace std;

int a_lower_12, b_lower_12, c_lower_12, n = 16;

unsigned long read_cycles(void)
{
  unsigned long cycles;
  asm volatile ("LABEL: rdcycle %0" : "=r" (cycles));
  return cycles;
}

void func1(void)
{
    asm volatile (".word 0x10000313");
}

void func2(void)
{
    asm volatile (".word 0x8371d7");
}

void gen_code()
{
    add_r6_r0_imm_256_macro;
    vsetvli_r7_r6_e32_macro;
    //func1();
    //func2();

    ld_r18_a_upper_20_macro;
    add_r18_r18_imm_a_lower_12_macro;
    ld_r21_imm_0xfffff_macro;
    if (a_lower_12 & 0x800) sub_r18_r18_r21_macro;
    //printf("loaded address of A\n");

    ld_r19_b_upper_20_macro;
    add_r19_r19_imm_b_lower_12_macro;
    if (b_lower_12 & 0x800) sub_r19_r19_r21_macro;
    //printf("loaded address of C\n");

    ld_r20_c_upper_20_macro;
    add_r20_r20_imm_c_lower_12_macro;
    if (c_lower_12 & 0x800) sub_r20_r20_r21_macro;
    //printf("loaded address of B\n");
    
    //copy base address of b and c arrays

    cp_r22_r19_r0_macro;


    for(int i=0; i<n; i++)
    {
        for(int j=0; j<n; j=j+8)
        {
            vlwv_v18_r18_macro;
            vlwv_v19_r19_macro;
            vmul_vv_v20_v18_v19_vm_macro;
            vredsum_vs_v21_v21_v20_macro;
            //ld_r25_r20_macro;
            //vswv_r20_v21_macro;                 //may be seg fault if vector is big
            //ld_r26_r20_macro;
            //add_r27_r25_r26_macro;
            //st_r20_r27_macro;
            add_18_18_imm_256_macro; 
            add_19_19_imm_256_macro; 
            //add_20_20_imm_256_macro; 
        }
        vswv_r20_v21_macro;                 
        add_20_20_imm_32_macro;
        cp_r19_r22_r0_macro;
        reset_v21_r0_macro;
    }
}

int main(int argc, char** argv)
{
    std::ifstream traceIn;
    int *a, *b, *c;    // = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
        // = {8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27};
        // = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    a = (int*)malloc((n * n) * sizeof(int));
    b = (int*)malloc(n * sizeof(int));
    c = (int*)malloc(n * sizeof(int));


    traceIn.open(argv[1]);

    if(traceIn.is_open())
    {    
        int i = 0, star_passed = 0;
        while( !traceIn.eof() )
        {
            string line;
            getline(traceIn, line);
            if (line.compare("**") == 0)
                break;
            else if (line.compare("*") == 0)
            {
                i = 0;
                star_passed = 1;
            }
            else if(line.compare("*") != 0)
            {
                if(star_passed == 0)
                {
                    a[i] = stoi(line);
                    i++;
                }
                else
                {
                    b[i] = stoi(line);
                    i++;
                }
            }
        }
    }

    // we want to define a vector of size 256 bits
    // and element size of 32 bits. 
    // which means each vector register can store 8 elements
    // we are passing the VLEN value through scalar register 2.

    generateAsm("add_r6_r0_imm_256_macro", ADD_IMM(R6,R0,256));
    generateAsm("vsetvli_r7_r6_e32_macro", VSETVLI(R7, R6, (E32 << VSEW_OFFSET)));

    //printf("A array base address: 0x%x\n", &a[0]);
    //int a_upper_20 = (int)(&a[0]);
    //int a_lower_12 = (int)(&a[0]);
    int a_upper_20 = (int)(a);
    a_lower_12 = (int)(a);
    a_upper_20 = a_upper_20 >> 12;
    a_lower_12 = a_lower_12 & 0xfff;
    //printf("A array address upper 20 bits: 0x%x\n", a_upper_20);
    //printf("A array address lower 12 bits: 0x%x\n", a_lower_12);
    generateAsm("ld_r18_a_upper_20_macro", LD_IMM(R18, a_upper_20));

    generateAsm("add_r18_r18_imm_a_lower_12_macro", ADD_IMM(R18, R18, a_lower_12));

    //printf("B array base address: 0x%x\n", &b[0]);
    int b_upper_20 = (int)(b);
    b_lower_12 = (int)(b);
    b_upper_20 = b_upper_20 >> 12;
    b_lower_12 = b_lower_12 & 0xfff;
    //printf("B array address upper 20 bits: 0x%x\n", b_upper_20);
    //printf("B array address lower 12 bits: 0x%x\n", b_lower_12);
    generateAsm("ld_r19_b_upper_20_macro", LD_IMM(R19, b_upper_20));
    generateAsm("add_r19_r19_imm_b_lower_12_macro", ADD_IMM(R19, R19, b_lower_12));

    //printf("C array base address: 0x%x\n", &c[0]);
    int c_upper_20 = (int)(c);
    c_lower_12 = (int)(c);
    c_upper_20 = c_upper_20 >> 12;
    c_lower_12 = c_lower_12 & 0xfff;
    //printf("C array address upper 20 bits: 0x%x\n", c_upper_20);
    //printf("C array address lower 12 bits: 0x%x\n", c_lower_12);
    generateAsm("ld_r20_c_upper_20_macro", LD_IMM(R20, c_upper_20));
    generateAsm("add_r20_r20_imm_c_lower_12_macro", ADD_IMM(R20, R20, c_lower_12));
    
    generateAsm("ld_r21_imm_0xfffff_macro", LD_IMM(R21, 0xfffff));
    generateAsm("sub_r18_r18_r21_macro", SUB_R_R(R18,R18,R21));
    generateAsm("sub_r19_r19_r21_macro", SUB_R_R(R19,R19,R21));
    generateAsm("sub_r20_r20_r21_macro", SUB_R_R(R20,R20,R21));
    
    generateAsm("vlwv_v18_r18_macro", VLWV(V18,R18));
    generateAsm("vlwv_v19_r19_macro", VLWV(V19,R19));
    generateAsm("vadd_vv_v20_v18_v19_vm_macro", VADD_VV(V20,V18,V19,VM));
    generateAsm("vswv_r20_v21_macro", VSWV(R20,V21));
    generateAsm("add_18_18_imm_256_macro", ADD_IMM(R18,R18,0b000000100000));
    generateAsm("add_19_19_imm_256_macro", ADD_IMM(R19,R19,0b000000100000));
    generateAsm("add_20_20_imm_256_macro", ADD_IMM(R20,R20,0b000000100000));
   
    
    generateAsm("cp_r22_r19_r0_macro", ADD_R_R(R22,R19,R0));
    generateAsm("add_20_20_imm_32_macro", ADD_IMM(R20,R20,0b000000000100));
    generateAsm("cp_r19_r22_r0_macro", ADD_R_R(R19,R22,R0));
    generateAsm("vmul_vv_v20_v18_v19_vm_macro", VMUL_VV(V20,V18,V19,VM));
    generateAsm("vredsum_vs_v21_v21_v20_macro", VREDSUM_VS(V21, V21, V20, VM));
    generateAsm("ld_r25_r20_macro", LD_SIMM(R25,R20,0b000000000000));
    generateAsm("ld_r26_r20_macro", LD_SIMM(R26,R20,0b000000000000));
    generateAsm("add_r27_r25_r26_macro", ADD_R_R(R27,R25,R26));
    generateAsm("st_r20_r27_macro", ST_SIMM(R20,0b00000,0b0000000,R27));
    generateAsm("reset_v21_r0_macro", VMV_VS(V21,R0));
    
 
    // call vector opcodes
    unsigned long s, e;
    s = read_cycles();
    gen_code();
    e = read_cycles();
    
    // check if C now contains the sum of A and B
    for (int i=0;i<n*n; i++ )
    {
        printf("%d ", a[i]);
    }
    printf("\n");
    for (int i=0;i<n; i++ )
    {
        printf("%d ", b[i]);
    }
    printf("\n");
    int pass = 1;
    for (int i=0;i<n; i++ )
    {
        printf("%d ", c[i]);
    }
    printf("\n");
    /*for (int i=0;i<sizeof(a)/sizeof(a[0]); i++ )
    {
        if (a[i] != c[i]) { pass = 0; break;}
    }
    if (pass) printf("Self-check passed!\n");
    else printf("Self-check failed!\n");*/
     
    
    printf("Took %d cycles\n", e-s);
    
    free(a);
    free(b);
    free(c);
    
    return 0;
}
