#include <array>
#include <gflags/gflags.h>
#include <cstdlib>
#include <string>
#include <unistd.h>

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
}