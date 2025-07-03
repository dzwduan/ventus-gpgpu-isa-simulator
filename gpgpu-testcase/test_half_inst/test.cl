__kernel void test_vadd64(__global half* out,
                          __global half* a,
                          __global half* b) {
    int tid = get_global_id(0);

    // 假设 ABI 下，内核前三个指针参数放在 a0=out, a1=a, a2=b

    __asm__ __volatile__ (
        ".word 0x0005a0fb  \n\t"   // vlw12.v v1, (a1)
        ".word 0x0006217b  \n\t"   // vlw12.v v2, (a2)
        :::
    );

    {
      unsigned long inst = 0;
      inst |= (0x00UL << 58);
      inst |= (0    << 57);
      inst |= (2UL  << 49);    // vs2=v2
      inst |= (1UL  << 41);    // vs1=v1
      inst |= (0    << 39);
      inst |= (0    << 36);
      inst |= (0    << 28);
      inst |= (3UL  << 20);    // vd=v3
      inst |= (0    << 17);
      inst |= (0    << 14);
      inst |= (1UL  << 11);    // type=float
      inst |= (0UL  << 9);     // srcw=16
      inst |= (0UL  << 7);     // dstw=16
      inst |= 0x54UL;          // opcode

      unsigned inst_lo = (unsigned)(inst & 0xFFFFFFFFUL);
      unsigned inst_hi = (unsigned)(inst >> 32);

      __asm__ __volatile__ (
          ".word %0\n\t"
          ".word %1\n\t"
          : : "i"(inst_lo), "i"(inst_hi)
      );
    }

    __asm__ __volatile__ (
        ".word 0x0035207b  \n\t"   // vsw12.v v3, (a0)
        :::
    );
}
