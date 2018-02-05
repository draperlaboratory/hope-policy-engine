cmake_minimum_required(VERSION 3.5)
project(renode_pipe)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_C_COMPILER gcc)
set (CMAKE_CXX_COMPILER g++)

find_package( Boost REQUIRED COMPONENTS program_options )
include_directories( ${Boost_INCLUDE_DIRS} )
include_directories("./include/policy-glue")
include_directories("./policy")
include_directories(".")


add_library(rv32-renode-validator SHARED
	riscv/main.cc riscv/rv32_validator.cc
	riscv/inst_decoder.c
	policy/policy_meta_set.c policy/policy_rule.c
	)

target_link_libraries(rv32-renode-validator yaml-cpp)
