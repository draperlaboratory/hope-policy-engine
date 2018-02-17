#include "metadata_memory_map.h"

#include "metadata_factory.h"
static metadata_factory_t *factory;
void init_metadata_renderer(metadata_factory_t *md_factory) {
  factory = md_factory;
}

std::string render_metadata(metadata_t const *metadata) {
  if (factory)
    return factory->render(metadata);
  return "<no renderer>";
}

void metadata_memory_map_t::add_range(address_t start, address_t end, metadata_t const *metadata) {
  int s = (start - base) / stride;
  int e = (end - base) / stride;
  if (e > map.size()) {
    map.resize(e, nullptr);
    end_address = index_to_addr(e);
  }
  while (s < e) {
//      printf("0x%x, 0x%x\n", s, e);
    metadata_t const *md;
    if (map[s]) {
      md = map[s];
//	metadata_t *new_md = new metadata_t(*md | *metadata);
      metadata_t *new_md = new metadata_t(*md);
      new_md->insert(metadata);
      md = md_cache->canonize(new_md);
      if (md != new_md)
	delete new_md;
    } else {
      md = md_cache->canonize(metadata);
    }
//    std::string md_s = render_metadata(md);
//    printf("0x%08x: %s\n", s, md_s.c_str());
    map[s] = md;
    s++;
  }
}
