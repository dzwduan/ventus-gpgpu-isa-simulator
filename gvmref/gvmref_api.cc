#pragma once

#include <cstdint>
#include <vector>
#include "api_structs.h"
#include "sim.h"
#include "processor.h"
#include "workgroup.h"
#include "gvmref.h"

static gvmref_t* ref = nullptr;

extern "C" {

int gvmref_vt_dev_open() {
  if (ref != nullptr) {
    delete ref;
  }
  ref = new gvmref_t();
  ref->wg.push_back(workgroup_t());
  return 0;
}
int gvmref_vt_dev_close() {
  if (ref != nullptr) {
    delete ref;
  }
  return 0;
}
int gvmref_vt_buf_alloc(uint64_t size, uint64_t *vaddr, int BUF_TYPE, uint64_t taskID, uint64_t kernelID) {
  assert(ref->wg.size() == 1);
  // 初始化阶段，只有一个 workgroup，初始化完成后会将其复制 num_workgroup 份
  if(size <= 0) return -1;
  return ref->wg[0].alloc_local_mem(size, vaddr);
}
int gvmref_vt_buf_free(uint64_t size, uint64_t *vaddr, uint64_t taskID, uint64_t kernelID) {
  assert(ref->wg.size() == 1);
  // 初始化阶段，只有一个 workgroup，初始化完成后会将其复制 num_workgroup 份
  return ref->wg[0].free_local_mem();
}
int gvmref_vt_one_buf_free(uint64_t size, uint64_t *vaddr, uint64_t taskID, uint64_t kernelID) {
  assert(ref->wg.size() == 1);
  // 初始化阶段，只有一个 workgroup，初始化完成后会将其复制 num_workgroup 份
  if(size <= 0) return -1;
  return ref->wg[0].free_local_mem(*vaddr);
}
int gvmref_vt_copy_to_dev(uint64_t dev_vaddr, const void *src_addr, uint64_t size, uint64_t taskID, uint64_t kernelID) {
  assert(ref->wg.size() == 1);
  // 初始化阶段，只有一个 workgroup，初始化完成后会将其复制 num_workgroup 份
  if(size <= 0) return -1;
  return ref->wg[0].copy_to_dev(dev_vaddr, size, src_addr);
}
int gvmref_vt_upload_kernel_file(const char* filename, int taskID) {
  ref->wg[0].set_filename(filename);
  return 0;
}
int gvmref_vt_start(void* metaData, uint64_t taskID) {
  auto knl_data = (meta_data *) metaData;
  ref->num_workgroup = (knl_data->kernel_size[0]) * (knl_data->kernel_size[1]) * (knl_data->kernel_size[2]);
  ref->num_warp = knl_data->wg_size;
  // TODO

}
  
} // extern "C"