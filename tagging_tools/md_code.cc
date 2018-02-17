#include <stdio.h>

//#include "policy_utils.h"

#include "tag_file.h"
#include "metadata_memory_map.h"
#include "metadata_factory.h"
#include "validator_exception.h"

#include "riscv_isa.h"

metadata_cache_t md_cache;
metadata_factory_t *md_factory;

extern void init_metadata_renderer(metadata_factory_t *md_factory);
//    init_metadata_renderer(md_factory);

void init() {
  try {
    md_factory = new metadata_factory_t(getenv("GENERATED_POLICY_DIR"));
    init_metadata_renderer(md_factory);
  } catch (validator::exception_t &e) {
    printf("exception: %s\n", e.what().c_str());
  }
}

void usage() {
  printf("usage: md_code <base_address> <code_address> <tag_file>\n");
}

struct abstract_instruction_stream_t {
  virtual ~abstract_instruction_stream_t() { }
  virtual bool read_instruction(insn_bits_t &insn) = 0;
  virtual void read_error() = 0;
};

struct read_error_t { };

class rv32_insn_stream_t : public abstract_instruction_stream_t {
  FILE *fp;
  public:
  rv32_insn_stream_t(FILE *fp) : fp(fp) { }
  virtual ~rv32_insn_stream_t() { }
  virtual bool read_instruction(insn_bits_t &insn) {
    if (fread(&insn, sizeof(insn_bits_t), 1, fp) != 1) {
      if (!feof(fp))
	read_error();
      return false;
    }
    return true;
  }
  virtual void read_error() { throw read_error_t(); }
};

int main(int argc, char **argv) {
try {
  address_t base_address;
  address_t code_address;
  const char *file_name;
  if (argc != 4) {
    usage();
    return 0;
  }
  base_address = strtol(argv[1], 0, 16);
  code_address = strtol(argv[2], 0, 16);
  file_name = argv[3];
  init();
  metadata_memory_map_t map(base_address, &md_cache);
//  printf("reading tags from %s\n", argv[2]);
  if (!load_tags(&map, file_name)) {
    printf("failed read\n");
    fprintf(stderr, "failed to read tags from %s\n", file_name);
    return 1;
  }
  printf("base addr = 0x%08x\ncode addr = 0x%08x\n", base_address, code_address);
//  printf("processing code stream...\n");
//  FILE *foo = fopen("/tmp/bits.bin", "rb");
//  rv32_insn_stream_t s(foo);
  rv32_insn_stream_t s(stdin);
  insn_bits_t insn;
  try {
    int cnt = 0;
    while (s.read_instruction(insn)) {
      uint32_t rs1, rs2, rs3, rd;
      int32_t imm;
      const char *name;
      int32_t flags = decode(insn, &rs1, &rs2, &rs3, &rd, &imm, &name);
      metadata_t const *metadata = md_factory->lookup_group_metadata(name);
      if (!metadata) {
	fprintf(stderr, "0x%08x: 0x%08x  %s - no group found for instruction\n", code_address, insn, name);
      } else {
//	std::string s = md_factory->render(metadata);
//	printf("0x%08x: %s\n", code_address, s.c_str());
	map.add_range(code_address, code_address + 4, metadata);
      }
      code_address += 4;
//      printf("%s\n", name);

//      printf("0x%08x ", insn);
//      if (cnt++ % 8 == 0)
//	printf("\n");
    }
  } catch (read_error_t &e) {
    fprintf(stderr, "read error on stdin\n");
    return 1;
  }
//  printf("done\n");
#if 0
  if (!load_tags(&map, argv[2])) {
    fprintf(stderr, "failed to read tags from %s\n", argv[2]);
    return 1;
  }
#endif
  printf("writing tags to %s\n", file_name);
  if (!save_tags(&map, file_name)) {
    printf("failed write of tag file\n");
    return 1;
  }
  return 0;
} catch (...) {
  printf("something awful happened\n");
  return 1;
}
}
