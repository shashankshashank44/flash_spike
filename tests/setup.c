#include <stdio.h>
#include <context_sr_macros.h>
#include <rv32_vec_ins.h>

void save_regs(void)
{
    sub_sp_sp_imm_4_macro;
    stw_sp_r1_macro;
    sub_sp_sp_imm_4_macro;
    stw_sp_r2_macro;
    sub_sp_sp_imm_4_macro;
    stw_sp_r3_macro;
    sub_sp_sp_imm_4_macro;
    stw_sp_r4_macro;
}

void restore_regs(void)
{
    ldw_r4_sp_macro;
    add_sp_sp_imm_4_macro;
    ldw_r3_sp_macro;
    add_sp_sp_imm_4_macro;
    ldw_r2_sp_macro;
    add_sp_sp_imm_4_macro;
    ldw_r1_sp_macro;
    add_sp_sp_imm_4_macro;
}

#ifdef GENERATE
int main()
{
    generateAsm("sub_sp_sp_imm_4_macro", SUB_IMM(R2, R2, 4));
    generateAsm("add_sp_sp_imm_4_macro", ADD_IMM(R2, R2, 4));
    generateAsm("ldw_r4_sp_macro", LDW(R4, R2, 0));
    generateAsm("stw_sp_r4_macro", STW(R2, R4, 0));
    generateAsm("ldw_r3_sp_macro", LDW(R3, R2, 0));
    generateAsm("stw_sp_r3_macro", STW(R2, R3, 0));
    generateAsm("ldw_r2_sp_macro", LDW(R2, R2, 0));
    generateAsm("stw_sp_r2_macro", STW(R2, R2, 0));
    generateAsm("ldw_r1_sp_macro", LDW(R1, R2, 0));
    generateAsm("stw_sp_r1_macro", STW(R2, R1, 0));
    return 0;
}
#endif
