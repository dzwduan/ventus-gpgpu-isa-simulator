编译过程：

`git clone` ventus-gpgpu-isa-simulator 后，切换到 difftest 分支，随后运行以下命令：

```Bash
mkdir build && cd build/
../configure --enable-commitlog
make
```

运行结束后，`build/` 目录下会出现 `libdifftest.so` 动态库。

API 列表：

```C++
void difftest_init(const char* metadata_file, const char* data_file, const char* elf_file);
// 初始化 REF 侧，需要 .metadata 和 .data 和 .riscv 文件

int difftest_exec(uint64_t n);
// 步进 n 条指令，返回值为 1 表示程序已运行结束
// 目前仅支持所有 warp 同时步进，不支持单 warp 粒度的控制

int difftest_display();
// 打印 XPR 和 PC

void difftest_regcpy(diff_context_t* ctx, bool direction, bool on_demand);
// direction 为 true 时将 ctx 的 XPR 和 PC 拷贝到 REF 侧，
// 此时 on_demand 表示只拷贝 ctx 与 REF 有差异的寄存器
// direction 为 false 时将 REF 侧的 XPR 和 PC 拷贝进 ctx

int difftest_done();
// 返回值为 1 表示程序已运行结束
```

以下是一些编写 difftest 顶层仿真程序时可能有用的定义：

```C++
// definitions from REF
const int NXPR = 256;
const int NFPR = 256;
const int NVPR = 256;
const int NCSR = 4096;
typedef struct {
  std::vector<std::array<uint64_t, 256>> xpr;
  std::vector<uint64_t> pc;
} diff_context_t;
enum { DIFFTEST_FROM_REF, DIFFTEST_TO_REF };
#define FMT_WORD "0x%016lx"
const char* xpr_name[] = {
  "x0",  "x1",  "x2",  "x3",  "x4",  "x5",  "x6",  "x7",
  "x8",  "x9",  "x10", "x11", "x12", "x13", "x14", "x15",
  "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23",
  "x24", "x25", "x26", "x27", "x28", "x29", "x30", "x31",
  "x32",  "x33",  "x34",  "x35",  "x36",  "x37",  "x38",  "x39",  "x40",  "x41",  "x42",  "x43",  "x44",  "x45",  "x46",  "x47",  "x48",  "x49",  "x50",  "x51",  "x52",  "x53",  "x54",  "x55",  "x56",  "x57",  "x58",  "x59",  "x60",  "x61",  "x62",  "x63",  "x64",  "x65",  "x66",  "x67",  "x68",  "x69",  "x70",  "x71",  "x72",  "x73",  "x74",  "x75",  "x76",  "x77",  "x78",  "x79",  "x80",  "x81",  "x82",  "x83",  "x84",  "x85",  "x86",  "x87",  "x88",  "x89",  "x90",  "x91",  "x92",  "x93",  "x94",  "x95",  "x96",  "x97",  "x98",  "x99",  "x100",  "x101",  "x102",  "x103",  "x104",  "x105",  "x106",  "x107",  "x108",  "x109",  "x110",  "x111",  "x112",  "x113",  "x114",  "x115",  "x116",  "x117",  "x118",  "x119",  "x120",  "x121",  "x122",  "x123",  "x124",  "x125",  "x126",  "x127",  "x128",  "x129",  "x130",  "x131",  "x132",  "x133",  "x134",  "x135",  "x136",  "x137",  "x138",  "x139",  "x140",  "x141",  "x142",  "x143",  "x144",  "x145",  "x146",  "x147",  "x148",  "x149",  "x150",  "x151",  "x152",  "x153",  "x154",  "x155",  "x156",  "x157",  "x158",  "x159",  "x160",  "x161",  "x162",  "x163",  "x164",  "x165",  "x166",  "x167",  "x168",  "x169",  "x170",  "x171",  "x172",  "x173",  "x174",  "x175",  "x176",  "x177",  "x178",  "x179",  "x180",  "x181",  "x182",  "x183",  "x184",  "x185",  "x186",  "x187",  "x188",  "x189",  "x190",  "x191",  "x192",  "x193",  "x194",  "x195",  "x196",  "x197",  "x198",  "x199",  "x200",  "x201",  "x202",  "x203",  "x204",  "x205",  "x206",  "x207",  "x208",  "x209",  "x210",  "x211",  "x212",  "x213",  "x214",  "x215",  "x216",  "x217",  "x218",  "x219",  "x220",  "x221",  "x222",  "x223",  "x224",  "x225",  "x226",  "x227",  "x228",  "x229",  "x230",  "x231",  "x232",  "x233",  "x234",  "x235",  "x236",  "x237",  "x238",  "x239",  "x240",  "x241",  "x242",  "x243",  "x244",  "x245",  "x246",  "x247",  "x248",  "x249",  "x250",  "x251",  "x252",  "x253",  "x254",  "x255"
};
// end definitions from REF
```