/* Copyright © 2017-2019 The Charles Stark Draper Laboratory, Inc. and/or Dover Microsystems, Inc. */
/* All rights reserved. */

/* Use and disclosure subject to the following license. */

/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the */
/* "Software"), to deal in the Software without restriction, including */
/* without limitation the rights to use, copy, modify, merge, publish, */
/* distribute, sublicense, and/or sell copies of the Software, and to */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions: */

/* The above copyright notice and this permission notice shall be */
/* included in all copies or substantial portions of the Software. */

/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND */
/* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE */
/* LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION */
/* OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION */
/* WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include "metadata_index_map.h"
#include "metadata_register_map.h"
#include "tag_file.h"
#include "uleb.h"

void usage() {
  std::printf("usage: dump_tags <tag_file> <-f <num_entries>>\n");
  std::printf("\t-f firmware tag format\n");
}

void dump_firmware_tags(const char* tag_filename, size_t num_entries) {
  policy_engine::reporter_t err;
  std::list<policy_engine::range_t> code_ranges;
  std::list<policy_engine::range_t> data_ranges;
  std::vector<policy_engine::metadata_t> metadata_values;
  int32_t register_default;
  int32_t csr_default;
  int32_t env_default;

  auto memory_index_map = policy_engine::metadata_index_map_t<policy_engine::metadata_memory_map_t, policy_engine::range_t>();
  auto register_index_map = policy_engine::metadata_index_map_t<policy_engine::metadata_register_map_t, std::string>();
  auto csr_index_map = policy_engine::metadata_index_map_t<policy_engine::metadata_register_map_t, std::string>();

  if (!load_firmware_tag_file(
    code_ranges, data_ranges, metadata_values,
    memory_index_map, register_index_map, csr_index_map,
    tag_filename,
    err,
    register_default, csr_default, env_default
  )) {
    throw std::runtime_error("Failed to load firmware tag file\n");
  }

  std::printf("Code ranges:\n");
  for (const policy_engine::range_t& r : code_ranges) {
    std::printf("{ 0x%" PRIaddr_pad " - 0x%" PRIaddr_pad " }\n", r.start, r.end);
  }

  std::printf("\nData ranges:\n");
  for (const policy_engine::range_t& r : data_ranges) {
    std::printf("{ 0x%" PRIaddr_pad " - 0x%" PRIaddr_pad " }\n", r.start, r.end);
  }

  std::printf("\nMetadata values:\n");
  for (size_t i = 0; i < metadata_values.size(); i++) {
    std::printf("%lu: { ", i);
    for (const meta_t& m : metadata_values[i]) {
      std::printf("%lx ", m);
    }
    std::printf("}\n");
  }

  std::printf("\nRegister tag entries:\n");
  std::printf("Default: %x\n", register_default);
  for (const auto& [ reg, ind ] : register_index_map) {
    std::printf("%s: %x\n", reg.c_str(), ind);
  }

  std::printf("\nCSR tag entries:\n");
  std::printf("Default: %x\n", csr_default);
  for(const auto& [ reg, ind ] : csr_index_map) {
    std::printf("%s: %x\n", reg.c_str(), ind);
  }

  std::printf("\nEnv tag default: %x\n", env_default);

  std::printf("\nMemory tag entries (showing %lu of %lu):\n", num_entries, memory_index_map.size());
  size_t entry_index = 0;
  for (const auto& [ range, ind ] : memory_index_map) {
    std::printf("{ 0x%" PRIaddr_pad " - 0x%" PRIaddr_pad " }: %x\n", range.start, range.end, ind);

    entry_index++;
    if (entry_index == num_entries) {
      break;
    }
  }
}

void dump_tags(const std::string& file_name) {
  policy_engine::uleb_reader_t reader(file_name);
  if (!reader)
    throw std::ios::failure("could not open " + file_name);

  int i = 0;
  uint8_t is64bit;

  if (reader.read_uleb<uint8_t>(is64bit) <= 0) {
    throw std::runtime_error("could not read 64-bit encoding flag byte");
  }
  
  while (!reader.eof()) {
    uint64_t start, end;
    uint32_t metadata_count;

    if (reader.read_uleb<uint64_t>(start) <= 0) {
      throw std::runtime_error("could not read range start");
    }

    if (reader.read_uleb<uint64_t>(end) <= 0) {
      throw std::runtime_error("could not read range end");
    }

    if (reader.read_uleb<uint32_t>(metadata_count) <= 0) {
      throw std::runtime_error("could not read metadata_count");
    }

    if (end < start) {
      throw std::runtime_error("range [" + std::to_string(start) + "," + std::to_string(end) + ") ends before start");
    } else if (static_cast<int32_t>(metadata_count) < 0) {
      throw std::runtime_error(std::string("illegal metadata count ") + std::to_string(static_cast<int>(metadata_count)));
    }

    std::printf("Entry %d, 0x%" PRIaddr_pad " - 0x%" PRIaddr_pad " (%d)\n", i++, start, end, metadata_count);

    std::printf("\tMetadata List:  ");

    for (uint32_t i = 0; i < metadata_count; i++) {
      meta_t meta;
      if (reader.read_uleb<meta_t>(meta) <= 0) {
        throw std::runtime_error("could not read meta value");
      }

      std::printf("%016lx, ", meta);
    }

    std::printf("end.\n");
  }
}

int main(int argc, char* argv[]) {
  policy_engine::reporter_t err;
  const char *tag_filename;
  char arg;
  int entries_arg = 0;
  bool firmware = false;
  size_t num_entries = 16;

  // Retrieve memory metadata from tag file
  tag_filename = argv[1];

  while ((arg = getopt(argc, argv, "f")) != -1) {
    switch (arg) {
    case 'f':
      firmware = true;

      if (optind < argc) {
        num_entries = strtoul(argv[optind], NULL, 0);
      } else {
        usage();
        return 1;
      }
      break;
    case '?':
      if (isprint(optopt)) {
        std::fprintf(stderr, "Unknown option `-%c'.\n", optopt);
      } else {
        std::fprintf(stderr, "Unknown option character 0x%x.\n", optopt);
      }

      usage();
      return 1;
    default:
      abort();
    }
  }

  if (argc < 2) {
    usage();
    return 1;
  }

  if (firmware) {
    try {
      dump_firmware_tags(tag_filename, num_entries);
    } catch (const std::exception& e) {
      std::fprintf(stderr, "failed to dump firmware tags: %s\n", e.what());
      return 1;
    }
  } else {
    try {
      dump_tags(tag_filename);
    } catch (const std::exception& e) {
      std::fprintf(stderr, "failed to dump tags: %s\n", e.what());
    }
  }

  return 0;
}
