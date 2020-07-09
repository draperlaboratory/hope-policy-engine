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

#include <stdio.h>
#include <unistd.h>

#include "basic_elf_io.h"
#include "tag_file.h"
#include "uleb.h"
#include "metadata_index_map.h"
#include "metadata_register_map.h"

using namespace policy_engine;

#include <fstream>
#include <sstream>
#include <string>
#include <cstring>

struct file_reader_t {
  FILE *fp;
  file_reader_t(FILE *fp) : fp(fp) { }
  bool read_byte(uint8_t &b) {
    return fread(&b, 1, 1, fp) == 1;
  }
};

void usage() {
  printf("usage: dump_tags <tag_file> <-f <num_entries>>\n");
  printf("\t-f firmware tag format\n");
}

bool dump_firmware_tags(const char* tag_filename, size_t num_entries) {
  stdio_reporter_t err;
  std::list<range_t> code_ranges;
  std::list<range_t> data_ranges;
  std::vector<const metadata_t *> metadata_values;
  int32_t register_default;
  int32_t csr_default;

  auto memory_index_map = metadata_index_map_t<metadata_memory_map_t, range_t>();
  auto register_index_map = metadata_index_map_t<metadata_register_map_t, std::string>();
  auto csr_index_map = metadata_index_map_t<metadata_register_map_t, std::string>();

  if(load_firmware_tag_file(code_ranges, data_ranges, metadata_values,
                            memory_index_map, register_index_map, csr_index_map,
                            register_default, csr_default, std::string(tag_filename)) == false) {
    err.error("Failed to load firmware tag file\n");
    return false;
  }

  printf("Code ranges:\n");
  for(auto &r : code_ranges) {
    printf("{ 0x%" PRIaddr_pad " - 0x%" PRIaddr_pad " }\n", r.start, r.end);
  }

  printf("\nData ranges:\n");
  for(auto &r : data_ranges) {
    printf("{ 0x%" PRIaddr_pad " - 0x%" PRIaddr_pad " }\n", r.start, r.end);
  }

  printf("\nMetadata values:\n");
  for(size_t i = 0; i < metadata_values.size(); i++) {
    printf("%lu: { ", i);
    for(const auto &m : *metadata_values[i]) {
      printf("%lx ", m);
    }
    printf("}\n");
  }

  printf("\nRegister tag entries:\n");
  printf("Default: %x\n", register_default);
  for(auto &it : register_index_map) {
    printf("%s: %x\n", it.first.c_str(), it.second);
  }

  printf("\nCSR tag entries:\n");
  printf("Default: %x\n", csr_default);
  for(auto &it : csr_index_map) {
    printf("%s: %x\n", it.first.c_str(), it.second);
  }

  printf("\nMemory tag entries (showing %lu of %lu):\n",
      num_entries, memory_index_map.size());
  size_t entry_index = 0;
  for(auto &it : memory_index_map) {
    printf("{ 0x%" PRIaddr_pad " - 0x%" PRIaddr_pad " }: %x\n", it.first.start, it.first.end, it.second);

    entry_index++;
    if(entry_index == num_entries) {
      break;
    }
  }

  return true;
}

bool dump_tags(std::string file_name) {
  FILE *fp = fopen(file_name.c_str(), "rb");
  int i = 0;

  if (!fp)
    return false;

  file_reader_t reader(fp);
  fseek(fp, 0, SEEK_END);
  size_t eof_point = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  while (eof_point != ftell(fp)) {
    address_t start;
    address_t end;
    uint32_t metadata_count;

    if (!read_uleb<file_reader_t, address_t>(&reader, start)) {
      fclose(fp);
      return false;
    }

    if (!read_uleb<file_reader_t, address_t>(&reader, end)) {
      fclose(fp);
      return false;
    }

    if (!read_uleb<file_reader_t, uint32_t>(&reader, metadata_count)) {
      fclose(fp);
      return false;
    }

    if (end < start)
    {
       fprintf(stderr, "Entry %d, Start (0x%" PRIaddr_pad ")is after End (0x%"
               PRIaddr_pad ")\n", i, start, end);
       fprintf(stderr, "Are you sure this is a simulation tag file?\n");
       fprintf(stderr, "Or did you want to set the -f (firmware tag file) option\n");
       return false;
    }
    else if ((int32_t)metadata_count < 0)
    {
       fprintf(stderr, "Entry %d, has negative entries (%d)\n", i, metadata_count);
       fprintf(stderr, "Are you sure this is a simulation tag file?\n");
       fprintf(stderr, "Or did you want to set the -f (firmware tag file) option\n");
       return false;
    }

    printf("Entry %d, 0x%" PRIaddr_pad " - 0x%" PRIaddr_pad
           " (%d)\n", i++, start, end, metadata_count);

    printf("\tMetadata List:  ");

    for (uint32_t i = 0; i < metadata_count; i++) {
      meta_t meta;

      if (!read_uleb<file_reader_t, meta_t>(&reader, meta)) {
         fclose(fp);
         return false;
      }

      printf("%016lx, ", meta);
    }

    printf("end.\n");
  }

  fclose(fp);

  return true;
}

int main(int argc, char **argv) {
  stdio_reporter_t err;
  const char *tag_filename;
  char arg;
  int entries_arg = 0;
  bool firmware = false;
  size_t num_entries = 16;

  // Retrieve memory metadata from tag file
  tag_filename = argv[1];

  while ((arg = getopt (argc, argv, "f")) != -1)
    switch (arg)
      {
      case 'f':
        firmware = true;

        if(optind < argc) {
           num_entries = strtoul(argv[optind], NULL, 0);
        } else {
          usage();
          return 1;
        }
        break;
      case '?':
        if (isprint(optopt)) {
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        }
        else {
          fprintf (stderr, "Unknown option character 0x%x.\n", optopt);
        }

        usage();
        return 1;
        break;
      default:
        abort ();
      }

  if(argc < 2) {
    usage();
    return 1;
  }

  if (firmware) {
     if(dump_firmware_tags(tag_filename, num_entries) == false) {
      err.error("Failed to dump firmware tags\n");
      return 1;
    }
  } else {
    if(dump_tags(tag_filename) == false) {
      err.error("Failed to dump tags\n");
      return 1;
    }
  }

  return 0;
}
