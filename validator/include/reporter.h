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

#ifndef REPORTER_H
#define REPORTER_H

#include <cstdio>
#include <string>

namespace policy_engine {

/**
   Generic reporter class, allowing error/informational messages to be channeled.
*/
class reporter_t {
private:
  std::FILE* err;
  std::FILE* warn;
  std::FILE* out;

public:
  int errors;
  int warnings;

  reporter_t(std::FILE* err=stderr, std::FILE* warn=stderr, std::FILE* out=stdout) : errors(0), warnings(0), err(err), warn(warn), out(out) {}

  template<class... A>
  void error(const char* fmt, const A&... args) {
    std::fprintf(err, "error: ");
    std::fprintf(err, fmt, args...);
    errors++;
  }

  template<class... A>
  void warning(const char* fmt, const A&... args) {
    std::fprintf(warn, "warning: ");
    std::fprintf(warn, fmt, args...);
    warnings++;
  }

  template<class... A> void info(const char* fmt, const A&... args) { std::fprintf(out, fmt, args...); }
};

} // namespace policy_engine

#endif
