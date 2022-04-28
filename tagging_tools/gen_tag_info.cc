#include <array>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <gflags/gflags.h>
#include <iostream>
#include <list>
#include <memory>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <yaml-cpp/yaml.h>
#include "basic_elf_io.h"
#include "elf_loader.h"
#include "elf_section_tagger.h"
#include "llvm_metadata_tagger.h"
#include "md_asm_ann.h"
#include "md_embed.h"
#include "md_entity.h"
#include "md_header.h"
#include "md_index.h"
#include "md_range.h"
#include "op_code_tagger.h"
#include "soc_tagger.h"
#include "tag_elf_file.h"
#include "tagging_utils.h"

std::list<std::string> soc_exclude = {"SOC.Memory.DDR4_0", "SOC.Memory.Ram_0"};

std::string get_isp_prefix() {
  if (std::getenv("ISP_PREFIX")) {
    return std::string(std::getenv("ISP_PREFIX"));
  } else {
    return std::string(std::getenv("HOME")) + "/.local/isp";
  }
}

DEFINE_string(policy_dir, "", "Directory with generated policy yaml");
DEFINE_string(tag_file, "", "File to output tag info");
DEFINE_string(bin, "", "Program binary to parse for tags");
DEFINE_string(log, "WARNING", "Logging level (DEBUG, WARNING, INFO)");
DEFINE_bool(entities, false, "Entities file for policy");
DEFINE_string(arch, "rv32", "Currently supported: rv32 (default), rv64");
DEFINE_string(soc_file, "", "SOC config file. If present, write TMT headers for PEX firmware");

int main(int argc, char* argv[]) {
  gflags::SetUsageMessage("Generate tag ranges file from ELF binary");
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  char** args_entities = argv;

  if (FLAGS_policy_dir.empty()) {
    std::printf("Missing policy directory!\n");
    exit(-1);
  }
  if (FLAGS_tag_file.empty()) {
    std::printf("Missing tag output file!\n");
    exit(-1);
  }
  if (FLAGS_bin.empty()) {
    std::printf("Missing binary to tag!\n");
    exit(-1);
  }

  std::string asm_file_name;
  if (FLAGS_tag_file.find(".taginfo") != std::string::npos) {
    asm_file_name = FLAGS_tag_file;
    asm_file_name = asm_file_name.replace(FLAGS_tag_file.find(".taginfo"), 8, ".text");
  } else {
    asm_file_name = FLAGS_tag_file;
  }

  std::string policy_base = FLAGS_policy_dir.substr(FLAGS_policy_dir.find_last_of("/") + 1);
  
  struct stat tag_buf;
  if (stat(FLAGS_tag_file.c_str(), &tag_buf) == 0) {
    if (std::remove(FLAGS_tag_file.c_str()) != 0) {
      std::printf("could not remove %s\n", FLAGS_tag_file.c_str());
      exit(-1);
    }
  }

  policy_engine::stdio_reporter_t err;

  YAML::Node policy_modules = YAML::LoadFile(FLAGS_policy_dir + "/policy_modules.yml");
  YAML::Node policy_inits = YAML::LoadFile(FLAGS_policy_dir + "/policy_init.yml");
  YAML::Node policy_metas = YAML::LoadFile(FLAGS_policy_dir + "/policy_meta.yml");

  { // open a new scope so that the ELF image cleans up properly before reopening it
    policy_engine::elf_image_t elf_image(FLAGS_bin);

    policy_engine::RangeFile range_file;
    policy_engine::LLVMMetadataTagger llvm_tagger;

    if (policy_inits["Require"]) {
      if (policy_inits["Require"]["elf"])
        policy_engine::generate_rwx_ranges(elf_image, range_file);
      if (policy_inits["Require"]["llvm"])
        policy_engine::RangeMap range_map = llvm_tagger.generate_policy_ranges(elf_image, range_file, policy_inits);
      if (policy_inits["Require"]["SOC"] && !FLAGS_soc_file.empty())
        policy_engine::generate_soc_ranges(FLAGS_soc_file, range_file, policy_inits);
      int rc = policy_engine::generate_tag_array(FLAGS_bin, range_file, policy_base, policy_metas, FLAGS_arch == "rv64");
      if (rc != 0)
        std::printf("Couldn't add .tag_array to binary\n");
    }
    range_file.finish();
    
    int range_result = policy_engine::md_range(FLAGS_policy_dir, range_file.name, FLAGS_tag_file);
    if (range_result != 0) {
      err.error("md_range failed");
      exit(range_result);
    }
  }
  
  // have to reopen the file here because it's been edited and the current copy is corrupt
  policy_engine::elf_image_t elf_image_post(FLAGS_bin);

  tag_op_codes(FLAGS_policy_dir, elf_image_post, FLAGS_tag_file);

  std::vector<std::string> entities(&argv[1], &argv[argc - 1]);
  int entity_result = policy_engine::md_entity(FLAGS_policy_dir, FLAGS_bin, FLAGS_tag_file, entities, err);
  if (entity_result != 0) {
    std::printf("md_entity failed\n");
    exit(entity_result);
  }

  int embed_result = policy_engine::md_embed(FLAGS_tag_file, FLAGS_policy_dir, FLAGS_bin + "-" + policy_base, FLAGS_arch == "rv64", err);
  if (embed_result != 0)
    std::printf("md_embed failed\n");

  std::ofstream asm_file(asm_file_name);
  std::string llvm_od_cmd = get_isp_prefix() + "/bin/llvm-objdump -dS " + FLAGS_bin;
  std::printf("%s\n", llvm_od_cmd.c_str());
  std::FILE* llvm_proc = popen(llvm_od_cmd.c_str(), "r");
  char llvm_buf[128];
  std::string llvm_od_out;
  while (std::fgets(llvm_buf, sizeof(llvm_buf), llvm_proc) != nullptr)
    llvm_od_out += llvm_buf;
  asm_file << llvm_od_out;
  int llvm_result = pclose(llvm_proc);
  asm_file.close();
  if (llvm_result != 0) {
    std::printf("objdump failed\n");
    exit(llvm_result);
  }

  int asm_ann_result = policy_engine::md_asm_ann(FLAGS_policy_dir, FLAGS_tag_file, asm_file_name);
  if (asm_ann_result != 0) {
    err.error("md_asm_ann failed");
    exit(asm_ann_result);
  }

  if (!FLAGS_soc_file.empty()) {
    int index_result = policy_engine::md_index(FLAGS_tag_file, FLAGS_policy_dir, err);
    if (index_result != 0) {
      err.error("md_index failed");
      exit(index_result);
    }

    int header_result = policy_engine::md_header(FLAGS_bin, FLAGS_soc_file, FLAGS_tag_file, FLAGS_policy_dir, soc_exclude, err);
    if (header_result != 0) {
      err.error("md_header failed");
      exit(header_result);
    }
  }

  return 0;
}