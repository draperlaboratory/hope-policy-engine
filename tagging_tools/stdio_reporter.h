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

#ifndef __STDIO_REPORTER_H__
#define __STDIO_REPORTER_H__

#include <cstdio>
#include <cstdarg>
#include "reporter.h"

namespace policy_engine {

/*
 * Herein are some utility classes that implement the basic APIs that are needed by our ELF loader
 * to load ELF files for utilities.  These implementations go to FILE * objects for I/O.
 */

/**
 * Basic reporter class that just talks to STDIO.
 */
class stdio_reporter_t : public reporter_t {
public:
  stdio_reporter_t() : reporter_t(0, 0) {}

  void error(const char *fmt, ...);
  void warning(const char *fmt, ...);
  void info(const char *fmt, ...);
};

} // namespace policy_engine

#endif // __STDIO_REPORTER_H__