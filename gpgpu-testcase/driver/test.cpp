#include "ventus.h"
#include <iostream>
#include <iomanip>
#include <cstdint>
using namespace std;

#ifndef KERNEL_ADDRESS
#define KERNEL_ADDRESS 0x800000b8 // 你的vadd64_vv函数所在PC地址
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
  meta_data(uint64_t id, uint64_t sz[], uint64_t wf, uint64_t wg,
            uint64_t meta, uint64_t lds, uint64_t pds,
            uint64_t sgpr, uint64_t vgpr, uint64_t pdsb)
      : kernel_id(id), wf_size(wf), wg_size(wg), metaDataBaseAddr(meta),
        ldsSize(lds), pdsSize(pds), sgprUsage(sgpr), vgprUsage(vgpr),
        pdsBaseAddr(pdsb) {
    kernel_size[0] = sz[0];
    kernel_size[1] = sz[1];
    kernel_size[2] = sz[2];
  }
};

int main() {
  uint64_t num_warp = 4;
  uint64_t num_thread = 16;
  uint64_t num_workgroups[3] = {1, 1, 1};
  uint64_t pdssize = 0x1000;
  uint64_t pdsbase = 0x8a000000;
  uint64_t knlbase = 0x90000000;

  meta_data meta(0, num_workgroups, num_thread, num_warp,
                 knlbase, 0, pdssize, 32, 32, pdsbase);

  vt_device_h p = nullptr;
  vt_dev_open(&p);

  int16_t in_a[16] = {1, 2, 3, 4, -1, -2, 32767, -32768, 10, 20, 30, -10, -20, 100, -100, 0};
  int16_t in_b[16] = {10, 20, -3, 5, 1, -2, 1, 1, -10, -20, -30, 10, 20, -100, 100, 0};
  int16_t out[16] = {};

  uint64_t vaddr_a, vaddr_b, vaddr_out, vaddr_meta, vaddr_bufbase;
  uint64_t size = 16 * sizeof(int16_t);

  vt_buf_alloc(p, size, &vaddr_a, 0, 0, 0);
  vt_buf_alloc(p, size, &vaddr_b, 0, 0, 0);
  vt_buf_alloc(p, size, &vaddr_out, 0, 0, 0);
  vt_buf_alloc(p, 64, &vaddr_meta, 0, 0, 0);
  vt_buf_alloc(p, 16, &vaddr_bufbase, 0, 0, 0);

  vt_copy_to_dev(p, vaddr_a, in_a, size, 0, 0);
  vt_copy_to_dev(p, vaddr_b, in_b, size, 0, 0);

  meta.metaDataBaseAddr = vaddr_meta;
  meta.pdsBaseAddr = pdsbase;

  uint32_t meta_buf[14] = {};
  meta_buf[0] = KERNEL_ADDRESS;
  meta_buf[1] = (uint32_t)vaddr_bufbase;
  meta_buf[2] = meta.kernel_size[0];
  meta_buf[6] = num_thread;
  meta_buf[12] = (uint32_t)vaddr_out;
  meta_buf[13] = (uint32_t)size;

  vt_copy_to_dev(p, vaddr_meta, meta_buf, sizeof(meta_buf), 0, 0);
  uint32_t buf_base[2] = {(uint32_t)vaddr_a, (uint32_t)vaddr_b};
  vt_copy_to_dev(p, vaddr_bufbase, buf_base, 8, 0, 0);

  vt_upload_kernel_file(p, (char *)"test.riscv", 0);
  vt_start(p, &meta, 0);
  cout << "Execution complete.\n";

  vt_copy_from_dev(p, vaddr_out, out, size, 0, 0);

  for (int i = 0; i < 16; i++) {
    cout << "out[" << i << "] = " << out[i] << endl;
  }

  return 0;
}
