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
#include "op_code_tagger.h"
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
DEFINE_bool(entities, false, "Entities file for policy");
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

  std::unique_ptr<FILE, decltype(&fclose)> elf_file(fopen(FLAGS_bin.c_str(), "r"), fclose);
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
  
  std::unique_ptr<FILE, decltype(&fclose)> elf_file_post(fopen(FLAGS_bin.c_str(), "r"), fclose);
  policy_engine::FILE_reader_t reader_post(elf_file_post.get());
  policy_engine::elf_image_t elf_image_post(&reader, &err);

  tag_op_codes(FLAGS_policy_dir, md_code, elf_image_post, FLAGS_tag_file);
  std::string entities_flat = "";
  if (FLAGS_entities)
    for (int i = 0; i < argc; i++)
      entities_flat += std::string(" ") + argv[i];
  int entity_result = pclose(popen((md_entity + " " + FLAGS_policy_dir + " " + FLAGS_bin + FLAGS_tag_file + entities_flat).c_str(), "r"));
  if (entity_result != 0) {
    std::printf("md_entity failed\n");
  }

  std::FILE* asm_file = fopen(asm_file_name.c_str(), "w");
  std::FILE* llvm_proc = popen((get_isp_prefix() + "/bin/llvm-objdump -dS " + FLAGS_bin).c_str(), "r");
  char llvm_buf[128];
  while (std::fgets(llvm_buf, sizeof(llvm_buf), llvm_proc) != nullptr)
    fwrite(llvm_buf, sizeof(llvm_buf), 1, asm_file);
  int llvm_result = pclose(llvm_proc);
  if (llvm_result != 0) {
    std::printf("objdump failed\n");
    exit(llvm_result);
  }
  fclose(asm_file);

  int asm_ann_result = pclose(popen((md_asm_ann + " " + FLAGS_policy_dir + " " + FLAGS_tag_file + " " + asm_file_name).c_str(), "r"));
  if (asm_ann_result != 0) {
    std::printf("md_asm_ann failed\n");
    exit(asm_ann_result);
  }

  if (!FLAGS_soc_file.empty()) {
    int index_result = pclose(popen((md_index + " " + FLAGS_tag_file + " " + FLAGS_policy_dir).c_str(), "r"));
    if (index_result != 0) {
      std::printf("md_index failed");
      exit(index_result);
    }
    std::string soc_exclude_flat;
    for (const std::string& exclude : soc_exclude)
      soc_exclude_flat += " " + exclude;
    int header_result = pclose(popen((md_header + " " + FLAGS_bin + " " + FLAGS_soc_file + " " + FLAGS_tag_file + " " + FLAGS_policy_dir + soc_exclude_flat).c_str(), "r"));
    if (header_result != 0) {
      std::printf("md_header failed\n");
      exit(header_result);
    }
  }
}