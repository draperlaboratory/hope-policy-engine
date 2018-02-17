cmake_minimum_required(VERSION 3.5)
project(renode_pipe)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_C_COMPILER gcc)
set (CMAKE_CXX_COMPILER g++)

find_package( Boost REQUIRED COMPONENTS program_options )
include_directories( ${Boost_INCLUDE_DIRS} )
include_directories("./validator/riscv")
include_directories("./validator/include")
include_directories("./include/policy-glue")
include_directories("./policy/include")
include_directories("./riscv")
include_directories("./tagging_tools")
include_directories(".")

#add_executable(meta_test
#	meta_test.cc meta_set_factory.cc metadata_factory.cc soc_tag_configuration.cc
#	riscv/debug.cc
#	policy/src/policy_meta_set.c policy/src/policy_utils.c
#	)
#target_link_libraries(meta_test yaml-cpp)

add_library(validator
  validator/src/metadata_factory.cc
  validator/src/tag_based_validator.cc
  validator/src/soc_tag_configuration.cc
  )
set_property(TARGET validator PROPERTY POSITION_INDEPENDENT_CODE ON)

add_library(rv32_validator
  validator/riscv/debug.cc
  validator/riscv/inst_decoder.c
  validator/riscv/rv32_validator.cc
  validator/riscv/meta_set_factory.cc
  
  # I would prefer to put these in a policy library build, but there are some
  # circular dependency issues that are being a pain.
  policy/src/policy_meta_set.c policy/src/policy_rule.c policy/src/policy_utils.c
  )
set_property(TARGET rv32_validator PROPERTY POSITION_INDEPENDENT_CODE ON)

add_library(tagging_tools
  tagging_tools/tag_file.cc tagging_tools/metadata_memory_map.cc
  )
set_property(TARGET tagging_tools PROPERTY POSITION_INDEPENDENT_CODE ON)

add_executable(md_range
	tagging_tools/md_range.cc
	)
target_link_libraries(md_range tagging_tools validator yaml-cpp)

add_executable(md_code
	tagging_tools/md_code.cc
	validator/riscv/inst_decoder.c
	)
target_link_libraries(md_code tagging_tools validator yaml-cpp)

add_executable(standalone
	validator/riscv/standalone.cc
	)
target_link_libraries(standalone rv32_validator tagging_tools validator yaml-cpp)

add_library(rv32-renode-validator SHARED
	validator/riscv/main.cc
	)
target_link_libraries(rv32-renode-validator rv32_validator tagging_tools validator yaml-cpp)
