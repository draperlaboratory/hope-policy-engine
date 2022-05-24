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
#include "elf_loader.h"
#include "llvm_metadata_tagger.h"
#include "md_asm_ann.h"
#include "md_embed.h"
#include "md_entity.h"
#include "md_header.h"
#include "md_index.h"
#include "metadata_factory.h"
#include "metadata_memory_map.h"
#include "range_map.h"
#include "reporter.h"
#include "tag_elf_file.h"
#include "tag_file.h"

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
DEFINE_string(soc_file, "", "SOC config file. If present, write TMT headers for PEX firmware");

int main(int argc, char* argv[]) {
  policy_engine::reporter_t err;

  gflags::SetUsageMessage("Generate tag ranges file from ELF binary");
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  char** args_entities = argv;

  if (FLAGS_policy_dir.empty()) {
    err.error("Missing policy directory!\n");
    exit(-1);
  }
  if (FLAGS_tag_file.empty()) {
    err.error("Missing tag output file!\n");
    exit(-1);
  }
  if (FLAGS_bin.empty()) {
    err.error("Missing binary to tag!\n");
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
  
  if (struct stat tag_buf; stat(FLAGS_tag_file.c_str(), &tag_buf) == 0)
    if (std::remove(FLAGS_tag_file.c_str()) != 0)
      throw std::ios::failure("could not remove " + FLAGS_tag_file);

  YAML::Node policy_modules = YAML::LoadFile(FLAGS_policy_dir + "/policy_modules.yml");
  YAML::Node policy_inits = YAML::LoadFile(FLAGS_policy_dir + "/policy_init.yml");
  YAML::Node policy_metas = YAML::LoadFile(FLAGS_policy_dir + "/policy_meta.yml");

  policy_engine::range_map_t range_map;
  if (policy_inits["Require"]) {
    policy_engine::elf_image_t elf_image(FLAGS_bin);
    policy_engine::llvm_metadata_tagger_t llvm_tagger(err);

    if (policy_inits["Require"]["elf"])
      range_map.add_rwx_ranges(elf_image, err);
    if (policy_inits["Require"]["llvm"])
      llvm_tagger.add_policy_ranges(range_map, elf_image, policy_inits);
    if (policy_inits["Require"]["SOC"] && !FLAGS_soc_file.empty())
      range_map.add_soc_ranges(FLAGS_soc_file, policy_inits, err);
    if (!policy_engine::add_tag_array(range_map, FLAGS_bin, policy_base, policy_metas, elf_image.word_bytes()))
      err.error("Couldn't add .tag_array to binary\n");
  }

  policy_engine::metadata_memory_map_t md_memory_map;
  policy_engine::metadata_factory_t md_factory(FLAGS_policy_dir);

  md_factory.apply_tags(md_memory_map, range_map);
  
  // have to reopen the file here because it's been edited and the current copy is corrupt
  policy_engine::elf_image_t elf_image_post(FLAGS_bin);

  for (const policy_engine::elf_section_t& section : elf_image_post.sections)
    if (section.flags & SHF_EXECINSTR)
      md_factory.tag_opcodes(md_memory_map, section.address, section.data, section.size, err);

  std::vector<std::string> entities{FLAGS_policy_dir + "/policy_entities.yml"};
  for (int i = 1; i < argc; i++)
    entities.push_back(argv[i]);
  policy_engine::md_entity(md_factory, md_memory_map, elf_image_post, entities, err);

  policy_engine::md_embed(md_factory, md_memory_map, elf_image_post, FLAGS_bin + "-" + policy_base, err);

  std::ofstream asm_file(asm_file_name);
  std::string llvm_od_cmd = get_isp_prefix() + "/bin/llvm-objdump -dS " + FLAGS_bin;
  std::FILE* llvm_proc = popen(llvm_od_cmd.c_str(), "r");
  char llvm_buf[128];
  std::string llvm_od_out;
  while (std::fgets(llvm_buf, sizeof(llvm_buf), llvm_proc) != nullptr)
    llvm_od_out += llvm_buf;
  asm_file << llvm_od_out;
  int llvm_result = pclose(llvm_proc);
  asm_file.close();
  if (llvm_result != 0) {
    err.error("objdump failed\n");
    exit(llvm_result);
  }

  policy_engine::md_asm_ann(md_factory, md_memory_map, asm_file_name);

  if (!FLAGS_soc_file.empty()) {
    policy_engine::md_index(md_factory, md_memory_map, FLAGS_tag_file, err);
    policy_engine::md_header(FLAGS_bin, FLAGS_soc_file, FLAGS_tag_file, FLAGS_policy_dir, soc_exclude, err);
  }

  return 0;
}