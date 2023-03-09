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

#ifndef ULEB_H
#define ULEB_H

#include <cstdint>
#include <cstring>
#include <fstream>
#include <string>

namespace policy_engine {

class uleb_reader_t {
private:
  std::ifstream is;
  std::streamsize size;

public:
  uleb_reader_t(const std::string& fname, std::ios::openmode mode=std::ios::binary) : is(std::ifstream(fname, mode)) {
    is.ignore(std::numeric_limits<std::streamsize>::max());
    size = is.gcount();
    is.clear();
    is.seekg(0, std::ios::beg);
  }

  template<class T=uint8_t> std::streamsize read(T& data, std::streamsize n=1) {
    try {
      std::streamsize b = n*sizeof(T)/sizeof(std::ofstream::char_type);
      std::ofstream::char_type bytes[b];
      std::streamsize r = is.read(bytes, b).gcount();
      if (r == b)
        std::memcpy(&data, bytes, b);
      return r*sizeof(std::ofstream::char_type)/sizeof(T);
    } catch (const std::ios::failure& e) {
      return 0;
    }
  }

  template<class T> std::streamsize read_uleb(T& value) {
    value = 0;
    int shift = 0;
    uint8_t b;
    do {
      if (read(b) != 1)
        return 0;
      value |= (((static_cast<T>(b)) & 0x7f) << shift);
      shift += 7;
    } while (b & 0x80);
    return shift/7;
  }

  std::streamsize length() { return size; }
  bool eof() { return is.tellg() >= size; }

  explicit operator bool() const { return static_cast<bool>(is); }
};

class uleb_writer_t {
private:
  std::ofstream os;

public:
  uleb_writer_t(const std::string& fname, std::ios::openmode mode=std::ios::binary) : os(std::ofstream(fname, mode)) {}

  template<class T=uint8_t> bool write(const T* data, std::size_t n=1) {
    try {
      os.write(reinterpret_cast<const std::ofstream::char_type*>(data), n*sizeof(T)/sizeof(std::ofstream::char_type));
      return !os.fail();
    } catch (const std::ios::failure& e) {
      return false;
    }
  }

  template<class T> bool write_uleb(T value) {
    do {
      uint8_t b = value & 0x7f;
      value >>= 7;
      if (value)
        b |= 0x80;
      if (!write(&b))
        return false;
    } while (value != 0);
    return true;
  }

  explicit operator bool() const { return static_cast<bool>(os); }
};

} // namespace policy_engine

#endif // ULEB_H