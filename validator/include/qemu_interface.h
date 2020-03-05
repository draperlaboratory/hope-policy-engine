/*
 * Copyright © 2017-2018 The Charles Stark Draper Laboratory, Inc. and/or Dover Microsystems, Inc.
 * All rights reserved. 
 *
 * Use and disclosure subject to the following license. 
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef QEMU_INTERFACE_H
#define QEMU_INTERFACE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t (*RegisterReader_t)(uint32_t);
typedef uint64_t (*MemoryReader_t)(uint64_t);

#ifdef RV64_VALIDATOR
uint32_t e_v_validate(uint64_t pc, uint32_t instr);
#else
uint32_t e_v_validate(uint32_t pc, uint32_t instr);
#endif

void e_v_set_callbacks(RegisterReader_t reg_reader, MemoryReader_t mem_reader);
uint32_t e_v_commit(void);
void e_v_set_metadata(const char *validator_cfg_path);
void e_v_violation_msg(char *dest, int n);
void e_v_pc_tag(char *dest, int n);
void e_v_csr_tag(char *dest, int n, uint64_t addr);
void e_v_reg_tag(char *dest, int n, uint64_t addr);
void e_v_mem_tag(char *dest, int n, uint64_t addr);
void e_v_set_pc_watch(bool watching);
void e_v_set_reg_watch(uint64_t addr);
void e_v_set_csr_watch(uint64_t addr);
void e_v_set_mem_watch(uint64_t addr);
void e_v_rule_cache_stats(void);


#ifdef __cplusplus
}
#endif


#endif
