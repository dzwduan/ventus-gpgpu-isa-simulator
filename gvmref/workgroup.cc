#include <cstdint>
#include <vector>
#include "api_structs.h"
#include "workgroup.h"

workgroup_t::workgroup_t():sim(nullptr),buffer(),buffer_data(){
  srcfilename=new char[128];
  logfilename=new char[128];
  uint64_t lds_vaddr;
  uint64_t pc_src_vaddr;
  fprintf(stderr, "spike device initialize: allocating local memory: ");
  alloc_const_mem(0x10000000,&lds_vaddr);
  fprintf(stderr, "spike device initialize: allocating pc source memory: ");
  alloc_const_mem(0x10000000, &pc_src_vaddr);
}