#ifndef __FILE_STREAM_H__
#define __FILE_STREAM_H__

#include <cstdio>

namespace policy_engine {

struct file_stream_t {
  enum whence_t { CUR, SET };
  virtual bool read(void* buff, std::size_t size) = 0;
  virtual bool seek(std::size_t where, whence_t whence) = 0;
};

}

#endif // __FILE_STREAM_H__