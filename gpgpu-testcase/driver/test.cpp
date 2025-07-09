#include "ventus.h"
#include <iostream>
#include <cstdint>
#include <cstdio>
using namespace std;

#ifndef KERNEL_ADDRESS
#define KERNEL_ADDRESS  0x800000b8
#endif

struct meta_data {
    uint64_t kernel_id;
    uint64_t kernel_size[3];
    uint64_t wf_size;
    uint64_t wg_size;
    uint64_t metaDataBaseAddr;
    uint64_t ldsSize;
    uint64_t pdsSize;
    uint64_t sgprUsage;
    uint64_t vgprUsage;
    uint64_t pdsBaseAddr;

    meta_data(uint64_t arg0, uint64_t arg1[], uint64_t arg2, uint64_t arg3, uint64_t arg4,
              uint64_t arg5, uint64_t arg6, uint64_t arg7, uint64_t arg8, uint64_t arg9)
        : kernel_id(arg0), wf_size(arg2), wg_size(arg3), metaDataBaseAddr(arg4),
          ldsSize(arg5), pdsSize(arg6), sgprUsage(arg7), vgprUsage(arg8), pdsBaseAddr(arg9) {
        kernel_size[0] = arg1[0];
        kernel_size[1] = arg1[1];
        kernel_size[2] = arg1[2];
    }
};

int main() {
    const int data_len = 16;
    uint16_t data_a[data_len] = {
        0x3C00, 0xBC00, 0x3800, 0xB800,
        0x0000, 0x7BFF, 0x0400, 0x8400,
        0x3C00, 0xC000, 0x3C00, 0x3400,
        0xB400, 0x0280, 0x8280, 0x4200
    };

    uint16_t data_b = 0x4000;  // 2.0f
    uint16_t data_c[data_len] = {0};

    uint64_t vaddr_a, vaddr_b_scalar, vaddr_c, vaddr_meta, vaddr_argbuf, vaddr_print;
    uint64_t size_print = 0x10000000;

    vt_device_h p = nullptr;
    vt_dev_open(&p);

    vt_buf_alloc(p, sizeof(data_a), &vaddr_a, 0, 0, 0);
    vt_buf_alloc(p, sizeof(uint16_t), &vaddr_b_scalar, 0, 0, 0);  // only 1 scalar
    vt_buf_alloc(p, sizeof(data_c), &vaddr_c, 0, 0, 0);
    vt_buf_alloc(p, sizeof(uint32_t) * 14, &vaddr_meta, 0, 0, 0);
    vt_buf_alloc(p, 3 * sizeof(uint32_t), &vaddr_argbuf, 0, 0, 0);
    vt_buf_alloc(p, size_print, &vaddr_print, 0, 0, 0);

    vt_copy_to_dev(p, vaddr_a, data_a, sizeof(data_a), 0, 0);
    vt_copy_to_dev(p, vaddr_b_scalar, &data_b, sizeof(uint16_t), 0, 0);

    uint64_t num_thread = data_len;
    uint64_t num_warp = 4;
    uint64_t num_workgroups[3] = {1, 1, 1};
    uint64_t pdsbase = 0x8a000000;

    meta_data meta(KERNEL_ADDRESS, num_workgroups, num_thread, num_warp,
                   vaddr_meta, 0x1000, 0x1000, 32, 32, pdsbase);

    uint32_t data_meta[14] = {0};
    data_meta[0]  = (uint32_t)meta.kernel_id;
    data_meta[1]  = (uint32_t)vaddr_argbuf;
    data_meta[2]  = (uint32_t)meta.kernel_size[0];
    data_meta[6]  = (uint32_t)meta.wf_size;
    data_meta[12] = (uint32_t)vaddr_print;
    data_meta[13] = (uint32_t)size_print;

    vt_copy_to_dev(p, vaddr_meta, data_meta, sizeof(data_meta), 0, 0);

    uint32_t arg_buf[3] = {
        (uint32_t)vaddr_a,
        (uint32_t)vaddr_b_scalar,
        (uint32_t)vaddr_c
    };
    vt_copy_to_dev(p, vaddr_argbuf, arg_buf, sizeof(arg_buf), 0, 0);

    vt_upload_kernel_file(p, "test.riscv", 0);
    vt_start(p, &meta, 0);

    vt_copy_from_dev(p, vaddr_c, data_c, sizeof(data_c), 0, 0);

    cout << "Result (vfadd.vx float16 bit patterns):" << endl;
    for (int i = 0; i < data_len; i++) {
        printf("res[%2d] = 0x%04x\n", i, data_c[i]);
    }

    vt_buf_free(p, 0, nullptr, 0, 0);
    vt_dev_close(p);
    return 0;
}
