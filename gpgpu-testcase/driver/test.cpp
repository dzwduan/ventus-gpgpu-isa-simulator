#include "ventus.h"
#include <iostream>
#include <cstdint>
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
    uint64_t num_thread = 16;
    uint64_t num_warp = 4;
    uint64_t num_workgroups[3] = {1, 1, 1};
    uint64_t num_workgroup = num_workgroups[0] * num_workgroups[1] * num_workgroups[2];
    uint64_t ldssize = 0x1000;
    uint64_t pdssize = 0x1000;
    uint64_t pdsbase = 0x8a000000;
    uint64_t knlbase = 0x90000000;

    meta_data meta(0, num_workgroups, num_thread, num_warp, knlbase, ldssize, pdssize, 32, 32, pdsbase);

    char filename[] = "test.riscv";

    const int data_len = 16;
    int16_t data_a[data_len] = {1,2,3,4,5,6,7,8,-8,-7,-6,-5,-4,-3,-2,-1};
    int16_t data_b[data_len] = {1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1};
    int16_t data_c[data_len] = {0};

    uint64_t vaddr_a, vaddr_b, vaddr_c, vaddr_meta, vaddr_argbuf, vaddr_print;
    uint64_t size_print = 0x10000000;

    vt_device_h p = nullptr;
    vt_dev_open(&p);
    
    vt_buf_alloc(p, sizeof(int16_t) * data_len, &vaddr_a, 0, 0, 0);
    vt_buf_alloc(p, sizeof(int16_t) * data_len, &vaddr_b, 0, 0, 0);
    vt_buf_alloc(p, sizeof(int16_t) * data_len, &vaddr_c, 0, 0, 0);

    vt_buf_alloc(p, pdssize * num_thread * num_warp * num_workgroup, &pdsbase, 0, 0, 0);
    vt_buf_alloc(p, sizeof(uint32_t) * 14, &vaddr_meta, 0, 0, 0);
    vt_buf_alloc(p, 3 * 4, &vaddr_argbuf, 0, 0, 0);
    vt_buf_alloc(p, size_print, &vaddr_print, 0, 0, 0);

    vt_copy_to_dev(p, vaddr_a, data_a, sizeof(int16_t) * data_len, 0, 0);
    vt_copy_to_dev(p, vaddr_b, data_b, sizeof(int16_t) * data_len, 0, 0);

    meta.kernel_id = KERNEL_ADDRESS;
    meta.metaDataBaseAddr = vaddr_meta;
    meta.pdsBaseAddr = pdsbase;

    uint32_t data_meta[14] = {0};
    data_meta[0] = (uint32_t)meta.kernel_id;
    data_meta[1] = (uint32_t)vaddr_argbuf;
    data_meta[2] = meta.kernel_size[0];
    data_meta[6] = num_thread;
    data_meta[12] = (uint32_t)vaddr_print;
    data_meta[13] = (uint32_t)size_print;

    vt_copy_to_dev(p, vaddr_meta, data_meta, sizeof(data_meta), 0, 0);

    uint32_t arg_buf[3] = {
        (uint32_t)vaddr_a,
        (uint32_t)vaddr_b,
        (uint32_t)vaddr_c
    };
    vt_copy_to_dev(p, vaddr_argbuf, arg_buf, sizeof(arg_buf), 0, 0);

    vt_upload_kernel_file(p, filename, 0);
    vt_start(p, &meta, 0);

    cout << "finish running" << endl;

    vt_copy_from_dev(p, vaddr_c, data_c, sizeof(int16_t) * data_len, 0, 0);

    cout << "Result:" << endl;
    cout << dec; 
    for (int i = 0; i < data_len; i++) {
        cout << data_c[i] << " ";
    }
    cout << endl;

    vt_buf_free(p, 0, nullptr, 0, 0);
    vt_dev_close(p);
    return 0;
}
