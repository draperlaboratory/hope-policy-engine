#include <stdio.h>

#include "meta_set_factory.h"

void dump_meta(meta_tree_t *tree, std::string path) {
//  std::vector<std::string> md = tree->find_metadata(meta_set_factory_t::split_dotted_name(path));
  std::vector<std::string> md;
  if (!tree->find_metadata(meta_set_factory_t::split_dotted_name(path), md)) {
    printf("path %s not found\n", path.c_str());
  } else {
    printf("%s: ", path.c_str());
    for (auto s: md)
      printf("%s ", s.c_str());
    printf("\n");
  }
}

int main() {
  YAML::Node n = YAML::LoadFile("/tmp/policy_new/policy_group.yml");
  n = n["Groups"];
  for (YAML::const_iterator it = n.begin(); it != n.end(); ++it) {
    std::string key = it->first.as<std::string>();       // <- key
    printf("key = %s\n", key.c_str());
    YAML::Node node = it->second;
//    assert(node.IsSequence());

    // iterate over names
    for (size_t i=0;i<node.size();i++) {
      std::string name = node[i].as<std::string>();
      printf("name = %s\n", name.c_str());
    }
  }
  
#if 0
  YAML::Node reqsAST = YAML::LoadFile("/tmp/policy_new/policy_init.yml");
  meta_tree_t tree;
  tree.populate(reqsAST);
  try {
  dump_meta(&tree, "requires.dover.SOC.IO.UART0");
  dump_meta(&tree, "requires.dover.riscv.Mach.Reg");
  dump_meta(&tree, "requires.dover.riscv.Mach.RegZero");
  dump_meta(&tree, "requires.dover.SOC.CSR.Default");
  dump_meta(&tree, "requires.dover.riscv.Mach.PC");
  dump_meta(&tree, "requires.dover.Kernel.Code.ElfSection.SHF_EXECINSTR");
  } catch (const char *s) {
    printf("error: %s\n", s);
  }
#endif
#if 0
  meta_set_cache_t cache;
  meta_set_factory_t ms_factory(&cache);
  meta_set_t *ms = ms_factory.get_meta_set("dover.SOC.IO.UART0");
  printf("1\n");
//  ms = ms_factory.get_meta_set("dover.Kernel.Code.ElfSection.SHF_EXECINSTR");
  printf("2\n");
  ms = ms_factory.get_meta_set("dover.riscv.Mach.Reg");
  printf("3\n");
  ms = ms_factory.get_meta_set("dover.riscv.Mach.RegZero");
  printf("4\n");
  ms = ms_factory.get_meta_set("dover.SOC.CSR.default");
  printf("5\n");
  ms = ms_factory.get_meta_set("dover.riscv.Mach.PC");
  printf("ms: ");
  for (int i = 0; i < META_SET_WORDS; i++)
    printf("0x%x ", ms->tags[1]);
  printf("\n");
#endif
  return 0;
}
