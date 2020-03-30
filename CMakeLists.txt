cmake_minimum_required(VERSION 3.5)
project(policy-engine)

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_C_STANDARD 99)
set (CMAKE_C_COMPILER gcc)
set (CMAKE_CXX_COMPILER g++)

find_package( Boost REQUIRED COMPONENTS program_options )
include_directories( ${Boost_INCLUDE_DIRS} )

# debug flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb -O2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -O2")

#find_package(gflags, REQUIRED)

include_directories("./validator/include")
include_directories("./tagging_tools")

add_library(validator
  validator/src/metadata_factory.cc
  validator/src/opgroup_rule.cc
  validator/src/tag_based_validator.cc
  validator/src/soc_tag_configuration.cc
  )
set_property(TARGET validator PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(validator PRIVATE
  ./policy/include
  ./validator/riscv
  ./validator/include/policy-glue
  )

add_library(validator64
  validator/src/metadata_factory.cc
  validator/src/opgroup_rule.cc
  validator/src/tag_based_validator.cc
  validator/src/soc_tag_configuration.cc
  )
set_property(TARGET validator64 PROPERTY POSITION_INDEPENDENT_CODE ON)
set_target_properties(validator64 PROPERTIES COMPILE_FLAGS -DRV64_VALIDATOR)
target_include_directories(validator64 PRIVATE
  ./policy/include
  ./validator/riscv
  ./validator/include/policy-glue
  )

add_library(rv32_validator
  validator/riscv/debug.cc
  validator/riscv/inst_decoder.c
  validator/riscv/rv32_validator.cc
  validator/riscv/meta_set_factory.cc
  validator/riscv/fake_riscv.cc
  validator/rule_cache/ideal_rule_cache/ideal_rule_cache.cc
  validator/rule_cache/finite_rule_cache/finite_rule_cache.cc
  validator/rule_cache/dmhc_rule_cache/compute_hash.cc
  validator/rule_cache/dmhc_rule_cache/dmhc.cc
  validator/rule_cache/dmhc_rule_cache/dmhc_rule_cache.cc
  
  # I would prefer to put these in a policy library build, but there are some
  # circular dependency issues that are being a pain.
  policy/src/policy_meta_set.c policy/src/policy_rule.c policy/src/policy_utils.c
  )
set_property(TARGET rv32_validator PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(rv32_validator PRIVATE
  ./policy/include
  ./validator/riscv
  ./validator/include/policy-glue
  ./validator/rule_cache
  ./validator/rule_cache/ideal_rule_cache
  ./validator/rule_cache/finite_rule_cache
  ./validator/rule_cache/dmhc_rule_cache
  )

add_library(rv64_validator
  validator/riscv/debug.cc
  validator/riscv/inst_decoder.c
  validator/riscv/rv32_validator.cc
  validator/riscv/meta_set_factory.cc
  validator/riscv/fake_riscv.cc
  validator/rule_cache/ideal_rule_cache/ideal_rule_cache.cc
  validator/rule_cache/finite_rule_cache/finite_rule_cache.cc
  validator/rule_cache/dmhc_rule_cache/compute_hash.cc
  validator/rule_cache/dmhc_rule_cache/dmhc.cc
  validator/rule_cache/dmhc_rule_cache/dmhc_rule_cache.cc
  
  # I would prefer to put these in a policy library build, but there are some
  # circular dependency issues that are being a pain.
  policy/src/policy_meta_set.c policy/src/policy_rule.c policy/src/policy_utils.c
  )
set_property(TARGET rv64_validator PROPERTY POSITION_INDEPENDENT_CODE ON)
set_target_properties(rv64_validator PROPERTIES COMPILE_FLAGS -DRV64_VALIDATOR)
target_include_directories(rv64_validator PRIVATE
  ./policy/include
  ./validator/riscv
  ./validator/include/policy-glue
  ./validator/rule_cache
  ./validator/rule_cache/ideal_rule_cache
  ./validator/rule_cache/finite_rule_cache
  ./validator/rule_cache/dmhc_rule_cache
  )

 add_library(tagging_tools
  tagging_tools/tag_file.cc
  tagging_tools/metadata_memory_map.cc
  tagging_tools/asm_annotater.cc
  tagging_tools/elf_loader.cc
  tagging_tools/elf_utils.cc
  tagging_tools/entity_binding.cc
  )
set_property(TARGET tagging_tools PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(tagging_tools PRIVATE
  ./validator/riscv
  )

add_library(tagging_tools64
  tagging_tools/tag_file.cc
  tagging_tools/metadata_memory_map.cc
  tagging_tools/asm_annotater.cc
  tagging_tools/elf_loader.cc
  tagging_tools/elf_utils.cc
  tagging_tools/entity_binding.cc
  )
set_property(TARGET tagging_tools64 PROPERTY POSITION_INDEPENDENT_CODE ON)
set_target_properties(tagging_tools64 PROPERTIES COMPILE_FLAGS -DRV64_VALIDATOR)
target_include_directories(tagging_tools64 PRIVATE
  ./validator/riscv
  )

add_executable(md_header
	tagging_tools/md_header.cc
	)
target_link_libraries(md_header tagging_tools validator yaml-cpp)

add_executable(md_header64
	tagging_tools/md_header.cc
	)
set_target_properties(md_header64 PROPERTIES COMPILE_FLAGS -DRV64_VALIDATOR)
target_link_libraries(md_header64 tagging_tools64 validator64 yaml-cpp)

add_executable(md_index
	tagging_tools/md_index.cc
	)
target_link_libraries(md_index tagging_tools validator yaml-cpp)

add_executable(md_index64
	tagging_tools/md_index.cc
	)
set_target_properties(md_index64 PROPERTIES COMPILE_FLAGS -DRV64_VALIDATOR)
target_link_libraries(md_index64 tagging_tools64 validator64 yaml-cpp)

add_executable(dump_tags
	tagging_tools/dump_tags.cc
	)
target_link_libraries(dump_tags tagging_tools validator yaml-cpp)

add_executable(dump_tags64
	tagging_tools/dump_tags.cc
	)
set_target_properties(dump_tags64 PROPERTIES COMPILE_FLAGS -DRV64_VALIDATOR)
target_link_libraries(dump_tags64 tagging_tools64 validator64 yaml-cpp)

add_executable(md_range
	tagging_tools/md_range.cc
	)
target_link_libraries(md_range tagging_tools validator yaml-cpp)

add_executable(md_range64
	tagging_tools/md_range.cc
	)
set_target_properties(md_range64 PROPERTIES COMPILE_FLAGS -DRV64_VALIDATOR)
target_link_libraries(md_range64 tagging_tools64 validator yaml-cpp)

add_executable(md_code
	tagging_tools/md_code.cc
	validator/riscv/inst_decoder.c
	)
target_link_libraries(md_code tagging_tools validator yaml-cpp)
target_include_directories(md_code PRIVATE
  ./validator/include/policy-glue

  # policy directory dependency is because riscv_isa.h drags it in.
  # There is no actual dependency.
  ./policy/include
  )

add_executable(md_code64
	tagging_tools/md_code.cc
	validator/riscv/inst_decoder.c
	)
target_link_libraries(md_code64 tagging_tools64 validator64 yaml-cpp)
set_target_properties(md_code64 PROPERTIES COMPILE_FLAGS -DRV64_VALIDATOR)
target_include_directories(md_code64 PRIVATE
  ./validator/include/policy-glue

  # policy directory dependency is because riscv_isa.h drags it in.
  # There is no actual dependency.
  ./policy/include
  )

add_executable(md_entity
	tagging_tools/md_entity.cc
	)
target_link_libraries(md_entity tagging_tools validator yaml-cpp gflags pthread)
target_include_directories(md_entity PRIVATE
  ./validator/include/policy-glue
  )

add_executable(md_entity64
	tagging_tools/md_entity.cc
	)
target_link_libraries(md_entity64 tagging_tools64 validator64 yaml-cpp gflags pthread)
set_target_properties(md_entity64 PROPERTIES COMPILE_FLAGS -DRV64_VALIDATOR)
target_include_directories(md_entity64 PRIVATE
  ./validator/include/policy-glue
  )

add_executable(md_asm_ann
	tagging_tools/md_asm_ann.cc
	)
target_link_libraries(md_asm_ann tagging_tools validator yaml-cpp gflags pthread)
target_include_directories(md_asm_ann PRIVATE
  ./validator/include/policy-glue

  # policy directory dependency is because riscv_isa.h drags it in.
  # There is no actual dependency.
  ./policy/include
  )

add_executable(md_asm_ann64
	tagging_tools/md_asm_ann.cc
	)
target_link_libraries(md_asm_ann64 tagging_tools64 validator64 yaml-cpp gflags pthread)
set_target_properties(md_asm_ann64 PROPERTIES COMPILE_FLAGS -DRV64_VALIDATOR)
target_include_directories(md_asm_ann64 PRIVATE
  ./validator/include/policy-glue

  # policy directory dependency is because riscv_isa.h drags it in.
  # There is no actual dependency.
  ./policy/include
  )

add_executable(standalone
	validator/riscv/standalone.cc
	)
target_link_libraries(standalone rv32_validator tagging_tools validator yaml-cpp)
target_include_directories(standalone PRIVATE
  ./policy/include
  ./validator/riscv
  ./validator/include/policy-glue
  ./validator/rule_cache
  ./validator/rule_cache/ideal_rule_cache
  ./validator/rule_cache/finite_rule_cache
  ./validator/rule_cache/dmhc_rule_cache
  )

add_library(rv32-sim-validator SHARED
	validator/riscv/main.cc
	)
target_link_libraries(rv32-sim-validator rv32_validator tagging_tools validator yaml-cpp)
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
target_link_libraries(rv64-sim-validator rv64_validator tagging_tools64 validator64 yaml-cpp)
target_include_directories(rv64-sim-validator PRIVATE
  ./policy/include
  ./validator/riscv
  ./validator/include/policy-glue
  ./validator/rule_cache
  ./validator/rule_cache/ideal_rule_cache
  ./validator/rule_cache/finite_rule_cache
  ./validator/rule_cache/dmhc_rule_cache
  )

add_executable(standalone64
	validator/riscv/standalone.cc
	)
set_target_properties(standalone64 PROPERTIES COMPILE_FLAGS -DRV64_VALIDATOR)
target_link_libraries(standalone64 rv64_validator tagging_tools64 validator64 yaml-cpp)
target_include_directories(standalone64 PRIVATE
  ./policy/include
  ./validator/riscv
  ./validator/include/policy-glue
  ./validator/rule_cache
  ./validator/rule_cache/ideal_rule_cache
  ./validator/rule_cache/finite_rule_cache
  ./validator/rule_cache/dmhc_rule_cache
  )
