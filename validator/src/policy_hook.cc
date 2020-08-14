#include <string>
#include <cstdlib>
#include <map>

#include "policy_hook.h"

std::map<std::string, uint64_t> hooks;

// Insert a name-address pair for the hook
void policy_hook_insert(const char *name, uint64_t address) {
  std::string _name(name);

  hooks.insert({ _name, address });
}

uint64_t policy_hook_get_address(const char *name) {
  std::string _name(name);
  return hooks[_name];
}
