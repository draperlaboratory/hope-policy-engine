#include <stdio.h>

//#include "policy_utils.h"

#include "tag_file.h"
#include "metadata_memory_map.h"
#include "metadata_cache.h"
#include "metadata_factory.h"
//#include "meta_set_factory.h"
#include "validator_exception.h"

//meta_set_cache_t ms_cache;
//meta_set_factory_t *ms_factory;
metadata_cache_t md_cache;
metadata_factory_t *md_factory;

extern void init_metadata_renderer(metadata_factory_t *md_factory);

void init() {
  try {
    md_factory = new metadata_factory_t(getenv("GENERATED_POLICY_DIR"));
    init_metadata_renderer(md_factory);
  } catch (validator::exception_t &e) {
    printf("exception: %s\n", e.what().c_str());
  }
}

bool apply_tag(metadata_memory_map_t *map, address_t start, address_t end, const char *tag_name) {
  metadata_t const *md = md_factory->lookup_metadata(tag_name);
//  meta_set_t *ms = ms_factory->get_meta_set(tag_name);
//  meta_set_t *ms = ms_factory->get_meta_set("requires.dover.Kernel.Code.ElfSection.SHF_EXECINSTR");
  if (!md) {
//    printf("tag %s not found\n", tag_name);
    return false;
  }
  map->add_range(start, end, md);
  return true;
}

#include <fstream>
#include <sstream>
#include <string>
bool load_range_file(metadata_memory_map_t *map, std::string file_name) {
  int lineno = 1;
  bool res = true;
  try {
    std::ifstream infile(file_name);
    std::string line;
    while (std::getline(infile, line)) {
      std::istringstream iss(line);
//      printf("Line = '%s'\n", line.c_str());
      std::vector<std::string> tokens {std::istream_iterator<std::string>{iss},
	  std::istream_iterator<std::string>{}};
      if (tokens.size() != 3) {
	fprintf(stderr, "%s: %d: bad format - wrong number of items\n", file_name.c_str(), lineno);
	res = false;
      } else {
	address_t start;
	address_t end;
	start = strtol(tokens[0].c_str(), 0, 16);
	end = strtol(tokens[1].c_str(), 0, 16);
//	printf("applying tag to 0x%x, 0x%x ... ", start, end);
	if (!apply_tag(map, start, end, tokens[2].c_str())) {
	  fprintf(stderr, "%s: %d: could not find tag %s\n", file_name.c_str(), lineno, tokens[2].c_str());
	  res = false;
	} else {
//	  printf("done\n");
	}
      }
//      for (auto s: tokens)
//	printf("'%s' ", s.c_str());
//      printf("\n");
      lineno++;
    }
  } catch (...) {
    fprintf(stderr, "error loading %s\n", file_name.c_str());
    return false;
  }
  return res;
}

void usage() {
  printf("usage: tag_range <base_address> <range_file> <tag_file>\n");
}

int main(int argc, char **argv) {
  address_t base_address;
  if (argc != 4) {
    usage();
    return 0;
  }
  base_address = strtol(argv[1], 0, 16);
  init();
  metadata_memory_map_t map(base_address, &md_cache);
  if (!load_range_file(&map, argv[2]))
    return 1;
  if (!save_tags(&map, argv[3])) {
    printf("failed write of tag file\n");
    return 1;
  }
#if 0
  apply_tag(&map, 0x80000000, 0x80010000, "requires.dover.Kernel.Code.ElfSection.SHF_EXECINSTR");
  for (auto &e: map) {
    printf("(0x%x, 0x%x): ", e.first.start, e.first.end);
    print_meta_set(e.second);
  }
  apply_tag(&map, 0x80000000, 0x80020000, "requires.dover.Kernel.Code.ElfSection.SHF_WRITE");
  for (auto &e: map) {
    printf("(0x%x, 0x%x): ", e.first.start, e.first.end);
    print_meta_set(e.second);
  }
  if (!save_tags(&map, "/tmp/foo.tags"))
    printf("failed write\n");
  metadata_memory_map_t map2(0x80000000, &ms_cache);
  printf("reading...\n");
  if (!load_tags(&map2, "/tmp/foo.tags"))
    printf("failed read\n");
  for (auto &e: map2) {
    printf("(0x%x, 0x%x): ", e.first.start, e.first.end);
    print_meta_set(e.second);
  }
  printf("loading\n");
  load_range_file(&map2, "/tmp/ranges.txt");
#endif
  return 0;
}
