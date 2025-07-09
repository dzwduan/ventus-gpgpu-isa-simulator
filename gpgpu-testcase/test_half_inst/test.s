        .text
        .attribute        4, 16
        .attribute        5, "rv32i2p0_m2p0_a2p0_zfinx1p0_zdinx1p0_zve32f1p0_zve32x1p0_zvl32b1p0_zhinx1p0"
        .file        "test.cl"
        .section        .ventus.resource.test,"w",@progbits
        .half        2
        .half        7
        .half        16
        .half        0
        .text
        .globl        test
        .p2align        2
        .type        test,@function
test:
        addi        sp, sp, 16
        sw        ra, -16(sp)
        lw        t0, 8(a0)
        sw        t0, -4(sp)
        lw        t0, 0(a0)
        sw        t0, -8(sp)
        lw        t0, 4(a0)
        sw        t0, -12(sp)
        vmv.v.x        v0, zero
        call        _Z13get_global_idj
        lw        t0, -12(sp)
        lh        t0, 0(t0)
        vsll.vi        v0, v0, 1
        lw        t1, -8(sp)
        vadd.vx        v1, v0, t1
        vlh12.v        v1, 0(v1)
        .align 3
        .dword 0x90020a0000102854
        lw        t0, -4(sp)
        vadd.vx        v0, v0, t0
        vsh12.v        v1, 0(v0)
        lw        ra, -16(sp)
        addi        sp, sp, -16
        ret
.Lfunc_end0:
        .size        test, .Lfunc_end0-test

        .ident        "clang version 16.0.0 (https://github.com/THU-DSP-LAB/llvm-project.git fbabe9474711354ca53ba7ee71cb9a991bc85a15)"
        .section        ".note.GNU-stack","",@progbits
        .addrsig
