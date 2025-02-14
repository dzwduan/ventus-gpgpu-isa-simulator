#ifndef DIFFTEST_H_
#define DIFFTEST_H_

#include <cstdint>
#include <utility>
#include <vector>

#include "sim.h"
#include "spike_main.h"

enum { DIFFTEST_TO_DUT, DIFFTEST_TO_REF };

typedef struct {
  uint64_t gpr[256];
  uint64_t pc;
} diff_context_t;

class DifftestRef {
 public:
  DifftestRef();
  ~DifftestRef();
  void step(uint64_t n);
  void get_regs(diff_context_t *ctx);
  void set_regs(diff_context_t *ctx, bool on_demand);
  void memcpy_from_dut(reg_t vaddr, void *data, size_t size);
  void display();
  void update_dynamic_config(void *config) {
    printf("DifftestRef::update_dynamic_config() not implemented\n");
  }
  void update_uarch_status(void *status) {
    printf("DifftestRef::update_uarch_status() not implemented\n");
  }
  int store_commit(uint64_t *addr, uint64_t *data, uint8_t *mask);
  void raise_intr(uint64_t no);

  /** functions from spike_main.h **/
  int alloc_const_mem(uint64_t size, uint64_t *dev_maddr);
  int alloc_local_mem(uint64_t size, uint64_t *dev_maddr);

 private:
  sim_t *sim;
  processor_t *proc;
  state_t *state;
  std::vector<mem_cfg_t> buffer;
  std::vector<std::pair<reg_t, mem_t *>> buffer_data;
  std::vector<mem_cfg_t> const_buffer;
  std::vector<std::pair<reg_t, mem_t *>> const_buffer_data;

  void init_sim(meta_data *knl_data, uint64_t knl_start_pc);
};

#endif