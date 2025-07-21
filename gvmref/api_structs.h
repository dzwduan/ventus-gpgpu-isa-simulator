#pragma once
#include <vector>
#include <cstdint>

enum { DIFFTEST_FROM_REF, DIFFTEST_TO_REF };

struct gvm_ref_warp_xreg_t {
  std::vector<uint64_t> xreg;
}; // 用于 set xreg API

struct gvm_ref_pc_t {
  // 最外层 vector 为 workgroup
  std::vector<std::vector<uint64_t>> pc;
};

struct gvm_ref_xreg_t {
  // 最外层 vector 为 workgroup
  std::vector<std::vector<std::array<uint64_t, 256>>> xpr; // const int NXPR = 256;
};

struct gvm_ref_step_return_info_t {
  // 每次 step 时，返回 step 的结果
  // 包括指令类型、写回寄存器地址、写回寄存器数据、mask 等
  int wg_done; // 该 workgroup 已完成？
};

struct meta_data{  // 这个metadata是供驱动使用的，而不是给硬件的
    uint64_t kernel_id;
    uint64_t kernel_size[3];///> 每个kernel的workgroup三维数目
    uint64_t wf_size; ///> 每个warp的thread数目
    uint64_t wg_size; ///> 每个workgroup的warp数目
    uint64_t metaDataBaseAddr;///> CSR_KNL的值，
    uint64_t ldsSize;///> 每个workgroup使用的local memory的大小
    uint64_t pdsSize;///> 每个thread用到的private memory大小
    uint64_t sgprUsage;///> 每个workgroup使用的标量寄存器数目
    uint64_t vgprUsage;///> 每个thread使用的向量寄存器数目
    uint64_t pdsBaseAddr;///> private memory的基址，要转成每个workgroup的基地址， wf_size*wg_size*pdsSize
    meta_data(uint64_t arg0,uint64_t arg1[],uint64_t arg2,uint64_t arg3,uint64_t arg4,uint64_t arg5,\
      uint64_t arg6,uint64_t arg7,uint64_t arg8,uint64_t arg9) \
      :kernel_id(arg0),wf_size(arg2),wg_size(arg3),metaDataBaseAddr(arg4),ldsSize(arg5),pdsSize(arg6),\
      sgprUsage(arg7),vgprUsage(arg8),pdsBaseAddr(arg9)
      {
        kernel_size[0]=arg1[0];kernel_size[1]=arg1[1];kernel_size[2]=arg1[2];
      }
};