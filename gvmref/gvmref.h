#pragma once

#include <cstdint>
#include <vector>
#include "api_structs.h"
#include "sim.h"
#include "processor.h"
#include "workgroup.h"

class gvmref_t {
public:
  gvmref_t();
  ~gvmref_t();
  std::vector<workgroup_t> wg;
  uint32_t num_workgroup; // 工作组数目
  uint32_t num_warp;      // 每个工作组的warp数目

};