	.text
	.attribute	4, 16
	.attribute	5, "rv32i2p0_m2p0_a2p0_zfinx1p0_zdinx1p0_zve32f1p0_zve32x1p0_zvl32b1p0_zhinx1p0"
	.file	"test.cl"
	.section	.ventus.resource.test,"w",@progbits
	.half	4
	.half	7
	.half	16
	.half	8
	.text
	.globl	test
	.p2align	2
	.type	test,@function
test:
	addi	sp, sp, 16
	addi	tp, tp, 8
	regext	zero, zero, 1
	vmv.v.x	v32, tp
	sw	ra, -16(sp)
	regext	zero, zero, 72
	vsw.v	v33, -4(v32)
	regext	zero, zero, 72
	vsw.v	v34, -8(v32)
	lw	t0, 0(a0)
	sw	t0, -4(sp)
	lw	t0, 4(a0)
	sw	t0, -8(sp)
	lw	t0, 8(a0)
	sw	t0, -12(sp)
	vmv.v.x	v0, zero
	call	_Z13get_global_idj
	regext	zero, zero, 1
	vsll.vi	v34, v0, 2
	lw	t1, -12(sp)
	lw	t0, -8(sp)
	regext	zero, zero, 64
	vadd.vx	v0, v34, t0
	.insn i 0x76, 0x2, v1, v0, 0
	regext	zero, zero, 65
	vadd.vx	v33, v34, t1
	regext	zero, zero, 64
	vadd.vx	v0, v33, zero
	call	foo
	regext	zero, zero, 8
	.insn i 0x7a, 0x2, v0, v33, 0
	lw	t0, -4(sp)
	regext	zero, zero, 64
	vadd.vx	v1, v34, t0
	vsw12.v	v0, 0(v1)
	lw	ra, -16(sp)
	regext	zero, zero, 9
	.insn i 0x7e, 0x2, v33, v32, -4
	regext	zero, zero, 9
	.insn i 0x7e, 0x2, v34, v32, -8
	addi	sp, sp, -16
	addi	tp, tp, -8
	regext	zero, zero, 1
	vmv.v.x	v32, tp
	ret
.Lfunc_end0:
	.size	test, .Lfunc_end0-test

	.globl	foo
	.p2align	2
	.type	foo,@function
foo:
	vmsle.vi	v1, v1, 0
	vrsub.vi	v1, v1, 0
	vor.vi	v1, v1, 1
	vsw12.v	v1, 0(v0)
	ret
.Lfunc_end1:
	.size	foo, .Lfunc_end1-foo

	.ident	"clang version 16.0.0 (git@github.com:THU-DSP-LAB/llvm-project.git 7411068a453d19fe0fc296530e71879869338a23)"
	.section	".note.GNU-stack","",@progbits
	.addrsig
