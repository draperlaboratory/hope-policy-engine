cmake_minimum_required(VERSION 3.5)
project(renode_pipe)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_C_COMPILER gcc)
set (CMAKE_CXX_COMPILER g++)

find_package( Boost REQUIRED COMPONENTS program_options )
include_directories( ${Boost_INCLUDE_DIRS} )
include_directories("./include/policy-glue")
include_directories("./policy/include")
include_directories("./riscv")
include_directories("./tagging_tools")
include_directories(".")

add_executable(meta_test
	meta_test.cc meta_set_factory.cc soc_tag_configuration.cc
	riscv/debug.cc
	policy/pex/policy_meta_set.c policy/pex/policy_utils.c
	)
target_link_libraries(meta_test yaml-cpp)

add_executable(tag_range
	 meta_set_factory.cc
	tagging_tools/tag_range.cc tagging_tools/tag_file.cc
	riscv/debug.cc
	policy/pex/policy_meta_set.c policy/pex/policy_utils.c
	)
target_link_libraries(tag_range yaml-cpp)

add_executable(standalone
	standalone.cc meta_set_factory.cc tag_based_validator.cc soc_tag_configuration.cc
	riscv/rv32_validator.cc riscv/debug.cc
	riscv/inst_decoder.c
	tagging_tools/tag_file.cc
	policy/pex/policy_meta_set.c policy/pex/policy_rule.c policy/pex/policy_utils.c
	)
target_link_libraries(standalone yaml-cpp)

add_library(rv32-renode-validator SHARED
	meta_set_factory.cc tag_based_validator.cc soc_tag_configuration.cc
	riscv/main.cc riscv/rv32_validator.cc riscv/debug.cc
	riscv/inst_decoder.c
	tagging_tools/tag_file.cc
	policy/pex/policy_meta_set.c policy/pex/policy_rule.c policy/pex/policy_utils.c
	)

target_link_libraries(rv32-renode-validator yaml-cpp)
