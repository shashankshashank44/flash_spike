	.file	"hello.c"
	.option nopic
	.attribute arch, "rv32i2p0_m2p0_c2p0"
	.attribute unaligned_access, 0
	.attribute stack_align, 16
	.text
	.align	1
	.globl	read_cycles
	.type	read_cycles, @function
read_cycles:
	addi	sp,sp,-32
	sw	s0,28(sp)
	addi	s0,sp,32
 #APP
# 6 "hello.c" 1
	LABEL: rdcycle a5
# 0 "" 2
 #NO_APP
	sw	a5,-20(s0)
	lw	a5,-20(s0)
	mv	a0,a5
	lw	s0,28(sp)
	addi	sp,sp,32
	jr	ra
	.size	read_cycles, .-read_cycles
	.section	.rodata
	.align	2
.LC0:
	.string	"Hello world"
	.align	2
.LC1:
	.string	"Sum is %d\n"
	.align	2
.LC2:
	.string	"Loop took %d cycles\n"
	.text
	.align	1
	.globl	main
	.type	main, @function
main:
	addi	sp,sp,-48
	sw	ra,44(sp)
	sw	s0,40(sp)
	addi	s0,sp,48
	li	a5,3
	sw	a5,-28(s0)
	li	a5,3
	sw	a5,-32(s0)
	lui	a5,%hi(.LC0)
	addi	a0,a5,%lo(.LC0)
	call	puts
	sw	zero,-20(s0)
	call	read_cycles
	sw	a0,-36(s0)
	sw	zero,-24(s0)
	j	.L4
.L5:
	lw	a4,-20(s0)
	lw	a5,-24(s0)
	add	a5,a4,a5
	sw	a5,-20(s0)
	lw	a5,-24(s0)
	addi	a5,a5,1
	sw	a5,-24(s0)
.L4:
	lw	a4,-24(s0)
	li	a5,4096
	blt	a4,a5,.L5
	call	read_cycles
	sw	a0,-40(s0)
	lw	a1,-20(s0)
	lui	a5,%hi(.LC1)
	addi	a0,a5,%lo(.LC1)
	call	printf
	lw	a4,-40(s0)
	lw	a5,-36(s0)
	sub	a5,a4,a5
	mv	a1,a5
	lui	a5,%hi(.LC2)
	addi	a0,a5,%lo(.LC2)
	call	printf
	lw	a4,-28(s0)
	lw	a5,-32(s0)
	add	a5,a4,a5
	mv	a0,a5
	lw	ra,44(sp)
	lw	s0,40(sp)
	addi	sp,sp,48
	jr	ra
	.size	main, .-main
	.ident	"GCC: (GNU) 9.2.0"
