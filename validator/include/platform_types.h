/*
 * Copyright © 2017-2018 Dover Microsystems, Inc.
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

#ifndef PLATFORM_TYPES_H
#define PLATFORM_TYPES_H

#include <cstdint>

#ifdef RV64_VALIDATOR
#define ADDRESS_T_MAX UINT64_MAX
#else
#define ADDRESS_T_MAX UINT32_MAX
#endif


namespace policy_engine {

#ifdef RV64_VALIDATOR
typedef uint64_t address_t;
typedef uint64_t reg_t;

#define PRIaddr_pad "016lx"
#define PRIaddr "lx"
#define PRIreg  "lx"

#define READER_MASK 0xFFFFFFFFFFFFFFFFull
#else
typedef uint32_t address_t;
typedef uint32_t reg_t;

#define PRIaddr_pad "08x"
#define PRIaddr "x"
#define PRIreg  "x"

#define READER_MASK 0x00000000FFFFFFFFull
#endif
typedef uint32_t insn_bits_t;

#define PLATFORM_WORD_SIZE 4

}

#endif
