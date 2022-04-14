#include <array>
#include <cstdlib>
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
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
  while (std::fgets(buf, sizeof(buf), pipe.get()) != nullptr)
    proc_stdout += buf;
  return proc_stdout;
}

int generate_tag_array(const std::string& elfname, RangeFile& range_file, const std::string& policy_name, YAML::Node policy_meta_info, bool rv64) {
  std::string tag_array_filename = "/tmp/tag_array_XXXXXX";
  int tag_array_file = mkstemp((char*)tag_array_filename.data());
  int length = policy_meta_info["MaxBit"].as<int>();

  int bytes_per_address = (rv64 ? 64 : 32)/8;
  std::string bfd_target = rv64 ? "elf64-littleriscv" : "elf32-littleriscv";
  std::string tool_prefix = "riscv64-unknown-elf-";

  std::vector<uint8_t> tag_array_bytes(bytes_per_address*(length + 1), 0);

  for (int i = 0; i < bytes_per_address; i++) {
    uint8_t byte = length >> (i*8);
    write(tag_array_file, &byte, 1);
  }
  write(tag_array_file, tag_array_bytes.data(), tag_array_bytes.size());
  close(tag_array_file);


  std::string objdump = check_output(tool_prefix + "objdump -h " + elfname);
  std::string base_command = objdump.find(".tag_array") != std::string::npos ?
    tool_prefix + "objcopy --target " + bfd_target + " --update-section .tag_array=" + tag_array_filename + " " + elfname + " " + elfname + "-" + policy_name :
    tool_prefix + "objcopy --target " + bfd_target + " --add-section .tag_array=" = tag_array_filename + " --set-section-flags .tag_array=readonly,data " + elfname + " " + elfname + "-" + policy_name;
  std::FILE* objcopy_proc = popen(base_command.c_str(), "r");
  int objcopy_result = pclose(objcopy_proc);
  unlink(tag_array_filename.c_str());
  if (objcopy_result < 0)
    return objcopy_result;
  
  uint64_t start_addr = 0;
  objdump = check_output(tool_prefix + "objdump --target " + bfd_target + " -h " + elfname + "-" + policy_name);
  std::istringstream objdump_iss(objdump);
  for (std::string line; std::getline(objdump_iss, line);) {
    if (line.find(".tag_array") != std::string::npos) {
      std::istringstream iss(line);
      int i = 0;
      for (std::string token; std::getline(iss, token, ' '); i++) {
        if (i == 3)
          start_addr = std::stoi(token, nullptr, 16);
      }
    }
  }
  if (start_addr > 0) {
    for (const auto& m : policy_meta_info["Metadata"]) {
      int mid = std::stoi(m["id"].as<std::string>());
      range_file.write_range(
        start_addr + (mid*bytes_per_address) + bytes_per_address,
        start_addr + (mid*bytes_per_address) + 2*bytes_per_address,
        m["name"].as<std::string>()
      );
    }
  }
  return objcopy_result;
}

}