	.text
	.attribute	4, 16
	.attribute	5, "rv32i2p0_m2p0_a2p0_zfinx1p0_zdinx1p0_zve32f1p0_zve32x1p0_zvl32b1p0_zhinx1p0"
	.file	"test.cl"
	.section	.ventus.resource.test_vadd64,"w",@progbits
	.half	0
	.half	1
	.half	0
	.half	0
	.text
	.globl	test_vadd64
	.p2align	2
	.type	test_vadd64,@function
test_vadd64:
	#APP
	.word	368891
	.word	401787

	#NO_APP
	#APP
	.word	3147860
	.word	262656

	#NO_APP
	#APP
	.word	3481723

	#NO_APP
	ret
.Lfunc_end0:
	.size	test_vadd64, .Lfunc_end0-test_vadd64

	.ident	"clang version 16.0.0 (https://github.com/THU-DSP-LAB/llvm-project.git fbabe9474711354ca53ba7ee71cb9a991bc85a15)"
	.section	".note.GNU-stack","",@progbits
	.addrsig
