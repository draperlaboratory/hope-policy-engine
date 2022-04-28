#include <cstdarg>
#include <cstdio>
#include "stdio_reporter.h"

namespace policy_engine {

void stdio_reporter_t::error(const char *fmt, ...) {
  std::fprintf(stderr, "error: ");
  std::va_list vl;
  va_start(vl, fmt);
  std::vfprintf(stderr, fmt, vl);
  va_end(vl);
  errors++;
}

void stdio_reporter_t::warning(const char *fmt, ...) {
  std::fprintf(stderr, "warning: ");
  std::va_list vl;
  va_start(vl, fmt);
  std::vfprintf(stderr, fmt, vl);
  va_end(vl);
  warnings++;
}

void stdio_reporter_t::info(const char *fmt, ...) {
  std::printf("info: ");
  std::va_list vl;
  va_start(vl, fmt);
  std::vprintf(fmt, vl);
  va_end(vl);
}

} // namespace policy_engine