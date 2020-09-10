	.file	"vec_add.c"
	.option nopic
	.attribute arch, "rv32i2p0_m2p0_c2p0"
	.attribute unaligned_access, 0
	.attribute stack_align, 16
	.text
	.align	1
	.globl	_Z11read_cyclesv
	.type	_Z11read_cyclesv, @function
_Z11read_cyclesv:
.LFB1:
	.cfi_startproc
	addi	sp,sp,-32
	.cfi_def_cfa_offset 32
	sw	s0,28(sp)
	.cfi_offset 8, -4
	addi	s0,sp,32
	.cfi_def_cfa 8, 0
 #APP
# 7 "vec_add.c" 1
	LABEL: rdcycle a5
# 0 "" 2
 #NO_APP
	sw	a5,-20(s0)
	lw	a5,-20(s0)
	mv	a0,a5
	lw	s0,28(sp)
	.cfi_restore 8
	.cfi_def_cfa 2, 32
	addi	sp,sp,32
	.cfi_def_cfa_offset 0
	jr	ra
	.cfi_endproc
.LFE1:
	.size	_Z11read_cyclesv, .-_Z11read_cyclesv
	.section	.text._Z16vadd_vv_0_1_2_vmv,"axG",@progbits,_Z16vadd_vv_0_1_2_vmv,comdat
	.align	1
	.weak	_Z16vadd_vv_0_1_2_vmv
	.type	_Z16vadd_vv_0_1_2_vmv, @function
_Z16vadd_vv_0_1_2_vmv:
.LFB2:
	.cfi_startproc
	addi	sp,sp,-16
	.cfi_def_cfa_offset 16
	sw	s0,12(sp)
	.cfi_offset 8, -4
	addi	s0,sp,16
	.cfi_def_cfa 8, 0
 #APP
# 13 "vec_add.c" 1
	.word 0x2208057
# 0 "" 2
 #NO_APP
	nop
	lw	s0,12(sp)
	.cfi_restore 8
	.cfi_def_cfa 2, 16
	addi	sp,sp,16
	.cfi_def_cfa_offset 0
	jr	ra
	.cfi_endproc
.LFE2:
	.size	_Z16vadd_vv_0_1_2_vmv, .-_Z16vadd_vv_0_1_2_vmv
	.section	.text._Z14vsetvli_1_2_32v,"axG",@progbits,_Z14vsetvli_1_2_32v,comdat
	.align	1
	.weak	_Z14vsetvli_1_2_32v
	.type	_Z14vsetvli_1_2_32v, @function
_Z14vsetvli_1_2_32v:
.LFB3:
	.cfi_startproc
	addi	sp,sp,-16
	.cfi_def_cfa_offset 16
	sw	s0,12(sp)
	.cfi_offset 8, -4
	addi	s0,sp,16
	.cfi_def_cfa 8, 0
 #APP
# 18 "vec_add.c" 1
	.word 0x817057
# 0 "" 2
 #NO_APP
	nop
	lw	s0,12(sp)
	.cfi_restore 8
	.cfi_def_cfa 2, 16
	addi	sp,sp,16
	.cfi_def_cfa_offset 0
	jr	ra
	.cfi_endproc
.LFE3:
	.size	_Z14vsetvli_1_2_32v, .-_Z14vsetvli_1_2_32v
	.section	.text._Z15add_imm_2_0_256v,"axG",@progbits,_Z15add_imm_2_0_256v,comdat
	.align	1
	.weak	_Z15add_imm_2_0_256v
	.type	_Z15add_imm_2_0_256v, @function
_Z15add_imm_2_0_256v:
.LFB4:
	.cfi_startproc
	addi	sp,sp,-16
	.cfi_def_cfa_offset 16
	sw	s0,12(sp)
	.cfi_offset 8, -4
	addi	s0,sp,16
	.cfi_def_cfa 8, 0
 #APP
# 23 "vec_add.c" 1
	.word 0x10000013
# 0 "" 2
 #NO_APP
	nop
	lw	s0,12(sp)
	.cfi_restore 8
	.cfi_def_cfa 2, 16
	addi	sp,sp,16
	.cfi_def_cfa_offset 0
	jr	ra
	.cfi_endproc
.LFE4:
	.size	_Z15add_imm_2_0_256v, .-_Z15add_imm_2_0_256v
	.section	.text._Z11ld_imm_10_Av,"axG",@progbits,_Z11ld_imm_10_Av,comdat
	.align	1
	.weak	_Z11ld_imm_10_Av
	.type	_Z11ld_imm_10_Av, @function
_Z11ld_imm_10_Av:
.LFB5:
	.cfi_startproc
	addi	sp,sp,-16
	.cfi_def_cfa_offset 16
	sw	s0,12(sp)
	.cfi_offset 8, -4
	addi	s0,sp,16
	.cfi_def_cfa 8, 0
 #APP
# 28 "vec_add.c" 1
	.word 0x7fbeb537
# 0 "" 2
 #NO_APP
	nop
	lw	s0,12(sp)
	.cfi_restore 8
	.cfi_def_cfa 2, 16
	addi	sp,sp,16
	.cfi_def_cfa_offset 0
	jr	ra
	.cfi_endproc
.LFE5:
	.size	_Z11ld_imm_10_Av, .-_Z11ld_imm_10_Av
	.section	.text._Z15add_imm_10_10_Av,"axG",@progbits,_Z15add_imm_10_10_Av,comdat
	.align	1
	.weak	_Z15add_imm_10_10_Av
	.type	_Z15add_imm_10_10_Av, @function
_Z15add_imm_10_10_Av:
.LFB6:
	.cfi_startproc
	addi	sp,sp,-16
	.cfi_def_cfa_offset 16
	sw	s0,12(sp)
	.cfi_offset 8, -4
	addi	s0,sp,16
	.cfi_def_cfa 8, 0
 #APP
# 33 "vec_add.c" 1
	.word 0xd4050513
# 0 "" 2
 #NO_APP
	nop
	lw	s0,12(sp)
	.cfi_restore 8
	.cfi_def_cfa 2, 16
	addi	sp,sp,16
	.cfi_def_cfa_offset 0
	jr	ra
	.cfi_endproc
.LFE6:
	.size	_Z15add_imm_10_10_Av, .-_Z15add_imm_10_10_Av
	.section	.text._Z11ld_imm_11_Cv,"axG",@progbits,_Z11ld_imm_11_Cv,comdat
	.align	1
	.weak	_Z11ld_imm_11_Cv
	.type	_Z11ld_imm_11_Cv, @function
_Z11ld_imm_11_Cv:
.LFB7:
	.cfi_startproc
	addi	sp,sp,-16
	.cfi_def_cfa_offset 16
	sw	s0,12(sp)
	.cfi_offset 8, -4
	addi	s0,sp,16
	.cfi_def_cfa 8, 0
 #APP
# 38 "vec_add.c" 1
	.word 0x7fbeb5b7
# 0 "" 2
 #NO_APP
	nop
	lw	s0,12(sp)
	.cfi_restore 8
	.cfi_def_cfa 2, 16
	addi	sp,sp,16
	.cfi_def_cfa_offset 0
	jr	ra
	.cfi_endproc
.LFE7:
	.size	_Z11ld_imm_11_Cv, .-_Z11ld_imm_11_Cv
	.section	.text._Z15add_imm_11_11_Cv,"axG",@progbits,_Z15add_imm_11_11_Cv,comdat
	.align	1
	.weak	_Z15add_imm_11_11_Cv
	.type	_Z15add_imm_11_11_Cv, @function
_Z15add_imm_11_11_Cv:
.LFB8:
	.cfi_startproc
	addi	sp,sp,-16
	.cfi_def_cfa_offset 16
	sw	s0,12(sp)
	.cfi_offset 8, -4
	addi	s0,sp,16
	.cfi_def_cfa 8, 0
 #APP
# 43 "vec_add.c" 1
	.word 0xd0058593
# 0 "" 2
 #NO_APP
	nop
	lw	s0,12(sp)
	.cfi_restore 8
	.cfi_def_cfa 2, 16
	addi	sp,sp,16
	.cfi_def_cfa_offset 0
	jr	ra
	.cfi_endproc
.LFE8:
	.size	_Z15add_imm_11_11_Cv, .-_Z15add_imm_11_11_Cv
	.section	.rodata
	.align	2
.LC2:
	.string	"add_imm opcode: 0x%x\n"
	.align	2
.LC3:
	.string	"vsetvli opcode: 0x%x\n"
	.align	2
.LC4:
	.string	"vadd opcode: 0x%x\n"
	.align	2
.LC5:
	.string	"A array base address: 0x%x\n"
	.align	2
.LC6:
	.string	"A array address upper 20 bits: 0x%x\n"
	.align	2
.LC7:
	.string	"A array address lower 12 bits: 0x%x\n"
	.align	2
.LC8:
	.string	"ld_imm opcode: 0x%x\n"
	.align	2
.LC9:
	.string	"C array base address: 0x%x\n"
	.align	2
.LC10:
	.string	"C array address upper 20 bits: 0x%x\n"
	.align	2
.LC11:
	.string	"C array address lower 12 bits: 0x%x\n"
	.align	2
.LC12:
	.string	"Took %d cycles\n"
	.align	2
.LC0:
	.word	0
	.word	1
	.word	2
	.word	3
	.word	4
	.word	5
	.word	6
	.word	7
	.align	2
.LC1:
	.word	8
	.word	9
	.word	10
	.word	11
	.word	12
	.word	13
	.word	14
	.word	15
	.text
	.align	1
	.globl	main
	.type	main, @function
main:
.LFB9:
	.cfi_startproc
	addi	sp,sp,-160
	.cfi_def_cfa_offset 160
	sw	ra,156(sp)
	sw	s0,152(sp)
	.cfi_offset 1, -4
	.cfi_offset 8, -8
	addi	s0,sp,160
	.cfi_def_cfa 8, 0
	lui	a5,%hi(.LC0)
	addi	a5,a5,%lo(.LC0)
	lw	a7,0(a5)
	lw	a6,4(a5)
	lw	a0,8(a5)
	lw	a1,12(a5)
	lw	a2,16(a5)
	lw	a3,20(a5)
	lw	a4,24(a5)
	lw	a5,28(a5)
	sw	a7,-96(s0)
	sw	a6,-92(s0)
	sw	a0,-88(s0)
	sw	a1,-84(s0)
	sw	a2,-80(s0)
	sw	a3,-76(s0)
	sw	a4,-72(s0)
	sw	a5,-68(s0)
	lui	a5,%hi(.LC1)
	addi	a5,a5,%lo(.LC1)
	lw	a7,0(a5)
	lw	a6,4(a5)
	lw	a0,8(a5)
	lw	a1,12(a5)
	lw	a2,16(a5)
	lw	a3,20(a5)
	lw	a4,24(a5)
	lw	a5,28(a5)
	sw	a7,-128(s0)
	sw	a6,-124(s0)
	sw	a0,-120(s0)
	sw	a1,-116(s0)
	sw	a2,-112(s0)
	sw	a3,-108(s0)
	sw	a4,-104(s0)
	sw	a5,-100(s0)
	sw	zero,-160(s0)
	sw	zero,-156(s0)
	sw	zero,-152(s0)
	sw	zero,-148(s0)
	sw	zero,-144(s0)
	sw	zero,-140(s0)
	sw	zero,-136(s0)
	sw	zero,-132(s0)
	li	a5,268435456
	addi	a5,a5,275
	sw	a5,-20(s0)
	lw	a1,-20(s0)
	lui	a5,%hi(.LC2)
	addi	a0,a5,%lo(.LC2)
	call	printf
	li	a5,8482816
	addi	a5,a5,87
	sw	a5,-24(s0)
	lw	a1,-24(s0)
	lui	a5,%hi(.LC3)
	addi	a0,a5,%lo(.LC3)
	call	printf
	li	a5,35684352
	addi	a5,a5,87
	sw	a5,-28(s0)
	lw	a1,-28(s0)
	lui	a5,%hi(.LC4)
	addi	a0,a5,%lo(.LC4)
	call	printf
	addi	a5,s0,-96
	mv	a1,a5
	lui	a5,%hi(.LC5)
	addi	a0,a5,%lo(.LC5)
	call	printf
	addi	a5,s0,-96
	sw	a5,-32(s0)
	addi	a5,s0,-96
	sw	a5,-36(s0)
	lw	a5,-32(s0)
	srai	a5,a5,12
	sw	a5,-32(s0)
	lw	a4,-36(s0)
	li	a5,4096
	addi	a5,a5,-1
	and	a5,a4,a5
	sw	a5,-36(s0)
	lw	a1,-32(s0)
	lui	a5,%hi(.LC6)
	addi	a0,a5,%lo(.LC6)
	call	printf
	lw	a1,-36(s0)
	lui	a5,%hi(.LC7)
	addi	a0,a5,%lo(.LC7)
	call	printf
	lw	a5,-32(s0)
	slli	a5,a5,12
	ori	a5,a5,1335
	sw	a5,-40(s0)
	lw	a1,-40(s0)
	lui	a5,%hi(.LC8)
	addi	a0,a5,%lo(.LC8)
	call	printf
	lw	a5,-36(s0)
	slli	a4,a5,20
	li	a5,327680
	addi	a5,a5,1299
	or	a5,a4,a5
	sw	a5,-44(s0)
	lw	a1,-44(s0)
	lui	a5,%hi(.LC2)
	addi	a0,a5,%lo(.LC2)
	call	printf
	addi	a5,s0,-160
	mv	a1,a5
	lui	a5,%hi(.LC9)
	addi	a0,a5,%lo(.LC9)
	call	printf
	addi	a5,s0,-160
	sw	a5,-48(s0)
	addi	a5,s0,-160
	sw	a5,-52(s0)
	lw	a5,-48(s0)
	srai	a5,a5,12
	sw	a5,-48(s0)
	lw	a4,-52(s0)
	li	a5,4096
	addi	a5,a5,-1
	and	a5,a4,a5
	sw	a5,-52(s0)
	lw	a1,-48(s0)
	lui	a5,%hi(.LC10)
	addi	a0,a5,%lo(.LC10)
	call	printf
	lw	a1,-52(s0)
	lui	a5,%hi(.LC11)
	addi	a0,a5,%lo(.LC11)
	call	printf
	lw	a5,-48(s0)
	slli	a5,a5,12
	ori	a5,a5,1463
	sw	a5,-40(s0)
	lw	a1,-40(s0)
	lui	a5,%hi(.LC8)
	addi	a0,a5,%lo(.LC8)
	call	printf
	lw	a5,-52(s0)
	slli	a4,a5,20
	li	a5,360448
	addi	a5,a5,1427
	or	a5,a4,a5
	sw	a5,-56(s0)
	lw	a1,-56(s0)
	lui	a5,%hi(.LC2)
	addi	a0,a5,%lo(.LC2)
	call	printf
	call	_Z11read_cyclesv
	sw	a0,-60(s0)
	call	_Z15add_imm_2_0_256v
	call	_Z14vsetvli_1_2_32v
	call	_Z16vadd_vv_0_1_2_vmv
	call	_Z11ld_imm_10_Av
	call	_Z15add_imm_10_10_Av
	call	_Z11ld_imm_11_Cv
	call	_Z15add_imm_11_11_Cv
	call	_Z11read_cyclesv
	sw	a0,-64(s0)
	lw	a4,-64(s0)
	lw	a5,-60(s0)
	sub	a5,a4,a5
	mv	a1,a5
	lui	a5,%hi(.LC12)
	addi	a0,a5,%lo(.LC12)
	call	printf
	li	a5,0
	mv	a0,a5
	lw	ra,156(sp)
	.cfi_restore 1
	lw	s0,152(sp)
	.cfi_restore 8
	.cfi_def_cfa 2, 160
	addi	sp,sp,160
	.cfi_def_cfa_offset 0
	jr	ra
	.cfi_endproc
.LFE9:
	.size	main, .-main
	.ident	"GCC: (GNU) 9.2.0"
