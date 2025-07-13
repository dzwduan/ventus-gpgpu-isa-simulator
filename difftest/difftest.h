#ifndef VT_REF_DIFFTEST_H_
#define VT_REF_DIFFTEST_H_

#include <queue>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "../VERSION"
#include "cachesim.h"
#include "cfg.h"
#include "extension.h"
#include "mmu.h"
#include "remote_bitbang.h"
#include "sim.h"
#include <dlfcn.h>
#include <fesvr/option_parser.h>
#include <fstream>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include <sys/types.h>
#include <utility>

struct meta_data_t { // 这个metadata是供驱动使用的，而不是给硬件的
  uint64_t startaddr;
  uint64_t kernel_id;
  uint64_t kernel_size[3];   ///> 每个kernel的workgroup三维数目
  uint64_t wf_size;          ///> 每个warp的thread数目
  uint64_t wg_size;          ///> 每个workgroup的warp数目
  uint64_t metaDataBaseAddr; ///> CSR_KNL的值，
  uint64_t ldsSize;          ///> 每个workgroup使用的local memory的大小
  uint64_t pdsSize;          ///> 每个thread用到的private memory大小
  uint64_t sgprUsage;        ///> 每个workgroup使用的标量寄存器数目
  uint64_t vgprUsage;        ///> 每个thread使用的向量寄存器数目
  uint64_t
      pdsBaseAddr; ///> private memory的基址，要转成每个workgroup的基地址， wf_size*wg_size*pdsSize
  uint64_t num_buffer;   ///> buffer的数目，包括pc
  uint64_t* buffer_base; ///> 各buffer的基址。第一块buffer是给硬件用的metadata
  uint64_t* buffer_size; ///> 各buffer的size，以Bytes为单位。实际使用的大小，用于初始化.data
  uint64_t* buffer_allocsize; ///> 各buffer的size，以Bytes为单位。分配的大小
  int insBufferIndex;         // 指令在哪一个buffer

  meta_data_t() {
    startaddr = 0;
    kernel_id = 0;
    kernel_size[0] = 0;
    kernel_size[1] = 0;
    kernel_size[2] = 0;
    wf_size = 0;
    wg_size = 0;
    metaDataBaseAddr = 0;
    ldsSize = 0;
    pdsSize = 0;
    sgprUsage = 0;
    vgprUsage = 0;
    pdsBaseAddr = 0;
    num_buffer = 0;
    buffer_base = nullptr;
    buffer_size = nullptr;
    buffer_allocsize = nullptr;
    insBufferIndex = 0;
  }; // 无参构造函数
};

enum { DIFFTEST_FROM_REF, DIFFTEST_TO_REF };
#define FMT_WORD "0x%016lx"

typedef struct {
  // 外层 vector 为 workgroup
  std::vector<std::vector<std::array<uint64_t, 256>>> xpr; // const int NXPR = 256;
  std::vector<std::vector<uint64_t>> pc;
} diff_ref_context_t; // 寄存器状态结构体

typedef struct {
  std::vector<uint64_t> xreg;
} diff_ref_warp_xreg_vec_t; // 用于 set xreg API

typedef struct {
  uint32_t wg_id;
  uint32_t warp_id;
  uint32_t num_step;
} step_info_t;

class DifftestRef {
public:
  DifftestRef();  // 初始化
  ~DifftestRef(); // 析构

  // 以下是来自 spike_device 类的函数
  int alloc_const_mem(uint64_t size, uint64_t* dev_maddr);
  int alloc_local_mem(uint64_t size, uint64_t* dev_maddr);
  int free_local_mem();
  int free_local_mem(uint64_t paddr);
  int copy_to_dev(uint64_t dev_maddr, uint64_t size, const void* data);
  int copy_from_dev(uint64_t dev_maddr, uint64_t size, void* data);
  // int run(meta_data_t* knl_data, uint64_t knl_start_pc);
  int set_filename(const char* filename, const char* logname = nullptr);

  // 以下是 difftest API 所需的函数
  step_info_t step_info;
  void step();                                 // 步进
  void get_regs(diff_ref_context_t* ctx);                    // 将spike的寄存器状态拷贝到ctx
  void set_regs(diff_ref_context_t* ctx, bool on_demand);    // 将ctx的寄存器状态拷贝到spike
  void set_warp_xreg(uint32_t wg_id, uint32_t warp_id, uint32_t xreg_usage, diff_ref_warp_xreg_vec_t xreg);
  // void memcpy_from_dut(reg_t dest, void* src, size_t n); // 将内存写入spike
  void display(int wg_id);                                        // 打印寄存器内容
  int done(uint32_t wg_id) { return sim[wg_id]->done(); }   // REF 侧已运行到结尾
  void update_dynamic_config(void* config) {
    printf("DifftestRef::update_dynamic_config() not implemented\n");
  } // 香山的代码中只是将 config->debug_difftest 赋值给了 sim->enable_difftest_logs
  void update_uarch_status(void* status) {
    printf("DifftestRef::update_uarch_status() not implemented\n");
  } // 将status选择性拷贝到spike的sim_t中
  // int store_commit(uint64_t* addr, uint64_t* data, uint8_t* mask) {
  //   return sim->dut_store_commit(addr, data, mask);
  // } // 追踪 dut 的内存写入

  // 以下用于 DifftestRef 构造函数
  void init_sim(uint64_t knl_start_pc, uint64_t currwgid);

  // 以下用来初始化 .metadata
  void initMetaData(const std::string& filename);

  // 以下用来初始化 .data
  void initData(const std::string& filename);
  
  // kernel 尺寸
  uint32_t num_workgroup; // 工作组数目
  uint32_t num_warp;      // 每个工作组的warp数目
  uint64_t currwgid;      // 本 DifftestRef 对象对应的 workgroup_id

private:
  // 外层 vector 为 workgroup
  std::vector<sim_t*> sim;
  std::vector<std::vector<state_t*>> state; // 寄存器文件
  std::vector<std::vector<processor_t*>> proc;

  std::vector<mem_cfg_t> buffer; // 可以在分配时让buffer地址对齐4k
  std::vector<std::pair<reg_t, mem_t*>> buffer_data;
  std::vector<mem_cfg_t> const_buffer;                     // 在构造函数中分配
  std::vector<std::pair<reg_t, mem_t*>> const_buffer_data; // 在构造函数中分配
  char* srcfilename;
  char* logfilename;

  // 以下用于 DifftestRef 构造函数
  cfg_t* cfg;

  // 以下用来初始化 .metadata
  meta_data_t m_metadata;
  void assignMetadata(const std::vector<uint64_t>& metadata, meta_data_t& mtd);
  bool isHexCharacter(char c);
  int charToHex(char c);
  void readHexFile(const std::string& filename, int itemSize, std::vector<uint64_t>& items);

  // 以下用来初始化 .data
  std::vector<std::vector<uint8_t>>* m_buffer_data;
  void readTextFile(const std::string& filename, std::vector<std::vector<uint8_t>>& buffers,
                    meta_data_t mtd);
  void init_local_and_private_mem(std::vector<std::vector<uint8_t>>& buffers, meta_data_t mtd);
};

class DifftestMultiWgRef{
public:
  std::vector<DifftestRef*> wg;
  uint32_t num_workgroup;
  // 以下用来初始化 .metadata
  void initMetaData(const std::string& filename);
private:
  // 以下用来初始化 .metadata
  meta_data_t m_metadata;
  void assignMetadata(const std::vector<uint64_t>& metadata, meta_data_t& mtd);
  bool isHexCharacter(char c);
  int charToHex(char c);
  void readHexFile(const std::string& filename, int itemSize, std::vector<uint64_t>& items);
};

static std::vector<std::pair<reg_t, mem_t*>> make_mems(const std::vector<mem_cfg_t>& layout) {
  std::vector<std::pair<reg_t, mem_t*>> mems;
  mems.reserve(layout.size());
  for (const auto& cfg : layout) {
    mems.push_back(std::make_pair(cfg.base, new mem_t(cfg.size)));
  }
  return mems;
}

#endif // VT_REF_DIFFTEST_H_