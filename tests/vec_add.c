#include <stdio.h>
#include <rv32_vec_ins.h>
#include <generated_macros.h>

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

int main()
{
    int a[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
    int b[] = {8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27};

    int c[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    // we want to define a vector of size 256 bits
    // and element size of 32 bits. 
    // which means each vector register can store 8 elements
    // we are passing the VLEN value through scalar register 2.

    generateAsm("add_r6_r0_imm_256_macro", ADD_IMM(R6,R0,256));
    generateAsm("vsetvli_r7_r6_e32_macro", VSETVLI(R7, R6, (E32 << VSEW_OFFSET)));

    //printf("A array base address: 0x%x\n", &a[0]);
    int a_upper_20 = (int)(&a[0]);
    int a_lower_12 = (int)(&a[0]);
    a_upper_20 = a_upper_20 >> 12;
    a_lower_12 = a_lower_12 & 0xfff;
    //printf("A array address upper 20 bits: 0x%x\n", a_upper_20);
    //printf("A array address lower 12 bits: 0x%x\n", a_lower_12);
    generateAsm("ld_r10_a_upper_20_macro", LD_IMM(R10, a_upper_20));

    generateAsm("add_r10_r10_imm_a_lower_12_macro", ADD_IMM(R10, R10, a_lower_12));

    //printf("B array base address: 0x%x\n", &b[0]);
    int b_upper_20 = (int)(&b[0]);
    int b_lower_12 = (int)(&b[0]);
    b_upper_20 = b_upper_20 >> 12;
    b_lower_12 = b_lower_12 & 0xfff;
    //printf("B array address upper 20 bits: 0x%x\n", b_upper_20);
    //printf("B array address lower 12 bits: 0x%x\n", b_lower_12);
    generateAsm("ld_r13_b_upper_20_macro", LD_IMM(R13, b_upper_20));
    generateAsm("add_r13_r13_imm_b_lower_12_macro", ADD_IMM(R13, R13, b_lower_12));

    //printf("C array base address: 0x%x\n", &c[0]);
    int c_upper_20 = (int)(&c[0]);
    int c_lower_12 = (int)(&c[0]);
    c_upper_20 = c_upper_20 >> 12;
    c_lower_12 = c_lower_12 & 0xfff;
    //printf("C array address upper 20 bits: 0x%x\n", c_upper_20);
    //printf("C array address lower 12 bits: 0x%x\n", c_lower_12);
    generateAsm("ld_r11_c_upper_20_macro", LD_IMM(R11, c_upper_20));
    generateAsm("add_r11_r11_imm_c_lower_12_macro", ADD_IMM(R11, R11, c_lower_12));
    generateAsm("ld_r12_imm_0xfffff_macro", LD_IMM(R12, 0xfffff));
    generateAsm("sub_r10_r10_r12_macro", SUB_R_R(R10,R10,R12));
    generateAsm("sub_r11_r11_r12_macro", SUB_R_R(R11,R11,R12));
    generateAsm("sub_r13_r13_r12_macro", SUB_R_R(R13,R13,R12));
    generateAsm("vlwv_v10_r10_macro", VLWV(V10,R10));
    generateAsm("vlwv_v13_r13_macro", VLWV(V13,R13));
    generateAsm("vadd_vv_v11_v10_v13_vm_macro", VADD_VV(V11,V10,V13,VM));
    generateAsm("vswv_r11_v11_macro", VSWV(R11,V11));
    generateAsm("add_10_10_imm_256_macro", ADD_IMM(R10,R10,0b000000100000));
    generateAsm("add_11_11_imm_256_macro", ADD_IMM(R11,R11,0b000000100000));
    generateAsm("add_13_13_imm_256_macro", ADD_IMM(R13,R13,0b000000100000));
    
    // call vector opcodes
    unsigned long s, e;
    s = read_cycles();
    add_r6_r0_imm_256_macro;
    vsetvli_r7_r6_e32_macro;

    ld_r10_a_upper_20_macro;
    add_r10_r10_imm_a_lower_12_macro;
    ld_r12_imm_0xfffff_macro;
    if (a_lower_12 & 0x800) sub_r10_r10_r12_macro;

    ld_r11_c_upper_20_macro;
    add_r11_r11_imm_c_lower_12_macro;
    if (c_lower_12 & 0x800) sub_r11_r11_r12_macro;

    ld_r13_b_upper_20_macro;
    add_r13_r13_imm_b_lower_12_macro;
    if (b_lower_12 & 0x800) sub_r13_r13_r12_macro;

    for(int i=0; i<(sizeof(c)/sizeof(c[0])); i=i+8)
    {
        vlwv_v10_r10_macro;
        vlwv_v13_r13_macro;
        vadd_vv_v11_v10_v13_vm_macro;
        vswv_r11_v11_macro;
        add_10_10_imm_256_macro; 
        add_11_11_imm_256_macro; 
        add_13_13_imm_256_macro; 
    }
    e = read_cycles();
    
    // check if C now contains the sum of A and B
    int pass = 1;
    for (int i=0;i<sizeof(c)/sizeof(c[0]); i++ )
    {
        printf("%d ", c[i]);
    }
    printf("\n");

    printf("Took %d cycles\n", e-s);
    
    return 0;
}
