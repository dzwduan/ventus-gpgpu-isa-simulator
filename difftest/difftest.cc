#include "difftest.h"

#include <cassert>
#include <cstdio>
#include <cstring>

#define VBASEADDR 0x70000000    // default vaddr for LDS
#define ARGBASEADDR 0x90000000  // default vaddr for kernel arg buffer

static void help(int exit_code = 1) {
  fprintf(stderr, "now you are in help function");
  exit(exit_code);
}

bool sort_mem_region(const mem_cfg_t &a, const mem_cfg_t &b) {
  if (a.base == b.base)
    return (a.size < b.size);
  else
    return (a.base < b.base);
}

void merge_overlapping_memory_regions(std::vector<mem_cfg_t> &mems) {
  // check the user specified memory regions and merge the overlapping or
  // eliminate the containing parts
  assert(!mems.empty());

  std::sort(mems.begin(), mems.end(), sort_mem_region);
  for (auto it = mems.begin() + 1; it != mems.end();) {
    reg_t start = prev(it)->base;
    reg_t end = prev(it)->base + prev(it)->size;
    reg_t start2 = it->base;
    reg_t end2 = it->base + it->size;

    // contains -> remove
    if (start2 >= start && end2 <= end) {
      it = mems.erase(it);
      // partial overlapped -> extend
    } else if (start2 >= start && start2 < end) {
      prev(it)->size = std::max(end, end2) - start;
      it = mems.erase(it);
      // no overlapping -> keep it
    } else {
      it++;
    }
  }
}

static std::vector<mem_cfg_t> parse_mem_layout(const char *arg) {
  std::vector<mem_cfg_t> res;

  // handle legacy mem argument
  char *p;
  auto mb = strtoull(arg, &p, 0);
  if (*p == 0) {
    reg_t size = reg_t(mb) << 20;
    if (size != (size_t)size)
      throw std::runtime_error("Size would overflow size_t");
    res.push_back(mem_cfg_t(reg_t(DRAM_BASE), size));
    return res;
  }

  // handle base/size tuples
  while (true) {
    auto base = strtoull(arg, &p, 0);
    if (!*p || *p != ':') printf("command line input fromat wrong\n");
    auto size = strtoull(p + 1, &p, 0);

    // page-align base and size
    auto base0 = base, size0 = size;
    size += base0 % PGSIZE;
    base -= base0 % PGSIZE;
    if (size % PGSIZE != 0) size += PGSIZE - size % PGSIZE;

    if (base + size < base) printf("page size alignmentation failed\n");

    if (size != size0) {
      fprintf(stderr,
              "Warning: the memory at  [0x%llX, 0x%llX] has been realigned\n"
              "to the %ld KiB page size: [0x%llX, 0x%llX]\n",
              base0, base0 + size0 - 1, long(PGSIZE / 1024), base,
              base + size - 1);
    }

    res.push_back(mem_cfg_t(reg_t(base), reg_t(size)));
    if (!*p) break;
    if (*p != ',') help();
    arg = p + 1;
  }

  merge_overlapping_memory_regions(res);

  return res;
}

static DifftestRef *ref = nullptr;

DifftestRef::DifftestRef() : sim(nullptr), proc(nullptr), state(nullptr) {
  uint64_t lds_vaddr;
  uint64_t pc_src_vaddr;
  printf("DifftestRef initialize: allocating local memory: ");
  alloc_const_mem(0x10000000, &lds_vaddr);
  printf("DifftestRef initialize: allocating pc source memory: ");
  alloc_const_mem(0x10000000, &pc_src_vaddr);

  // Initialize the simulator
  init_sim();
}

DifftestRef::~DifftestRef() {
  for (const auto &pair : buffer_data) {
    delete pair.second;
  }
  for (const auto &pair : const_buffer_data) {
    delete pair.second;
  }

  delete sim;
}

int DifftestRef::alloc_const_mem(uint64_t size, uint64_t *vaddr) {
  uint64_t base;
#define PGSHIFT 12
  const reg_t PGSIZE = 1 << PGSHIFT;
  if (size <= 0 || vaddr == nullptr) return -1;
  if (const_buffer.empty()) {
    base = VBASEADDR;
  } else {
    base = const_buffer.back().base + const_buffer.back().size;
  }

  auto base0 = base, size0 = size;
  size += base0 % PGSIZE;
  base -= base0 % PGSIZE;
  if (size % PGSIZE != 0) size += PGSIZE - size % PGSIZE;

  if (base + size < base) {
    fprintf(stderr, "DifftestRef::alloc_const_mem(): base + size < base\n");
    exit(1);
  }

  if (size != size0) {
    fprintf(stderr,
            "Warning: the memory at  [0x%lX, 0x%lX] has been realigned\n"
            "to the %ld KiB page size: [0x%lX, 0x%lX]\n",
            base0, base0 + size0 - 1, long(PGSIZE / 1024), base,
            base + size - 1);
  }

  printf("Allocating const mem at 0x%lx with %ld bytes\n", base, size);

  const_buffer.push_back(mem_cfg_t(reg_t(base), reg_t(size)));
  const_buffer_data.push_back(std::make_pair(base, new mem_t(size)));

  *vaddr = base;
  return 0;
}

void DifftestRef::memcpy_from_dut(reg_t vaddr, void *data, size_t size) {
  bool found = false;
  printf("DifftestRef::memcpy_from_dut(): copying to 0x%lx with %ld bytes\n",
         vaddr, size);

  // Search in buffer_data
  for (size_t i = 0; i < buffer.size(); ++i) {
    if (vaddr >= buffer[i].base && vaddr < buffer[i].base + buffer[i].size) {
      if (vaddr + size > buffer[i].base + buffer[i].size) {
        fprintf(stderr, "Cannot copy to 0x%lx with size %ld\n", vaddr, size);
        return;
      }
      buffer_data[i].second->store(vaddr - buffer_data[i].first, size,
                                   (const uint8_t *)data);
      found = true;
      printf("found data in buffer_data[%ld]\n", i);
      break;
    }
  }

  // Search in const_buffer_data if not found
  if (!found) {
    for (size_t i = 0; i < const_buffer.size(); ++i) {
      if (vaddr >= const_buffer[i].base &&
          vaddr < const_buffer[i].base + const_buffer[i].size) {
        if (vaddr + size > const_buffer[i].base + const_buffer[i].size) {
          fprintf(stderr, "Cannot copy to 0x%lx with size %ld\n", vaddr, size);
          return;
        }
        const_buffer_data[i].second->store(vaddr - const_buffer_data[i].first,
                                           size, (const uint8_t *)data);
        found = true;
        printf("found data in const_buffer_data[%ld]\n", i);
        break;
      }
    }
  }

  if (!found) {
    fprintf(stderr, "vaddr 0x%lx does not fit any allocated buffer\n", vaddr);
  }
}

void DifftestRef::init_sim(meta_data *knl_data, uint64_t knl_start_pc) {
  std::vector<std::pair<reg_t, mem_t *>> mems = const_buffer_data;
  mems.insert(mems.end(), buffer_data.begin(), buffer_data.end());

  // Create default configuration
  cfg_t *cfg = new cfg_t(std::make_pair((reg_t)0, (reg_t)0),  // initrd_bounds
                         nullptr,                             // bootargs
                         "RV32GCV",                           // ISA
                         DEFAULT_PRIV,                        // priv
                         DEFAULT_VARCH,                       // varch
                         std::vector<mem_cfg_t>(),            // mem_layout
                         std::vector<int>(),                  // hartids
                         false                                // real_time_clint
  );

  // Create simulator instance
  sim = new sim_t(
      cfg,
      false,                                                 // halted
      mems,                                                  // mems
      std::vector<std::pair<reg_t, abstract_device_t *>>(),  // plugin_devices
      std::vector<std::string>(),                            // htif_args
      debug_module_config_t(),                               // dm_config
      nullptr,                                               // log_path
      false,                                                 // dtb_enabled
      nullptr                                                // dtb_file
  );

  // Get the first processor
  proc = sim->get_core(0);
  state = proc->get_state();
}

void DifftestRef::step(uint64_t n) { sim->step(n); }

void DifftestRef::display() {
  printf("DifftestRef::display is not implemented yet\n");
}

int DifftestRef::store_commit(uint64_t *addr, uint64_t *data, uint8_t *mask) {
  return sim->dut_store_commit(addr, data, mask);
}

void DifftestRef::raise_intr(uint64_t no) {
  printf("DifftestRef::raise_intr (触发中断) is not implemented yet\n");
}

/******************************************************************************/
/*********************************difftest api*********************************/
/******************************************************************************/

extern "C" {

void difftest_init() { ref = new DifftestRef(); }

void difftest_regcpy(diff_context_t *dut, bool direction, bool on_demand) {
  if (direction == DIFFTEST_TO_REF) {
    ref->set_regs(dut, on_demand);
  } else {
    ref->get_regs(dut);
  }
}

void difftest_csrcpy(void *dut, bool direction) {}

void difftest_memcpy(uint64_t addr, void *buf, size_t n, bool direction) {
  if (direction == DIFFTEST_TO_REF) {
    ref->memcpy_from_dut(addr, buf, n);
  } else {
    printf("difftest_memcpy with DIFFTEST_TO_DUT is not supported yet\n");
    fflush(stdout);
    assert(0);
  }
}

void difftest_exec(uint64_t n) { ref->step(n); }

void difftest_display() { ref->display(); }

void update_dynamic_config(void *config) { ref->update_dynamic_config(config); }

void difftest_uarchstatus_sync(void *dut) { ref->update_uarch_status(dut); }

int difftest_store_commit(uint64_t *addr, uint64_t *data, uint8_t *mask) {
  return ref->store_commit(addr, data, mask);
}

void difftest_raise_intr(uint64_t NO) { ref->raise_intr(NO); }

void difftest_load_flash(void *flash_bin, size_t size) {}
}
