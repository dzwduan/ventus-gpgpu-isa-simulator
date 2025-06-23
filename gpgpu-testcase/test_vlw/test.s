	.text
	.attribute	4, 16
	.attribute	5, "rv32i2p0_m2p0_a2p0_zfinx1p0_zdinx1p0_zve32f1p0_zve32x1p0_zvl32b1p0_zhinx1p0"
	.file	"test.cl"
	.section	.ventus.resource.test,"w",@progbits
	.half	4
	.half	7
	.half	12
	.half	4
	.text
	.globl	test
	.p2align	2
	.type	test,@function
test:
	addi	sp, sp, 12
	addi	tp, tp, 4
	regext	zero, zero, 1
	vmv.v.x	v32, tp
	sw	ra, -12(sp)
	lw	t0, 0(a0)
	sw	t0, -4(sp)
	lw	t0, 4(a0)
	sw	t0, -8(sp)
	vmv.v.x	v0, zero
	call	_Z13get_global_idj
	vadd.vx	v2, v0, zero
	vsll.vi	v0, v0, 2
	lw	t0, -8(sp)
	vadd.vx	v0, v0, t0
	vlw_global.v	v0, 0(v0)
	regext	zero, zero, 8
	vsw_private.v	v0, -4(v32)
	addi	t0, tp, -4
	lw	t1, -4(sp)
	vmv.v.x	v0, t1
	vmv.v.x	v3, t0
	call	foo
	lw	ra, -12(sp)
	addi	sp, sp, -12
	addi	tp, tp, -4
	regext	zero, zero, 1
	vmv.v.x	v32, tp
	ret
.Lfunc_end0:
	.size	test, .Lfunc_end0-test

	.globl	foo
	.p2align	2
	.type	foo,@function
foo:
	vlw_private.v	v1, 0(v3)
	vmsle.vi	v1, v1, 0
	vrsub.vi	v1, v1, 0
	vor.vi	v1, v1, 1
	vsll.vi	v2, v2, 2
	vadd.vv	v0, v0, v2
	vsw12.v	v1, 0(v0)
	ret
.Lfunc_end1:
	.size	foo, .Lfunc_end1-foo

	.ident	"clang version 16.0.0 (https://github.com/THU-DSP-LAB/llvm-project.git 69dd6693295c79a68395dc27356adff6928130ba)"
	.section	".note.GNU-stack","",@progbits
	.addrsig
