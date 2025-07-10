// See LICENSE for license details.

#ifndef _RISCV_DISASM_H
#define _RISCV_DISASM_H

#include "decode.h"
#include "isa_parser.h"
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>

extern const char* xpr_name[NXPR];
extern const char* fpr_name[NFPR];
extern const char* vr_name[NVPR];
extern const char* csr_name(int which);

class arg_t
{
 public:
  virtual std::string to_string(insn_t val) const = 0;
  virtual ~arg_t() {}
};

class disasm_insn_t
{
 public:
  NOINLINE disasm_insn_t(const char* name_, uint64_t match, uint64_t mask,
                         const std::vector<const arg_t*>& args)
    : match(match), mask(mask), args(args)
  {
    name = name_;
    std::replace(name.begin(), name.end(), '_', '.');
  }

  bool operator == (insn_t insn) const
  {
    return (insn.bits() & mask) == match;
  }

  const char* get_name() const
  {
    return name.c_str();
  }

  std::string to_string(insn_t insn) const
  {
    std::string s(name);

    if (args.size())
    {
      bool next_arg_optional  = false;
      s += std::string(std::max(1, 8 - int(name.size())), ' ');
      for (size_t i = 0; i < args.size(); i++) {
        if (args[i] == nullptr) {
          next_arg_optional = true;
          continue;
        }
        std::string argString = args[i]->to_string(insn);
        if (next_arg_optional) {
          next_arg_optional = false;
          if (argString.empty()) continue;
        }
        if (i != 0) s += ", ";
        s += argString;
      }
    }
    return s;
  }

  uint64_t get_match() const { return match; }
  uint64_t get_mask() const { return mask; }

 private:
  uint64_t match;
  uint64_t mask;
  std::vector<const arg_t*> args;
  std::string name;
};

class disassembler_t
{
 public:
  disassembler_t(const isa_parser_t *isa);
  ~disassembler_t();

  std::string disassemble(insn_t insn) const;
  const disasm_insn_t* lookup(insn_t insn) const;

  void add_insn(disasm_insn_t* insn);

 private:
  static const int HASH_SIZE = 1023;
  std::vector<const disasm_insn_t*> chain[HASH_SIZE+1];

  void add_instructions(const isa_parser_t* isa);

  const disasm_insn_t* probe_once(insn_t insn, size_t idx) const;

  static const uint64_t MASK1 = 0xfc0000000000387fULL;
  static const uint64_t MASK2 = 0xe003;

  static unsigned int hash(insn_bits_t insn, uint64_t mask)
  {
    return (insn & mask) % HASH_SIZE;
  }
};

#endif
