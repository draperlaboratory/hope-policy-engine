#ifndef VALIDATOR_H
#define VALIDATOR_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "renode_validator.h"

typedef uint32_t address_t;
typedef uint32_t insn_bits_t;

class abstract_validator_t {
  public:
  virtual ~abstract_validator_t() { }
  virtual bool validate(address_t pc, insn_bits_t insn) = 0;
  virtual void commit() = 0;
};

class always_ok_validator_t final : public abstract_validator_t {
  public:
  bool validate(address_t pc, insn_bits_t insn) { return true; }
  void commit() { }
};

class abstract_renode_validator_t : abstract_validator_t {
  protected:
  RegisterReader_t reg_reader;
  MemoryReader_t mem_reader;
  public:
  abstract_renode_validator_t(RegisterReader_t rr, MemoryReader_t mr) : reg_reader(rr), mem_reader(mr) { }
  virtual ~abstract_renode_validator_t() { }
  virtual bool validate(address_t pc, insn_bits_t insn) = 0;
  virtual void commit() = 0;
};

#endif
