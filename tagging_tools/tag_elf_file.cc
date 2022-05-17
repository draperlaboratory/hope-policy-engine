#include <array>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>
#include <yaml-cpp/yaml.h>
#include "tagging_utils.h"

namespace policy_engine {

std::string check_output(const std::string& cmd) {
  char buf[128];
  std::string proc_stdout;
  std::unique_ptr<std::FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
  while (std::fgets(buf, sizeof(buf), pipe.get()) != nullptr)
    proc_stdout += buf;
  return proc_stdout;
}

int generate_tag_array(const std::string& elfname, range_file_t& range_file, const std::string& policy_name, YAML::Node policy_meta_info, int address_bytes) {
  std::string tag_array_filename = "tag_array";
  std::ofstream tag_array_file(tag_array_filename, std::ios::binary);
  int length = policy_meta_info["MaxBit"].as<int>();

  std::string bfd_target = address_bytes == 8 ? "elf64-littleriscv" : "elf32-littleriscv";
  std::string tool_prefix = "riscv64-unknown-elf-";

  std::vector<uint8_t> tag_array_bytes(address_bytes*(length + 1), 0);

  for (int i = 0; i < address_bytes; i++) {
    uint8_t byte = i < sizeof(length) ? (length >> (i*8)) : 0;
    tag_array_file << byte;
  }
  for (const uint8_t& byte : tag_array_bytes)
    tag_array_file << byte;
  tag_array_file.close();

  std::string objdump = check_output(tool_prefix + "objdump -h " + elfname);
  std::string elfname_policy = elfname + "-" + policy_name;
  std::string base_command = tool_prefix + "objcopy --target=" + bfd_target;
  if (objdump.find(".tag_array") != std::string::npos)
    base_command += " --update-section .tag_array=" + tag_array_filename + " ";
  else
    base_command += " --add-section .tag_array=" + tag_array_filename + " --set-section-flags .tag_array=readonly,data ";
  base_command += elfname + " " + elfname_policy;
  std::FILE* objcopy_proc = popen(base_command.c_str(), "r");
  int objcopy_result = pclose(objcopy_proc);
  if (objcopy_result < 0)
    return objcopy_result;
  
  uint64_t start_addr = 0;
  objdump = check_output(tool_prefix + "objdump --target " + bfd_target + " -h " + elfname + "-" + policy_name);
  std::istringstream objdump_iss(objdump);
  for (std::string line; std::getline(objdump_iss, line);) {
    if (line.find(".tag_array") != std::string::npos) {
      std::istringstream iss(line);
      int i = 0;
      for (std::string token; std::getline(iss, token, ' ');) {
        if (!token.empty()) {
          if (i == 3)
            start_addr = std::stoi(token, nullptr, 16);
          i++;
        }
      }
    }
  }
  if (start_addr > 0) {
    for (const auto& m : policy_meta_info["Metadata"]) {
      int mid = std::stoi(m["id"].as<std::string>());
      range_file.write_range(
        start_addr + (mid*address_bytes) + address_bytes,
        start_addr + (mid*address_bytes) + 2*address_bytes,
        m["name"].as<std::string>()
      );
    }
  }
  return objcopy_result;
}

}