cmake_minimum_required(VERSION 3.5)
project(policy-engine)

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_C_STANDARD 99)
set (CMAKE_C_COMPILER gcc)
set (CMAKE_CXX_COMPILER g++)

find_package( Boost REQUIRED COMPONENTS program_options )
include_directories( ${Boost_INCLUDE_DIRS} )

#find_package(gflags, REQUIRED)

include_directories("./validator/include")
include_directories("./tagging_tools")

add_library(validator
  validator/src/metadata_factory.cc
  validator/src/tag_based_validator.cc
  validator/src/soc_tag_configuration.cc
  )
set_property(TARGET validator PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(validator PRIVATE
  ./policy/include
  ./validator/riscv
  )

add_library(rv32_validator
  validator/riscv/debug.cc
  validator/riscv/inst_decoder.c
  validator/riscv/rv32_validator.cc
  validator/riscv/meta_set_factory.cc
  validator/riscv/fake_riscv.cc
  #pipes
  validator/pipe/ideal_pipe/ideal_pipe.cc
  validator/pipe/finite_pipe/finite_pipe.cc
  validator/pipe/dmhc_pipe/compute_hash.cc
  validator/pipe/dmhc_pipe/dmhc.cc
  validator/pipe/dmhc_pipe/dmhc_pipe.cc
  
  # I would prefer to put these in a policy library build, but there are some
  # circular dependency issues that are being a pain.
  policy/src/policy_meta_set.c policy/src/policy_rule.c policy/src/policy_utils.c
  )
set_property(TARGET rv32_validator PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(rv32_validator PRIVATE
  ./policy/include
  ./validator/riscv
  ./validator/include/policy-glue
  #pipe headers
  ./validator/pipe
  ./validator/pipe/ideal_pipe
  ./validator/pipe/finite_pipe
  ./validator/pipe/dmhc_pipe
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

add_executable(md_range
	tagging_tools/md_range.cc
	)
target_link_libraries(md_range tagging_tools validator yaml-cpp)

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

add_executable(md_entity
	tagging_tools/md_entity.cc
	)
target_link_libraries(md_entity tagging_tools validator yaml-cpp gflags pthread)
target_include_directories(md_entity PRIVATE
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

add_executable(standalone
	validator/riscv/standalone.cc
	)
target_link_libraries(standalone rv32_validator tagging_tools validator yaml-cpp)
target_include_directories(standalone PRIVATE
  ./policy/include
  ./validator/riscv
  ./validator/include/policy-glue
  )

add_library(rv32-renode-validator SHARED
	validator/riscv/main.cc
	)
target_link_libraries(rv32-renode-validator rv32_validator tagging_tools validator yaml-cpp)
target_include_directories(rv32-renode-validator PRIVATE
  ./policy/include
  ./validator/riscv
  ./validator/include/policy-glue
  )

#addition
option(ENABLE_PIPE "Enable PIPE" ON)
IF (ENABLE_PIPE)
	target_compile_definitions(rv32_validator PRIVATE ENABLE_PIPE=1)
ENDIF (ENABLE_PIPE)

#option(ENABLE_IDEAL_PIPE "Enable ideal PIPE" ON)
#IF (ENABLE_IDEAL_PIPE)
#	target_compile_definitions(rv32_validator PRIVATE ENABLE_IDEAL_PIPE=1)
#ENDIF (ENABLE_IDEAL_PIPE)

option(ENABLE_FINITE_PIPE "Enable finite PIPE" ON)
IF (ENABLE_FINITE_PIPE)
	target_compile_definitions(rv32_validator PRIVATE ENABLE_FINITE_PIPE=1 CAPACITY=4)
ENDIF (ENABLE_FINITE_PIPE)

#option(ENABLE_DMHC_PIPE "Enable dmhc PIPE" ON)
#IF (ENABLE_DMHC_PIPE)
#	target_compile_definitions(rv32_validator PRIVATE ENABLE_DMHC_PIPE=1 CAPACITY=1024 IWIDTH=32 OWIDTH=32 K_DMHC=4 NO_EVICT=0)
#ENDIF (ENABLE_DMHC_PIPE)
