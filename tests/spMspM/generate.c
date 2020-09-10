#include <rv32_vec_ins.h>
#include <stdio.h>

void generateAsm(char * mnemonic, unsigned long opcode)
{
    printf("#define %s asm volatile (\".word 0x%x\")\n", mnemonic, opcode);
}

