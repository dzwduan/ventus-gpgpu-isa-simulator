#pragma once

#include <cstdint>
#include <vector>
#include "api_structs.h"
#include "sim.h"
#include "processor.h"
#include "cfg.h"


class workgroup_t {
public:
  workgroup_t();
  ~workgroup_t();

  // 以下是来自 spike_device 类的函数
  int alloc_const_mem(uint64_t size, uint64_t* dev_maddr);
  int alloc_local_mem(uint64_t size, uint64_t* dev_maddr);
  int free_local_mem();
  int free_local_mem(uint64_t paddr);
  int copy_to_dev(uint64_t dev_maddr, uint64_t size, const void* data);
  int set_filename(const char* filename, const char* logname = nullptr);

  // 以下是 GVM API 所需的函数
  gvm_ref_step_return_info_t step(uint32_t warp_id); // 步进 1 条指令
  void set_warp_xreg(uint32_t wg_id, uint32_t warp_id, uint32_t xreg_usage, gvm_ref_warp_xreg_t xreg);
  int done() { return sim->done(); } // 本 workgroup 已运行到结尾
  void init_sim(meta_data* knl_data, uint64_t knl_start_pc, uint64_t currwgid); // spike_device::run 的初始化部分

  // kernel 尺寸
  uint32_t num_workgroup; // 工作组数目
  uint32_t num_warp;      // 每个工作组的warp数目
  uint64_t currwgid;      // 本 DifftestRef 对象对应的 workgroup_id

private:
  sim_t* sim;
  std::vector<state_t*> state; // 寄存器文件
  std::vector<processor_t*> proc;

  std::vector<mem_cfg_t> buffer; // 可以在分配时让buffer地址对齐4k
  std::vector<std::pair<reg_t, mem_t*>> buffer_data;
  std::vector<mem_cfg_t> const_buffer;                     // 在构造函数中分配
  std::vector<std::pair<reg_t, mem_t*>> const_buffer_data; // 在构造函数中分配
  char* srcfilename;
  char* logfilename;

  cfg_t* cfg;
};