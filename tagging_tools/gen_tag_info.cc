#include <array>
#include <cstdio>
#include <cstdlib>
#include <gflags/gflags.h>
#include <iostream>
#include <memory>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <yaml-cpp/yaml.h>
#include "basic_elf_io.h"
#include "elf_loader.h"
#include "elf_section_tagger.h"
#include "llvm_metadata_tagger.h"
#include "soc_tagger.h"
#include "tag_elf_file.h"
#include "tagging_utils.h"

std::array<std::string, 2> soc_exclude = {"SOC.Memory.DDR4_0", "SOC.Memory.Ram_0"};

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
DEFINE_string(entites, "", "Entities file for policy");
DEFINE_string(arch, "rv32", "Currently supported: rv32 (default), rv64");
DEFINE_string(soc_file, "", "SOC config file. If present, write TMT headers for PEX firmware");

int main(int argc, char* argv[]) {
  std::string md_range = "md_range";
  std::string md_code = "md_code";
  std::string md_asm_ann = "md_asm_ann";
  std::string md_embed = "md_embed";
  std::string md_entity = "md_entity";
  std::string md_header = "md_header";
  std::string md_index = "md_index";

  gflags::SetUsageMessage("Generate tag ranges file from ELF binary");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

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
  
  struct stat buf;
  if (stat(FLAGS_tag_file.c_str(), &buf) == 0) {
    if (std::remove(FLAGS_tag_file.c_str()) != 0) {
      std::printf("could not remove %s\n", FLAGS_tag_file.c_str());
      exit(-1);
    }
  }

  YAML::Node policy_modules = YAML::LoadFile(FLAGS_policy_dir + "/policy_modules.yml");
  YAML::Node policy_inits = YAML::LoadFile(FLAGS_policy_dir + "/policy_init.yml");
  YAML::Node policy_metas = YAML::LoadFile(FLAGS_policy_dir + "/policy_meta.yml");

  std::unique_ptr<FILE, decltype(&fclose)> elf_file(fopen(FLAGS_bin.c_str(), "r"), fclose);
  policy_engine::stdio_reporter_t err;
  policy_engine::FILE_reader_t reader(elf_file.get());
  policy_engine::elf_image_t elf_image(&reader, &err);

  policy_engine::RangeFile range_file;
  policy_engine::LLVMMetadataTagger llvm_tagger;

  if (policy_inits["Require"]) {
    if (policy_inits["Require"].as<std::string>().find("elf") != std::string::npos)
      policy_engine::generate_rwx_ranges(elf_image, range_file);
    if (policy_inits["Require"].as<std::string>().find("llvm") != std::string::npos)
      policy_engine::RangeMap range_map = llvm_tagger.generate_policy_ranges(elf_image, range_file, policy_inits);
    if (policy_inits["Require"].as<std::string>().find("SOC") != std::string::npos && !FLAGS_soc_file.empty())
      policy_engine::generate_soc_ranges(FLAGS_soc_file, range_file, policy_inits);
    size_t last = FLAGS_policy_dir.find_last_of("/");
    int rc = policy_engine::generate_tag_array(FLAGS_bin, range_file, FLAGS_policy_dir.substr(0, last), policy_metas, FLAGS_arch == "rv64");
    if (rc != 0)
      std::printf("Couldn't add .tag_array to binary\n");
  }
  range_file.finish();

  if (FLAGS_arch == "rv64")
    md_range += "64";
    md_code += "64";
    md_asm_ann += "64";
    md_embed += "64";
    md_entity += "64";
    md_header += "64";
    md_index += "64";
  
  int range_result = pclose(popen((md_range + " " + FLAGS_policy_dir + " " + range_file.name() + " " + FLAGS_tag_file).c_str(), "r"));
  if (range_result != 0)
    std::printf("md_range failed\n");
    exit(range_result);
}