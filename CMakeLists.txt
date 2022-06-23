cmake_minimum_required(VERSION 3.5)
project(policy-engine)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_C_STANDARD 99)
set (CMAKE_C_COMPILER gcc)
set (CMAKE_CXX_COMPILER g++)

find_package( Boost REQUIRED COMPONENTS program_options )
include_directories( ${Boost_INCLUDE_DIRS} )

# debug flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -O0")

include_directories("./validator/include")
include_directories("./tagging_tools")

add_library(tagging_tools
  tagging_tools/annotate.cc
  tagging_tools/elf_loader.cc
  tagging_tools/embed.cc
  tagging_tools/entity_binding.cc
  tagging_tools/llvm_metadata_tagger.cc
  tagging_tools/metadata_memory_map.cc
  tagging_tools/range_map.cc
  tagging_tools/symbol_table.cc
  tagging_tools/tag_elf_file.cc
  tagging_tools/tag_file.cc
  validator/riscv/inst_decoder.cc
  )
set_property(TARGET tagging_tools PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(tagging_tools PRIVATE
  ./policy/include
  ./validator/riscv
  ./validator/include/policy-glue
  )

add_library(validator
  validator/src/metadata_factory.cc
  validator/src/opgroup_rule.cc
  validator/src/soc_tag_configuration.cc
  )
set_property(TARGET validator PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(validator tagging_tools)
target_include_directories(validator PRIVATE
  ./policy/include
  ./validator/riscv
  ./validator/include/policy-glue
  )

add_library(rv_validator
  validator/riscv/debug.cc
  validator/riscv/inst_decoder.cc
  validator/riscv/rv_validator.cc
  validator/riscv/meta_set_factory.cc
  validator/rule_cache/ideal_rule_cache/ideal_rule_cache.cc
  validator/rule_cache/finite_rule_cache/finite_rule_cache.cc
  validator/rule_cache/dmhc_rule_cache/compute_hash.cc
  validator/rule_cache/dmhc_rule_cache/dmhc.cc
  validator/rule_cache/dmhc_rule_cache/dmhc_rule_cache.cc
  
  # I would prefer to put these in a policy library build, but there are some
  # circular dependency issues that are being a pain.
  policy/src/policy_meta_set.c policy/src/policy_rule.c policy/src/policy_utils.c
  )
set_property(TARGET rv_validator PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(rv_validator PRIVATE
  ./policy/include
  ./validator/riscv
  ./validator/include/policy-glue
  ./validator/rule_cache
  ./validator/rule_cache/ideal_rule_cache
  ./validator/rule_cache/finite_rule_cache
  ./validator/rule_cache/dmhc_rule_cache
  )

add_executable(gen_tag_info
  tagging_tools/gen_tag_info.cc
  )
target_link_libraries(gen_tag_info tagging_tools validator rv_validator yaml-cpp gflags elf)
target_include_directories(gen_tag_info PRIVATE
  ./policy/include
  ./validator/include
  ./validator/include/policy-glue
  ./validator/riscv
  )

add_library(rv32-sim-validator SHARED
	validator/riscv/main.cc
	)
target_link_libraries(rv32-sim-validator rv_validator tagging_tools validator yaml-cpp)
target_include_directories(rv32-sim-validator PRIVATE
  ./policy/include
  ./validator/riscv
  ./validator/include/policy-glue
  ./validator/rule_cache
  ./validator/rule_cache/ideal_rule_cache
  ./validator/rule_cache/finite_rule_cache
  ./validator/rule_cache/dmhc_rule_cache
  )

add_library(rv64-sim-validator SHARED
	validator/riscv/main.cc
	)
set_target_properties (rv64-sim-validator PROPERTIES COMPILE_FLAGS -DRV64_VALIDATOR)
target_link_libraries(rv64-sim-validator rv_validator tagging_tools validator yaml-cpp)
target_include_directories(rv64-sim-validator PRIVATE
  ./policy/include
  ./validator/riscv
  ./validator/include/policy-glue
  ./validator/rule_cache
  ./validator/rule_cache/ideal_rule_cache
  ./validator/rule_cache/finite_rule_cache
  ./validator/rule_cache/dmhc_rule_cache
  )
