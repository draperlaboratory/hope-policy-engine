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
#include <stdlib.h>
#include "tag_file.h"
#include "elf_utils.h"
#include "basic_elf_io.h"
#include "metadata_index_map.h"
#include "metadata_factory.h"
#include "metadata_register_map.h"

using namespace policy_engine;

#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <unistd.h>

#ifdef RV64_VALIDATOR
        char* bfd_target = "elf64-littleriscv";
#else
        char* bfd_target = "elf32-littleriscv";
#endif
char* riscv_prefix = "riscv64-unknown-elf-";

void usage(char** argv) {
    printf("usage: %s <tag_file> <policy_dir> <elf to embed tags>\n", argv[0]);
}


bool save_tags_to_temp(std::vector<const metadata_t *> &metadata_values,
        metadata_index_map_t<metadata_memory_map_t, range_t> &memory_index_map,
        std::string old_elf_name, std::string new_elf_name, char tempfile[]) {
    FILE *elf_in;
    stdio_reporter_t err;
    elf_in = fopen(old_elf_name.c_str(), "rb");
    FILE_reader_t reader(elf_in);
    elf_image_t img(&reader, &err);
    img.load();
    int fd = mkstemp(tempfile);
    FILE *section_file = fdopen(fd, "wb");

    if (!elf_in)
        return false;

    size_t mem_map_size = memory_index_map.size();
    fwrite(&mem_map_size, (sizeof(address_t)), 1, section_file);
    int i = 0;
    for (auto &e : memory_index_map) {
        range_t range = e.first;
        address_t metadata_size = (address_t) metadata_values[e.second]->size();

        fwrite(&range.start, (sizeof(address_t)), 1, section_file);
        fwrite(&range.end, (sizeof(address_t)), 1, section_file);
        fwrite(&metadata_size, (sizeof(address_t)), 1, section_file);

        for(const meta_t &m : *metadata_values[e.second]) {
            address_t sorta_m = (address_t) m;
            fwrite(&sorta_m, (sizeof(address_t)), 1, section_file);
        }
    }

    fclose(section_file);
    close(fd);

    return true;
}


bool embed_tags_in_elf(std::vector<const metadata_t *> &metadata_values,
        metadata_index_map_t<metadata_memory_map_t, range_t> &memory_index_map,
        std::string old_elf_name, std::string new_elf_name, bool update) {
    char section_temp_file[] = "/tmp/sectionXXXXXX";
    if (!save_tags_to_temp(metadata_values, memory_index_map, old_elf_name, new_elf_name, section_temp_file)) {
        printf("Failed to save tags\n");
        return false;
    }

    //char command_string[512];
    std::string command_string;
    
    char* base_command;
    if (update)
        base_command = "%sobjcopy --target %s --update-section .initial_tag_map=%s %s %s >/dev/null 2>&1";
    else
        base_command = "%sobjcopy --target %s --add-section .initial_tag_map=%s --set-section-flags .initial_tag_map=readonly,data %s %s >/dev/null 2>&1";

    command_string.append(base_command);
    command_string += riscv_prefix;
    command_string += bfd_target;
    command_string += section_temp_file;
    command_string += old_elf_name;
    command_string += new_elf_name;
    
    //    sprintf(command_string, base_command, riscv_prefix,bfd_target, section_temp_file, old_elf_name.c_str(), new_elf_name.c_str());
//     printf(command_string);
    int ret = system(command_string.c_str());

    if (remove(section_temp_file))
        printf("Failed to delete temporary file %s.\n", section_temp_file);

    return (ret == 0);
}

int main(int argc, char **argv) {
    if(argc < 4) {
        usage(argv);
        return 1;
    }
    stdio_reporter_t err;
    const char *tag_filename;
    const char *elf_filename = argv[3];
    const char *policy_dir;
    metadata_memory_map_t metadata_memory_map;
    std::vector<const metadata_t *> metadata_values;


    // Retrieve memory metadata from tag file
    tag_filename = argv[1];
    if(!load_tags(&metadata_memory_map, tag_filename)) {
        err.error("Failed to load tags\n");
        return 1;
    }

    // Retrieve register metadata from policy
    policy_dir = argv[2];
    metadata_factory_t metadata_factory(policy_dir);

    // Transform (memory/register -> metadata) maps into a metadata list and (memory/register -> index) maps
    auto memory_index_map =
        metadata_index_map_t<metadata_memory_map_t, range_t>(&metadata_memory_map, &metadata_values);

    // Figure out if the section already exists in the elf. This affects the exact command needed to update the elf.
    char base_command[] = "%sobjdump --target %s -d -j .initial_tag_map %s >/dev/null 2>&1";

    // char command_string[256];
    std::string command_string;
    command_string += base_command;
    command_string += riscv_prefix;
    command_string += bfd_target;
    command_string += elf_filename;
    
    //    sprintf(command_string, base_command, riscv_prefix, bfd_target, elf_filename);
    int ret = system(command_string.c_str());

    if(!embed_tags_in_elf(metadata_values, memory_index_map, elf_filename, elf_filename, ret == 0)) {
        err.error("Failed to save indexes to tag file\n");
        return 1;
    }
    return 0;
}

