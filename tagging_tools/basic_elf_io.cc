#include <cstdarg>
#include <cstdio>
#include "basic_elf_io.h"
#include "file_stream.h"

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

bool FILE_reader_t::seek(std::size_t where, whence_t whence) {
  int w;
  switch (whence) {
    case CUR: w = SEEK_CUR; break;
    case SET: w = SEEK_SET; break;
  }
  return fseek(fp, where, w) == 0;
}

} // namespace policy_engine