#ifndef _RV32_VEC_INS_H
#define _RV32_VEC_INS_H

#define V0 0
#define V1 1
#define V2 2
#define V3 3
#define V4 4
#define V5 5
#define V6 6
#define V7 7
#define V8 8
#define V9 9
#define V10 10
#define V11 11
#define V12 12
#define V13 13
#define V14 14
#define V15 15
#define V16 16
#define V17 17
#define V18 18
#define V19 19
#define V20 20
#define V21 21
#define V22 22
#define V23 23
#define V24 24
#define V25 25
#define V26 26
#define V27 27
#define V28 28
#define V29 29
#define V30 31
#define V31 31

#define VM 1
#define NO_VM 0

#define R0 0
#define R1 1
#define R2 2
#define R3 3
#define R4 4
#define R5 5
#define R6 6
#define R7 7
#define R8 8
#define R9 9
#define R10 10
#define R11 11
#define R12 12
#define R13 13 
#define R14 14
#define R15 15 
#define R16 16 
#define R17 17 
#define R18 18 
#define R19 19 
#define R20 20 
#define R21 21
#define R22 22 
#define R23 23 
#define R24 24
#define R25 25
#define R26 26
#define R27 27
#define R28 28
#define R29 29
#define R30 30
#define R31 31

#define E8    (0b000)
#define E16   (0b001)
#define E32   (0b010)
#define E64   (0b011)
#define E128  (0b100)
#define E256  (0b101)
#define E512  (0b110)
#define E1024 (0b111)
#define VSEW_OFFSET (2)

#define OP_IMM (0b0010011)
#define OP_LUI (0b0110111)
#define OP_SUB_R_R (0b0110011)
#define OP_LDW (0b0000011)
#define OP_STW (0b0100011)

#define FUNCT3_ADDI (0b000)
#define FUNCT3_ORI  (0b110)
#define FUNCT3_SUB_R_R  (0b000)
#define FUNCT3_LDW  (0b010)
#define FUNCT3_STW  (0b010)

#define FUNCT7_SUB_R_R (0b0100000)

#define FUNCT6_VADD_VV (0b000000)
#define FUNCT6_VMUL_VV (0b100101)
#define FUNCT6_VREDSUM_VS (0b000000)

#define OP_V (0b1010111)
#define OP_V_OPIVV (0b000)
#define OP_V_OPFVV (0b001)
#define OP_V_OPMVV (0b010)
#define OP_V_OPIVI (0b011)
#define OP_V_OPIVX (0b100)
#define OP_V_OPFVF (0b101)
#define OP_V_OPMVX (0b110)

#define ADD_IMM(rd, rs1, imm) (OP_IMM | (rd << 7) | (FUNCT3_ADDI << 12) | (rs1 << 15) | (imm << 20))
#define SUB_IMM(rd, rs1, imm) ADD_IMM(rd, rs1, -imm)

#define OR_IMM(rd, rs1, imm) (OP_IMM | (rd << 7) | (FUNCT3_ORI << 12) | (rs1 << 15) | (imm << 20))

#define LD_IMM(rd, imm) (OP_LUI | (rd << 7) |  (imm << 12))

#define LDW(rd, rs1, imm) (OP_LDW | (rd << 7) | (FUNCT3_LDW << 12) | (rs1 << 15) | (imm << 20)) 
#define STW(rd, rs, imm) (OP_STW | ((imm & 0x1f) << 7) | (rd << 15) | (FUNCT3_STW << 12) | (rs << 20) | (((imm >> 5) & 0x7f ) << 25)) 

#define SUB_R_R(rd, rs1, rs2) (OP_SUB_R_R | (rd << 7) | (FUNCT3_SUB_R_R << 12) | (rs1 << 15) | (rs2 << 20) | (FUNCT7_SUB_R_R << 25) )

#define VADD_VV(vd, vs1, vs2, vm) (OP_V | (vd << 7) | (OP_V_OPIVV << 12) | (vs1 << 15) | (vs2 << 20) | (vm << 25) | (FUNCT6_VADD_VV << 26))

#define VMUL_VV(vd, vs1, vs2, vm) (OP_V | (vd << 7) | (OP_V_OPMVV << 12) | (vs1 << 15) | (vs2 << 20) | (vm << 25) | (FUNCT6_VMUL_VV << 26))

#define VREDSUM_VS(vd, vs1, vs2, vm) (OP_V | (vd << 7) | (OP_V_OPMVV << 12) | (vs1 << 15) | (vs2 << 20) | (vm << 25) | (FUNCT6_VREDSUM_VS << 26))

#define VSETVLI(rd, rs1, imm) (OP_V | (rd << 7) | (0b111 << 12) | (rs1 << 15) | (imm << 20))

#define VLWV(vd, rs1) ( 0b0000111 | (vd << 7) | (0b110 << 12) | (rs1 << 15) | (0b00000 << 20) | (0b1<<25) | (0b000<<26) | (0b000<<29) )

#define VSWV(rs, vs) ( 0b0100111 | (vs << 7) | (0b110 << 12) | (rs << 15) | (0b00000 << 20) | (0b1<<25) | (0b000<<26) | (0b000<<29) )

void generateAsm(char * mnemonic, unsigned long opcode);

#endif
