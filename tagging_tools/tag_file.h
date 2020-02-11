/*
 * Copyright © 2017-2018 Dover Microsystems, Inc.
 * All rights reserved. 
 *
 * Use and disclosure subject to the following license. 
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef TAG_FILE_H
#define TAG_FILE_H

#include <string>
#include <map>
#include <vector>
#include "metadata_memory_map.h"
#include "metadata_register_map.h"
#include "metadata_index_map.h"
#include "elf_utils.h"

namespace policy_engine {

bool load_tags(metadata_memory_map_t *map, std::string file_name);
bool save_tags(metadata_memory_map_t *map, std::string file_name);
bool save_tag_indexes(std::vector<const metadata_t *> &metadata_values,
                      metadata_index_map_t<metadata_memory_map_t, range_t> &memory_index_map,
                      metadata_index_map_t<metadata_register_map_t, std::string> &register_index_map,
                      metadata_index_map_t<metadata_register_map_t, std::string> &csr_index_map,
                      int32_t register_default, int32_t csr_default,
                      std::string file_name);
bool write_headers(std::list<range_t> &code_ranges,
                   std::list<range_t> &data_ranges,
                   bool is_64_bit, std::string tag_filename);
bool load_firmware_tag_file(std::list<range_t> &code_ranges,
                            std::list<range_t> &data_ranges,
                            std::vector<const metadata_t *> &metadata_values,
                            metadata_index_map_t<metadata_memory_map_t, range_t> &metadata_index_map,
                            metadata_index_map_t<metadata_register_map_t, std::string> &register_index_map,
                            metadata_index_map_t<metadata_register_map_t, std::string> &csr_index_map,
                            int32_t &register_default, int32_t &csr_default,
                            std::string file_name);
 
  // An arg_val_map maps each address to a vector of arg values stored there
 typedef std::map<uint32_t, std::vector<uint32_t>*> arg_val_map_t;
 
 arg_val_map_t * load_tag_args(metadata_memory_map_t *map, std::string file_name);
  
} // namespace policy_engine

#endif
