#include "difftest.h"
#include "disasm.h"
#include "mmu.h"
#include "sim.h"

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <sys/types.h>

#define VBASEADDR 0x70000000   // default vaddr for LDS
#define ARGBASEADDR 0x90000000 // default vaddr for kernel arg buffer
#define SPIKE_RUN_WG_NUM 1

/********* functions for initialize metadata ************/

void DifftestMultiWgRef::assignMetadata(const std::vector<uint64_t>& metadata, meta_data_t& mtd) {
  int index = 0;

  mtd.startaddr = metadata[index++];

  mtd.kernel_id = metadata[index++];

  for (int i = 0; i < 3; i++) {
    mtd.kernel_size[i] = metadata[index++];
  }

  mtd.wf_size = metadata[index++];
  mtd.wg_size = metadata[index++];
  mtd.metaDataBaseAddr = metadata[index++];
  mtd.ldsSize = metadata[index++];
  mtd.pdsSize = metadata[index++];
  mtd.sgprUsage = metadata[index++];
  mtd.vgprUsage = metadata[index++];
  mtd.pdsBaseAddr = metadata[index++];

  mtd.num_buffer = metadata[index++] + 1; // add localmem buffer

  mtd.buffer_base = new uint64_t[mtd.num_buffer];

  for (int i = 0; i < mtd.num_buffer - 1; i++) {
    mtd.buffer_base[i] = metadata[index++];
    if (mtd.buffer_base[i] == mtd.startaddr)
      mtd.insBufferIndex = i;
  }
  mtd.buffer_base[mtd.num_buffer - 1] = 0x70000000;
  // ldsBaseAddr_core: localmem base addr

  mtd.buffer_size = new uint64_t[mtd.num_buffer];
  for (int i = 0; i < mtd.num_buffer - 1; i++) {
    mtd.buffer_size[i] = metadata[index++];
  }
  mtd.buffer_size[mtd.num_buffer - 1] = 0;

  mtd.buffer_allocsize = new uint64_t[mtd.num_buffer];
  for (int i = 0; i < mtd.num_buffer - 1; i++) {
    mtd.buffer_allocsize[i] = metadata[index++];
  }
  mtd.buffer_allocsize[mtd.num_buffer - 1] = mtd.ldsSize;
}

bool DifftestMultiWgRef::isHexCharacter(char c) {
  return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

int DifftestMultiWgRef::charToHex(char c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  else if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  else if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  else
    return -1; // Invalid character
}

void DifftestMultiWgRef::readHexFile(
    const std::string& filename, int itemSize,
    std::vector<uint64_t>& items) { // itemSize为每个数据的比特数，这里为64
  std::ifstream file(filename);

  if (!file) {
    std::cout << "Error opening file: " << filename << std::endl;
    return;
  }

  char c;
  int bits = 0;
  uint64_t value = 0;
  bool leftside = false;

  while (file.get(c)) {
    if (c == '\n') {
      if (bits != 0)
        leftside = true;
      continue;
    }

    if (!isHexCharacter(c)) {
      std::cout << "Invalid character found: " << c << " in " << filename << std::endl;
      continue;
    }

    int hexValue = charToHex(c);
    if (leftside)
      value = value | ((uint64_t)hexValue << (92 - bits));
    else
      value = (value << 4) | hexValue;
    bits += 4;

    if (bits >= itemSize) {
      items.push_back(value);
      value = 0;
      bits = 0;
      leftside = false;
    }
  }

  if (bits > 0) {
    std::cout << "Warning: Incomplete item found at the end of the file!" << std::endl;
  }

  file.close();
}

void DifftestMultiWgRef::initMetaData(const std::string& filename) {
  std::vector<uint64_t> metadata;
  readHexFile(filename, 64, metadata);
  assignMetadata(metadata, m_metadata);
  uint64_t num_workgroup_x = m_metadata.kernel_size[0];
  uint64_t num_workgroup_y = m_metadata.kernel_size[1];
  uint64_t num_workgroup_z = m_metadata.kernel_size[2];
  num_workgroup = num_workgroup_x * num_workgroup_y * num_workgroup_z;
}

/********* functions for initialize metadata ************/

void DifftestRef::assignMetadata(const std::vector<uint64_t>& metadata, meta_data_t& mtd) {
  int index = 0;
  assert(metadata.size() > 0);
  mtd.startaddr = metadata[index++];

  mtd.kernel_id = metadata[index++];

  for (int i = 0; i < 3; i++) {
    mtd.kernel_size[i] = metadata[index++];
  }

  mtd.wf_size = metadata[index++];
  mtd.wg_size = metadata[index++];
  mtd.metaDataBaseAddr = metadata[index++];
  mtd.ldsSize = metadata[index++];
  mtd.pdsSize = metadata[index++];
  mtd.sgprUsage = metadata[index++];
  mtd.vgprUsage = metadata[index++];
  mtd.pdsBaseAddr = metadata[index++];

  mtd.num_buffer = metadata[index++] + 1; // add localmem buffer

  mtd.buffer_base = new uint64_t[mtd.num_buffer];

  for (int i = 0; i < mtd.num_buffer - 1; i++) {
    mtd.buffer_base[i] = metadata[index++];
    if (mtd.buffer_base[i] == mtd.startaddr)
      mtd.insBufferIndex = i;
  }
  mtd.buffer_base[mtd.num_buffer - 1] = 0x70000000;
  // ldsBaseAddr_core: localmem base addr

  mtd.buffer_size = new uint64_t[mtd.num_buffer];
  for (int i = 0; i < mtd.num_buffer - 1; i++) {
    mtd.buffer_size[i] = metadata[index++];
  }
  mtd.buffer_size[mtd.num_buffer - 1] = 0;

  mtd.buffer_allocsize = new uint64_t[mtd.num_buffer];
  for (int i = 0; i < mtd.num_buffer - 1; i++) {
    mtd.buffer_allocsize[i] = metadata[index++];
  }
  mtd.buffer_allocsize[mtd.num_buffer - 1] = mtd.ldsSize;
}

bool DifftestRef::isHexCharacter(char c) {
  return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

int DifftestRef::charToHex(char c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  else if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  else if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  else
    return -1; // Invalid character
}

void DifftestRef::readHexFile(
    const std::string& filename, int itemSize,
    std::vector<uint64_t>& items) { // itemSize为每个数据的比特数，这里为64
  std::ifstream file(filename);

  if (!file) {
    std::cout << "Error opening file: " << filename << std::endl;
    return;
  }

  char c;
  int bits = 0;
  uint64_t value = 0;
  bool leftside = false;

  while (file.get(c)) {
    if (c == '\n') {
      if (bits != 0)
        leftside = true;
      continue;
    }

    if (!isHexCharacter(c)) {
      std::cout << "Invalid character found: " << c << " in " << filename << std::endl;
      continue;
    }

    int hexValue = charToHex(c);
    if (leftside)
      value = value | ((uint64_t)hexValue << (92 - bits));
    else
      value = (value << 4) | hexValue;
    bits += 4;

    if (bits >= itemSize) {
      items.push_back(value);
      value = 0;
      bits = 0;
      leftside = false;
    }
  }

  if (bits > 0) {
    std::cout << "Warning: Incomplete item found at the end of the file!" << std::endl;
  }

  file.close();
}

void DifftestRef::initMetaData(const std::string& filename) {
  std::vector<uint64_t> metadata;
  readHexFile(filename, 64, metadata);
  assignMetadata(metadata, m_metadata);
}

/********** functions for initialize data **************/

void DifftestRef::readTextFile(const std::string& filename,
                               std::vector<std::vector<uint8_t>>& buffers, meta_data_t mtd) {

  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Failed to open file: " << filename << std::endl;
    return;
  }

  std::string line;
  int bufferIndex = 0;
  std::vector<uint8_t> buffer;
  for (int bufferIndex = 0; bufferIndex < mtd.num_buffer; bufferIndex++) {
    buffer.reserve(mtd.buffer_allocsize[bufferIndex]); // 提前分配空间
    int readbytes = 0;
    while (readbytes < mtd.buffer_size[bufferIndex]) {
      std::getline(file, line);
      for (int i = line.length(); i > 0; i -= 2) {
        std::string hexChars = line.substr(i - 2, 2);
        uint8_t byte = std::stoi(hexChars, nullptr, 16);
        buffer.push_back(byte);
      }
      readbytes += 4;
    }
    buffer.resize(mtd.buffer_allocsize[bufferIndex]);
    buffers[bufferIndex] = buffer;
    buffer.clear();
  }

  file.close();
}

void DifftestRef::init_local_and_private_mem(std::vector<std::vector<uint8_t>>& buffers,
                                             meta_data_t mtd) {
  uint64_t ldsSize = mtd.ldsSize;
  uint64_t pdsSize = mtd.pdsSize;
  buffers[mtd.num_buffer - 1].resize(ldsSize);
}

void DifftestRef::initData(const std::string& filename) {

  m_buffer_data = new std::vector<std::vector<uint8_t>>(m_metadata.num_buffer);
  // 此时num_buffer已经是.meta文件里的num_buffer+1，包含了末尾的local buffer
  readTextFile(filename.c_str(), *m_buffer_data, m_metadata);
  init_local_and_private_mem(*m_buffer_data, m_metadata);

  // 接下来将 m_buffer_data 中的数据写入 spike 需要的 buffer 中
  // 遍历所有 buffer
  for (size_t i = 0; i < m_metadata.num_buffer; ++i) {
    // 从 metadata 获取内存区域基地址和尺寸
    uint64_t base = m_metadata.buffer_base[i];
    uint64_t actual_size = m_metadata.buffer_size[i];
    uint64_t alloc_size = m_metadata.buffer_allocsize[i];
    uint64_t aligned_size = (alloc_size + PGSIZE - 1) & ~(PGSIZE - 1);

    // 添加内存配置
    buffer.push_back(mem_cfg_t(reg_t(base), reg_t(aligned_size)));

    // 创建 mem_t 对象并拷贝数据
    mem_t* mem = new mem_t(aligned_size);
    if (i < m_buffer_data->size() && !(*m_buffer_data)[i].empty()) {
      // 有初始化数据的缓冲区
      if (actual_size > alloc_size) {
        printf("DifftestRef::initData Buffer[%zu] overflow: actual_size=0x%lx > alloc_size=0x%lx\n",
               i, actual_size, alloc_size);
        exit(1);
      }
      uint8_t* data = (*m_buffer_data)[i].data();
      if ((mem->store(0, (*m_buffer_data)[i].size(), data)) == false) {
        printf("DifftestRef::initData mem->store failed!\n");
        exit(1);
      }

    } else {
      // 未初始化缓冲区进行零初始化
      std::vector<uint8_t> zeros(alloc_size, 0);
      if ((mem->store(0, alloc_size, zeros.data())) == false) {
        printf("DifftestRef::initData mem->store failed!\n");
        exit(1);
      }
      printf("DifftestRef::initData m_buffer_data[%zu] is empty, store zeros\n", i);
    }

    // 保存到 buffer_data
    buffer_data.push_back(std::make_pair(base, mem));
  }
}
/************ functions from class spike_device *************/

static void help(int exit_code = 1) {
  fprintf(stderr, "now you are in help function");
  exit(exit_code);
}

bool sort_mem_region(const mem_cfg_t& a, const mem_cfg_t& b) {
  if (a.base == b.base)
    return (a.size < b.size);
  else
    return (a.base < b.base);
}

void merge_overlapping_memory_regions(std::vector<mem_cfg_t>& mems) {
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

static std::vector<mem_cfg_t> parse_mem_layout(const char* arg) {
  std::vector<mem_cfg_t> res;

  // handle legacy mem argument
  char* p;
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
    if (!*p || *p != ':')
      printf("command line input fromat wrong\n");
    auto size = strtoull(p + 1, &p, 0);

    // page-align base and size
    auto base0 = base, size0 = size;
    size += base0 % PGSIZE;
    base -= base0 % PGSIZE;
    if (size % PGSIZE != 0)
      size += PGSIZE - size % PGSIZE;

    if (base + size < base)
      printf("page size alignmentation failed\n");

    if (size != size0) {
      fprintf(stderr,
              "Warning: the memory at  [0x%llX, 0x%llX] has been realigned\n"
              "to the %ld KiB page size: [0x%llX, 0x%llX]\n",
              base0, base0 + size0 - 1, long(PGSIZE / 1024), base, base + size - 1);
    }

    res.push_back(mem_cfg_t(reg_t(base), reg_t(size)));
    if (!*p)
      break;
    if (*p != ',')
      help();
    arg = p + 1;
  }

  merge_overlapping_memory_regions(res);

  return res;
}

int DifftestRef::alloc_const_mem(uint64_t size, uint64_t* vaddr) {
  uint64_t base;
#define PGSHIFT 12
  const reg_t PGSIZE = 1 << PGSHIFT;
  if (size <= 0 || vaddr == nullptr)
    return -1;
  if (const_buffer.empty()) {
    base = VBASEADDR;
  } else {
    base = const_buffer.back().base + const_buffer.back().size;
  }

  auto base0 = base, size0 = size;
  size += base0 % PGSIZE;
  base -= base0 % PGSIZE;
  if (size % PGSIZE != 0)
    size += PGSIZE - size % PGSIZE;

  if (base + size < base)
    help();

  if (size != size0) {
    fprintf(stderr,
            "Warning: the memory at  [0x%lX, 0x%lX] has been realigned\n"
            "to the %ld KiB page size: [0x%lX, 0x%lX]\n",
            base0, base0 + size0 - 1, long(PGSIZE / 1024), base, base + size - 1);
  }

  printf("to allocate at 0x%lx with %ld bytes \n", base, size);

  const_buffer.push_back(mem_cfg_t(reg_t(base), reg_t(size)));
  const_buffer_data.push_back(std::pair(base, new mem_t(size)));

  *vaddr = base;
  return 0;
}

static unsigned long atoul_safe(const char* s) {
  char* e;
  auto res = strtoul(s, &e, 10);
  if (*e)
    help();
  return res;
}

static unsigned long atoul_nonzero_safe(const char* s) {
  auto res = atoul_safe(s);
  if (!res)
    help();
  return res;
}

static std::vector<int> parse_hartids(const char* s)
// 假设输入字符串是 "0,1,2,3"，那么执行后的返回结果为：
// std::vector<int> hartids = {0, 1, 2, 3};
{
  std::string const str(s);
  std::stringstream stream(str);
  std::vector<int> hartids;

  int n;
  while (stream >> n) {
    hartids.push_back(n);
    if (stream.peek() == ',')
      stream.ignore();
  }

  return hartids;
}

int DifftestRef::set_filename(const char* filename, const char* logname) {
  sprintf(srcfilename, "%s", filename);
  if (logname == nullptr)
    sprintf(logfilename, "%s.log", filename);
  else
    sprintf(logfilename, "%s", logname);
  return 0;
}

/**************** class DifftestRef **********************/

DifftestRef::DifftestRef() : sim(), state(), proc(), buffer(), buffer_data() {
  // 构造函数，difftest_init API 的入口
  srcfilename = new char[256];
  logfilename = new char[256];
  uint64_t lds_vaddr;
  uint64_t pc_src_vaddr;
  printf("DifftestRef initialize: allocating local memory: ");
  alloc_const_mem(0x10000000, &lds_vaddr);
  printf("DifftestRef initialize: allocating pc source memory: ");
  alloc_const_mem(0x10000000, &pc_src_vaddr);
}

void DifftestRef::init_sim(uint64_t knl_start_pc, uint64_t wg_id) {
  currwgid = wg_id;
  // DifftestRef::init_sim: 从 m_metadata 初始化参数
  num_warp = m_metadata.wg_size;
  uint64_t num_thread = m_metadata.wf_size;
  uint64_t num_workgroup_x = m_metadata.kernel_size[0];
  uint64_t num_workgroup_y = m_metadata.kernel_size[1];
  uint64_t num_workgroup_z = m_metadata.kernel_size[2];
  uint64_t init_sim_t_num_workgroup = num_workgroup_x * num_workgroup_y * num_workgroup_z;
  num_workgroup = 1; // for multi workgroup difftest. run 1 workgroup in each DifftestRef.
  uint64_t num_processor = num_warp * init_sim_t_num_workgroup; // 这是本kernel需要的所有warp的数量
  uint64_t ldssize = m_metadata.ldsSize;
  // uint64_t pdssize=m_metadata.pdsSize * num_thread;
  uint64_t pdssize = 0x10000000;
  uint64_t pdsbase = m_metadata.pdsBaseAddr;
  uint64_t start_pc = knl_start_pc;
  uint64_t knlbase = m_metadata.metaDataBaseAddr;
  if ((ldssize) > 0x10000000) {
    fprintf(stderr, "lds size is too large. please modify VBASEADDR");
    exit(-1);
  }

  // Create default configuration
  // 与 spike_device::run() 函数相比，这里的 cfg 改为了指针，
  // 以避免 init_sim() 结束后 cfg 被销毁
  cfg = new cfg_t(std::make_pair((reg_t)0, (reg_t)0), // initrd_bounds
                  nullptr,                            // bootargs
                  "RV32GCV",                          // ISA
                  DEFAULT_PRIV,                       // priv
                  DEFAULT_VARCH,                      // varch
                  parse_mem_layout("2048"),           // mem_layout
                  std::vector<int>(),                 // hartids
                  false                               // real_time_clint
  );

  // DifftestRef::init_sim: 默认初始化参数
  bool debug = false;
  bool halted = false;
  bool histogram = false;
  bool log = false;
  bool socket = false; // command line option -s
  bool dump_dts = false;
  bool dtb_enabled = true;
  const char* kernel = NULL;
  reg_t kernel_offset, kernel_size;
  std::vector<std::pair<reg_t, abstract_device_t*>> plugin_devices;
  std::unique_ptr<icache_sim_t> ic;
  std::unique_ptr<dcache_sim_t> dc;
  std::unique_ptr<cache_sim_t> l2;
  bool log_cache = false;
  bool log_commits = false;
  const char* log_path = nullptr;
  std::vector<std::function<extension_t*()>> extensions;
  const char* initrd = NULL;
  const char* dtb_file = NULL;
  uint16_t rbb_port = 0;
  bool use_rbb = false;
  unsigned dmi_rti = 0;
  reg_t blocksz = 64;
  debug_module_config_t dm_config = {.progbufsize = 2,
                                     .max_sba_data_width = 0,
                                     .require_authentication = false,
                                     .abstract_rti = 0,
                                     .support_hasel = true,
                                     .support_abstract_csr_access = true,
                                     .support_haltgroups = true,
                                     .support_impebreak = true};
  cfg_arg_t<size_t> nprocs(1); // init nprocs to 1, will be overwritten by -p arg

  // DifftestRef::init_sim: 生成 htif_args
  auto const device_parser = [&plugin_devices](const char* s) {
    const std::string str(s);
    std::istringstream stream(str);

    // We are parsing a string like name,base,args.

    // Parse the name, which is simply all of the characters leading up to the
    // first comma. The validity of the plugin name will be checked later.
    std::string name;
    std::getline(stream, name, ',');
    if (name.empty()) {
      throw std::runtime_error("Plugin name is empty.");
    }

    // Parse the base address. First, get all of the characters up to the next
    // comma (or up to the end of the string if there is no comma). Then try to
    // parse that string as an integer according to the rules of strtoull. It
    // could be in decimal, hex, or octal. Fail if we were able to parse a
    // number but there were garbage characters after the valid number. We must
    // consume the entire string between the commas.
    std::string base_str;
    std::getline(stream, base_str, ',');
    if (base_str.empty()) {
      throw std::runtime_error("Device base address is empty.");
    }
    char* end;
    reg_t base = static_cast<reg_t>(std::strtoull(base_str.c_str(), &end, 0));
    if (end != &*base_str.cend()) {
      throw std::runtime_error("Error parsing device base address.");
    }

    // The remainder of the string is the arguments. We could use getline, but
    // that could ignore newline characters in the arguments. That should be
    // rare and discouraged, but handle it here anyway with this weird in_avail
    // technique. The arguments are optional, so if there were no arguments
    // specified we could end up with an empty string here. That's okay.
    auto avail = stream.rdbuf()->in_avail();
    std::string args(avail, '\0');
    stream.readsome(&args[0], avail);

    plugin_devices.emplace_back(base, new mmio_plugin_device_t(name, args));
  };

  // command analyze
  option_parser_t parser;
  // 注册一堆命令行解析函数，被 parser 存储在 opts 中，告诉 parser 如何解析命令行参数
  parser.option('d', 0, 0, [&](const char* s) { debug = true; });
  parser.option('g', 0, 0, [&](const char* s) { histogram = true; });
  parser.option('l', 0, 0, [&](const char* s) { log = true; });
#ifdef HAVE_BOOST_ASIO
  parser.option('s', 0, 0, [&](const char* s) { socket = true; });
#endif
  // 将命令行选项的配置信息添加到 opts 列表中. 例如下面为选项 '-p' 注册了一个回调函数
  // 后续parser实际执行时，会将 '-p' 选项的参数转换为无符号整数并赋值给 nprocs
  parser.option('p', 0, 1, [&](const char* s) { nprocs = atoul_nonzero_safe(s); });
  parser.option('m', 0, 1, [&](const char* s) { cfg->mem_layout = parse_mem_layout(s); });
  // I wanted to use --halted, but for some reason that doesn't work.
  parser.option('H', 0, 0, [&](const char* s) { halted = true; });
  parser.option(0, "rbb-port", 1, [&](const char* s) {
    use_rbb = true;
    rbb_port = atoul_safe(s);
  });
  parser.option(0, "pc", 1, [&](const char* s) { cfg->start_pc = strtoull(s, 0, 0); });
  parser.option(0, "hartids", 1, [&](const char* s) {
    cfg->hartids = parse_hartids(s);
    cfg->explicit_hartids = true;
  });
  parser.option(0, "ic", 1, [&](const char* s) { ic.reset(new icache_sim_t(s)); });
  parser.option(0, "dc", 1, [&](const char* s) { dc.reset(new dcache_sim_t(s)); });
  parser.option(0, "l2", 1, [&](const char* s) { l2.reset(cache_sim_t::construct(s, "L2$")); });
  parser.option(0, "log-cache-miss", 0, [&](const char* s) { log_cache = true; });
  parser.option(0, "isa", 1, [&](const char* s) { cfg->isa = s; });
  parser.option(0, "priv", 1, [&](const char* s) { cfg->priv = s; });
  parser.option(0, "varch", 1, [&](const char* s) { cfg->varch = s; });
  parser.option(0, "gpgpuarch", 1, [&](const char* s) { cfg->gpgpuarch = s; });
  parser.option(0, "device", 1, device_parser);
  parser.option(0, "extension", 1, [&](const char* s) { extensions.push_back(find_extension(s)); });
  parser.option(0, "dump-dts", 0, [&](const char* s) { dump_dts = true; });
  parser.option(0, "disable-dtb", 0, [&](const char* s) { dtb_enabled = false; });
  parser.option(0, "dtb", 1, [&](const char* s) { dtb_file = s; });
  parser.option(0, "kernel", 1, [&](const char* s) { kernel = s; });
  parser.option(0, "initrd", 1, [&](const char* s) { initrd = s; });
  parser.option(0, "bootargs", 1, [&](const char* s) { cfg->bootargs = s; });
  parser.option(0, "real-time-clint", 0, [&](const char* s) { cfg->real_time_clint = true; });
  parser.option(0, "extlib", 1, [&](const char* s) {
    void* lib = dlopen(s, RTLD_NOW | RTLD_GLOBAL);
    if (lib == NULL) {
      fprintf(stderr, "Unable to load extlib '%s': %s\n", s, dlerror());
      exit(-1);
    }
  });
  parser.option(0, "dm-progsize", 1, [&](const char* s) { dm_config.progbufsize = atoul_safe(s); });
  parser.option(0, "dm-no-impebreak", 0,
                [&](const char* s) { dm_config.support_impebreak = false; });
  parser.option(0, "dm-sba", 1,
                [&](const char* s) { dm_config.max_sba_data_width = atoul_safe(s); });
  parser.option(0, "dm-auth", 0, [&](const char* s) { dm_config.require_authentication = true; });
  parser.option(0, "dmi-rti", 1, [&](const char* s) { dmi_rti = atoul_safe(s); });
  parser.option(0, "dm-abstract-rti", 1,
                [&](const char* s) { dm_config.abstract_rti = atoul_safe(s); });
  parser.option(0, "dm-no-hasel", 0, [&](const char* s) { dm_config.support_hasel = false; });
  parser.option(0, "dm-no-abstract-csr", 0,
                [&](const char* s) { dm_config.support_abstract_csr_access = false; });
  parser.option(0, "dm-no-halt-groups", 0,
                [&](const char* s) { dm_config.support_haltgroups = false; });
  parser.option(0, "log-commits", 0, [&](const char* s) { log_commits = true; });
  parser.option(0, "log", 1, [&](const char* s) { log_path = s; });
  FILE* cmd_file = NULL;
  parser.option(0, "debug-cmd", 1, [&](const char* s) {
    if ((cmd_file = fopen(s, "r")) == NULL) {
      fprintf(stderr, "Unable to open command file '%s'\n", s);
      exit(-1);
    }
  });
  parser.option(0, "blocksz", 1, [&](const char* s) {
    blocksz = strtoull(s, 0, 0);
    if (((blocksz & (blocksz - 1))) != 0) {
      fprintf(stderr, "--blocksz should be power of 2\n");
      exit(-1);
    }
  });
  // const char* argv[] = " -d -l --log-commits -p1 --isa rv64gv_zfh --varch vlen:256,elen:32
  // --gpgpuarch numw:1,numt:8,numwg:1 build/my.riscv > log/my.log 2>&1";
  int argc = 14;
  // mem的和sim的config可以直接赋值，但htif的只能通过命令行传
  // 为了减少出错的可能，用spike默认模式转为字符串传递。

  char arg_num_core[16];
  char arg_vlen_elen[64];
  char arg_mem_scope[64];
  char arg_gpgpu[256];
  char arg_start_pc[32];
  ;
  char arg_logfilename[256];
  sprintf(arg_logfilename, "--log=%s", logfilename);
  sprintf(arg_num_core, "-p%ld",
          num_processor); // arg_num_core = "-p{num_processor}"
  // sprintf(arg_gpgpu,
  //         "numw:%ld,numt:%ld,numwg:%ld,kernelx:%ld,kernely:%ld,kernelz:%ld,ldssize:"
  //         "0x%lx,pdssize:0x%lx,pdsbase:0x%lx,knlbase:0x%lx,currwgid:%lx",
  //         num_warp, num_thread, init_sim_t_num_workgroup, num_workgroup_x, num_workgroup_y, num_workgroup_z,
  //         ldssize, pdssize, pdsbase, knlbase, currwgid);
  sprintf(arg_gpgpu,
          "numw:%ld,numt:%ld,numwg:%ld,kernelx:%ld,kernely:%ld,kernelz:%ld,ldssize:"
          "0x%lx,pdssize:0x%lx,pdsbase:0x%lx,knlbase:0x%lx,currwgid:%lx",
          num_warp, num_thread, init_sim_t_num_workgroup, num_workgroup_x, num_workgroup_y, num_workgroup_z,
          ldssize, pdssize, pdsbase, knlbase, currwgid);
  printf("arg gpgpu is %s\n", arg_gpgpu);
  sprintf(arg_vlen_elen, "vlen:%ld,elen:%d", num_thread * 32, 32);
  sprintf(arg_mem_scope, "-m0x70000000:0x%lx", buffer.back().base + buffer.back().size);
  printf("vaddr mem scope is %s\n", arg_mem_scope);
  sprintf(arg_start_pc, "--pc=0x%lx", start_pc);
  // strcat(arg_mem_scope,temp);
  //----num_core----pc----mem_scope   //mem_scope is unused now.
  //---vlen_elen--gpgpu--log_file_output
  // 使用 strings 字符串数组存储默认参数
  char strings[][32] = {"spike",
                        "-l",
                        "--log-commits",
                        " ",
                        "--isa",
                        "rv32gcv_zfh",
                        " ",
                        "-m0x70000000:0x90000000",
                        "--varch",
                        " ",
                        "--gpgpuarch",
                        "numw:1,numt:8,numwg:1",
                        " ",
                        " "};
  // 将 strings 中的默认参数传入 argv
  char** argv = new char*[argc];
  for (int i = 0; i < argc; i++) {
    argv[i] = strings[i];
  }
  // 使用实际参数替换 argv 中的默认参数
  argv[11] = arg_gpgpu;
  argv[3] = arg_num_core;
  argv[12] = arg_logfilename;
  printf("src file is %s, run log is written to %s\n", srcfilename, logfilename);
  argv[13] = srcfilename;
  argv[9] = arg_vlen_elen;
  argv[7] = arg_mem_scope;
  argv[6] = arg_start_pc;
  for (int i = 0; i < argc; i++) {
    printf("%s ", argv[i]);
  }
  printf("\n");
  // 解析参数
  auto argv1 = parser.parse(argv);
  // nprocs is set to num_processor = wg_size * num_workgroup

  // 生成 htif_args
  std::vector<std::string> htif_args(argv1,
                                     (const char* const*)argv + argc); // vector的范围构造方法

  // Set default set of hartids based on nprocs, but don't set the
  // explicit_hartids flag (which means that downstream code can know that
  // we've only set the number of harts, not explicitly chosen their IDs).
  if (cfg->explicit_hartids) {
    if (nprocs.overridden() && (nprocs() != cfg->nprocs())) {
      std::cerr << "Number of specified hartids (" << cfg->nprocs()
                << ") doesn't match specified number of processors (" << nprocs() << ").\n";
      exit(1);
      // 这里是正确的吗？cfg.hartids().size() must equal nprocs()
      // however, the else block below sets cfg.hartids().size() = num_warp * SPIKE_RUN_WG_NUM <
      // nprocs()
    }
  } else {
    // Set default set of hartids based on nprocs, but don't set the
    // explicit_hartids flag (which means that downstream code can know that
    // we've only set the number of harts, not explicitly chosen their IDs).
    std::vector<int> default_hartids;
    default_hartids.reserve(nprocs()); // 最大大小为 num_processor = wg_size * num_workgroup
    for (size_t i = 0; i < num_warp * SPIKE_RUN_WG_NUM; ++i) {
      default_hartids.push_back(i);
    }
    cfg->hartids = default_hartids; // cfg.hartids().size() = num_warp * SPIKE_RUN_WG_NUM
  }

  // DifftestRef::init_sim: 初始化 mems 以供 new sim_t 使用
  // 对应 spike_device 的 all_buffer_data
  std::vector<std::pair<reg_t, mem_t*>> mems = const_buffer_data;
  mems.insert(mems.end(), buffer_data.begin(), buffer_data.end());

  proc.resize(num_workgroup);
  state.resize(num_workgroup);
  for (int j=0; j<num_workgroup; j++) {
    // Create simulator instance
    sim_t* sim_new = new sim_t(cfg,            // cfg
                    halted,         // halted
                    mems,           // mems
                    plugin_devices, // plugin_devices
                    htif_args,      // args
                    dm_config,      // dm_config
                    log_path,       // log_path
                    dtb_enabled,    // dtb_enabled
                    dtb_file,       // dtb_file
  #ifdef HAVE_BOOST_ASIO
                    nullptr, nullptr,
  #endif                    // HAVE_BOOST_ASIO
                    nullptr // cmd_file
    );

    // DifftestRef::init_sim: new sim_t 后的初始化
    if (dump_dts) {
      printf("%s", sim_new->get_dts());
      // return 0;
    }

    if (ic && l2)
      ic->set_miss_handler(&*l2);
    if (dc && l2)
      dc->set_miss_handler(&*l2);
    if (ic)
      ic->set_log(log_cache);
    if (dc)
      dc->set_log(log_cache);

    for (size_t i = 0; i < num_warp;
        i++) { // TODO: 此处不完善。感觉迭代范围应该是 num_warp * SPIKE_RUN_WG_NUM
      if (ic)
        sim_new->get_core(i)->get_mmu()->register_memtracer(&*ic);
      if (dc)
        sim_new->get_core(i)->get_mmu()->register_memtracer(&*dc);
      for (auto e : extensions)
        sim_new->get_core(i)->register_extension(e());
      sim_new->get_core(i)->get_mmu()->set_cache_blocksz(blocksz);
    }

    sim_new->set_debug(debug);
  #ifdef RISCV_ENABLE_COMMITLOG
    sim_new->configure_log(log, log_commits);
  #endif // RISCV_ENABLE_COMMITLOG
    sim_new->set_histogram(histogram);
    
    // DifftestRef::init_sim: Get the first processor
    for (size_t i = 0; i < num_warp; i++) {
      proc[j].push_back(sim_new->get_core(i));
      state[j].push_back(proc[j][i]->get_state());
    }

    sim_new->init_difftest(); // 来自 sim_t::run()
    sim.push_back(sim_new);
    // currwgid++;
  }

  // 跳过 spike 初始化指令，不知道是不是必要的
  // spike 每次运行前都会执行 ~5 条指令，然后才从 0x8000_0000 开始，尚不知为何
  for (int k = 0; k < num_workgroup; k++) {
    for (int i = 0; i < num_warp; i++) {
      step_info.wg_id = k;
      step_info.warp_id = i;
      step_info.num_step = 5; // spike 的初始化指令似乎每次都是 5 条
      step();
    }
  }
  // 初始化 spike 状态
  diff_ref_context_t ctx;
  ctx.xpr.resize(num_workgroup);
  ctx.pc.resize(num_workgroup);
  for (int k=0; k<num_workgroup; k++) {
    ctx.xpr[k].resize(num_warp);
    ctx.pc[k].resize(num_warp);
    for (int j = 0; j < num_warp; j++) {
        ctx.pc[k][j] = knl_start_pc;
    }
  }
  set_regs(&ctx, false); // 将knl_start_pc设置到ref中
}

DifftestRef::~DifftestRef() {
  for (const auto& pair : buffer_data) {
    delete pair.second;
  }
  for (const auto& pair : const_buffer_data) {
    delete pair.second;
  }
  delete cfg;
  for (auto& item: sim) delete item;
  delete srcfilename;
  delete logfilename;
}

void DifftestRef::step() {
  // 根据 step_info 步进 ref
  assert(step_info.wg_id < num_workgroup);
  assert(step_info.warp_id < num_warp);
  assert(step_info.num_step > 0);
  sim[step_info.wg_id]->sim_step_info.warp_id = step_info.warp_id;
  sim[step_info.wg_id]->sim_step_info.num_step = step_info.num_step;
  sim[step_info.wg_id]->step_difftest();
}

void DifftestRef::get_regs(diff_ref_context_t* ctx) {
  // 将 state 中的寄存器数据拷贝进 ctx:
  assert(ctx != nullptr);
  ctx->xpr.clear();
  ctx->pc.clear();
  ctx->xpr.resize(num_workgroup);
  ctx->pc.resize(num_workgroup);
  for (int k=0; k<num_workgroup; k++) {
    for (int j = 0; j < num_warp; j++) {
      std::array<uint64_t, 256> temp_xpr;
      for (int i = 0; i < NXPR; i++) {
        temp_xpr[i] = state[k][j]->XPR[i];
      }
      ctx->xpr[k].push_back(temp_xpr);
      ctx->pc[k].push_back(state[k][j]->pc);
    }
  }
}

void DifftestRef::set_regs(diff_ref_context_t* ctx, bool on_demand) {
  assert(ctx != nullptr);
  // 将 ctx 中的寄存器数据拷贝进 state:
  for (int k=0; k<num_workgroup; k++) {
    for (int j = 0; j < num_warp; j++) {
      for (int i = 0; i < NXPR; i++) {
        if (!on_demand || state[k][j]->XPR[i] != ctx->xpr[k][j][i]) {
          state[k][j]->XPR.write(i, ctx->xpr[k][j][i]);
        }
      }
      if (!on_demand || state[k][j]->pc != ctx->pc[k][j]) {
        state[k][j]->pc = ctx->pc[k][j];
      }
    }
  }
}

void DifftestRef::set_warp_xreg
  (uint32_t wg_id, uint32_t warp_id, uint32_t xreg_usage, diff_ref_warp_xreg_vec_t xreg) {
  assert(wg_id < num_workgroup);
  assert(warp_id < num_warp);
  assert(xreg.xreg.size() == xreg_usage);
  for (int i=0; i<xreg_usage; i++) {
    // 将 xreg 中的寄存器数据拷贝进 state:
    state[wg_id][warp_id]->XPR.write(i, xreg.xreg[i]);
  }
}

// void DifftestRef::memcpy_from_dut(reg_t vaddr, void* data, size_t size) {
//   bool found = false;
//   printf("DifftestRef::memcpy_from_dut(): copying to 0x%lx with %ld bytes\n", vaddr, size);

//   // Search in buffer_data
//   for (size_t i = 0; i < buffer.size(); ++i) {
//     if (vaddr >= buffer[i].base && vaddr < buffer[i].base + buffer[i].size) {
//       if (vaddr + size > buffer[i].base + buffer[i].size) {
//         fprintf(stderr, "Cannot copy to 0x%lx with size %ld\n", vaddr, size);
//         return;
//       }
//       buffer_data[i].second->store(vaddr - buffer_data[i].first, size, (const uint8_t*)data);
//       found = true;
//       printf("found data in buffer_data[%ld]\n", i);
//       break;
//     }
//   }

//   // Search in const_buffer_data if not found
//   if (!found) {
//     for (size_t i = 0; i < const_buffer.size(); ++i) {
//       if (vaddr >= const_buffer[i].base && vaddr < const_buffer[i].base + const_buffer[i].size) {
//         if (vaddr + size > const_buffer[i].base + const_buffer[i].size) {
//           fprintf(stderr, "Cannot copy to 0x%lx with size %ld\n", vaddr, size);
//           return;
//         }
//         const_buffer_data[i].second->store(vaddr - const_buffer_data[i].first, size,
//                                            (const uint8_t*)data);
//         found = true;
//         printf("found data in const_buffer_data[%ld]\n", i);
//         break;
//       }
//     }
//   }

//   if (!found) {
//     fprintf(stderr, "vaddr 0x%lx does not fit any allocated buffer\n", vaddr);
//   }
// }

void DifftestRef::display(int wg_id) {
  // 打印ref寄存器内容
  int k, i, j;
  for (k=0; k < num_workgroup; k++) {
    for (j = 0; j < num_warp; j++) {
      printf("Workgroup %d, Warp %d:\n", wg_id/*k*/, j);
      for (i = 0; i < NXPR; i++) {
        printf("%4s: " FMT_WORD " ", xpr_name[i], state[k][j]->XPR[i]);
        if (i % 4 == 3) {
          printf("\n");
        }
      }
      printf("pc: " FMT_WORD "\n", state[k][j]->pc);
    }
    printf("\n--------------------------------------------------\n\n");
  }
}

/***************** difftest API ********************/

static DifftestMultiWgRef* multi_wg_ref = nullptr; // 何时释放？

extern "C" {

void difftest_ref_init(const char* metadata_file, const char* data_file, const char* elf_file) {
  multi_wg_ref = new DifftestMultiWgRef();
  multi_wg_ref->initMetaData(metadata_file);
  for (uint64_t wg_id = 0; wg_id < multi_wg_ref->num_workgroup; wg_id++) {
    DifftestRef* ref = new DifftestRef();
    // init .metadata
    ref->initMetaData(metadata_file);
    // init .data
    ref->initData(data_file);
    ref->set_filename(elf_file, nullptr);
    uint64_t knl_start_pc = 0x80000000;
    ref->init_sim(knl_start_pc, wg_id); // 对应 spike_device::run() 函数
    multi_wg_ref->wg.push_back(ref);
  }
}

void difftest_ref_regcpy(diff_ref_context_t* ctx, bool direction, bool on_demand) {
  diff_ref_context_t* temp = new diff_ref_context_t();
  if (direction == DIFFTEST_TO_REF) {
    // DIFFTEST_TO_REF
    // ref->set_regs(ctx, on_demand);
    printf("error: difftest_ref_regcpy with direction DIFFTEST_TO_REF is not supported yet. it is recommended to check the regcpy direction.\n");
    assert(0);
    // 将ctx的寄存器状态拷贝到ref，on_demand为true表示只拷贝需要的寄存器
  } else {
    for (int i=0; i<multi_wg_ref->num_workgroup; i++) {
      multi_wg_ref->wg[i]->get_regs(temp); // 将ref的寄存器状态拷贝到ctx
      ctx->xpr.push_back(temp->xpr[0]);
      ctx->pc.push_back(temp->pc[0]);
    }
  }
}

void difftest_ref_set_warp_xreg(uint32_t wg_id, uint32_t warp_id, uint32_t xreg_usage, diff_ref_warp_xreg_vec_t xreg) {
  multi_wg_ref->wg[wg_id]->set_warp_xreg(0, warp_id, xreg_usage, xreg);
}

void difftest_ref_csrcpy(void* dut, bool direction) {} // TODO

// void difftest_ref_memcpy(uint64_t addr, void* buf, size_t n, bool direction) { // TODO
//   if (direction == DIFFTEST_TO_REF) {
//     // DIFFTEST_TO_REF
//     ref->memcpy_from_dut(addr, buf, n);
//   } else {
//     printf("difftest_memcpy with DIFFTEST_TO_DUT is not supported yet\n");
//     exit(1);
//   }
// } // TODO: new sim_t后，DifftestRef类的buffer就不再对sim_t有影响，那么如何实现该API？

int difftest_ref_exec(uint32_t wg_id, uint32_t warp_id, uint32_t n) {
  for (int i=0; i<n; i++) {
    multi_wg_ref->wg[wg_id]->step_info.wg_id = 0;
    multi_wg_ref->wg[wg_id]->step_info.warp_id = warp_id;
    multi_wg_ref->wg[wg_id]->step_info.num_step = 1;
    multi_wg_ref->wg[wg_id]->step(); // 执行1条指令
  }
  return multi_wg_ref->wg[wg_id]->done(0);
}

void difftest_ref_display() {
  for (int i=0; i<multi_wg_ref->num_workgroup; i++){
    multi_wg_ref->wg[i]->display(i);
  }
} // 打印寄存器内容

void update_dynamic_config(void* config) {} // TODO
// 香山的代码中只是将 config->debug_difftest 赋值给了 sim->enable_difftest_logs

void difftest_ref_uarchstatus_sync(void* dut) {} // TODO
// 香山：将dut的uarch_status选择性拷贝到ref

// int difftest_ref_store_commit(uint64_t* addr, uint64_t* data, uint8_t* mask) { // TODO
//   return ref->store_commit(addr, data, mask);
//   // 香山的实现中，store_commit函数中调用了sim_t::dut_store_commit
//   // 其中dut_store_commit并不在sim_t中，而是在它的父类的父类diff_trace_t中
// }

int difftest_ref_done(uint32_t wg_id) { return multi_wg_ref->wg[wg_id]->done(0); }

} // extern "C"