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

class stream_reader_t {
private:
  std::ifstream is;
  std::streamsize size;

public:
  stream_reader_t(const std::string& fname, std::ios::openmode mode=std::ios::binary) : is(std::ifstream(fname, mode)) {
    is.ignore(std::numeric_limits<std::streamsize>::max());
    size = is.gcount();
    is.clear();
    is.seekg(0, std::ios::beg);
  }

  template<class T> std::streamsize read(T* data, std::streamsize n) {
    try {
      std::streamsize b = n*sizeof(T)/sizeof(std::ofstream::char_type);
      std::ofstream::char_type bytes[b];
      std::streamsize r = is.read(bytes, b).gcount();
      if (r == b)
        std::memcpy(data, bytes, b);
      return r*sizeof(std::ofstream::char_type)/sizeof(T);
    } catch (const std::ios::failure& e) {
      return 0;
    }
  }

  bool read_byte(uint8_t& b) { return read(&b, 1) == 1; }

  std::streamsize length() { return size; }
  bool eof() { return is.tellg() >= size; }

  explicit operator bool() const { return static_cast<bool>(is); }
};

class stream_writer_t {
private:
  std::ofstream os;

public:
  stream_writer_t(const std::string& fname, std::ios::openmode mode=std::ios::binary) : os(std::ofstream(fname, mode)) {}

  template<class T> bool write(const T* data, std::size_t n) {
    try {
      os.write(reinterpret_cast<const std::ofstream::char_type*>(data), n*sizeof(T)/sizeof(std::ofstream::char_type));
      return !os.fail();
    } catch (const std::ios::failure& e) {
      return false;
    }
  }

  bool write_byte(uint8_t b) { return write(&b, 1); }

  explicit operator bool() const { return static_cast<bool>(os); }
};

/**
 *  ULEB encoding functions.  These are generics that allow you to specify a reader type and
 *  a value type.
 */

/**
 *  Reads a ULEB encoded unsigned value.  The <code>StreamType</code> type parameter specifies
 *  a type that must support the following API:
 *
 *  bool read_byte(uint8_t &b)
 *  Returns <code>true</code> on success, <code>false</code> on failure.
 *
 *  The ValueType type parameter will be the integral type to be read.  E.g. \a uint32_t.
 *
 *  \return Returns true on success and false on failure.
 */
template <typename StreamType, typename ValueType> bool read_uleb(StreamType& stream, ValueType& v) {
  v = 0;
  int shift = 0;
  uint8_t b;
  do {
    if (!stream.read_byte(b))
      return false;
    v = v | ((((ValueType)b) & 0x7f) << shift);
    shift += 7;
  } while (b & 0x80);
  return true;
}

/**
 *  Writes a ULEB encoded unsigned value.  The <code>StreamType</code> type parameter specifies
 *  a type that must support the following API:
 *
 *  bool write_byte(uint8_t b)
 *  Returns <code>true</code> on success, <code>false</code> on failure.
 *
 *  The ValueType type parameter will be the integral type to be written.  E.g. \a uint32_t.
 *
 *  \return Returns true on success and false on failure.
 */
template <typename StreamType, typename ValueType> bool write_uleb(StreamType& stream, ValueType v) {
  do {
    uint8_t b = v & 0x7f;
    v = v >> 7;
    if (v)
      b |= 0x80;
    if (!stream.write_byte(b))
      return false;
  } while (v != 0);
  return true;
}

} // namespace policy_engine

#endif // ULEB_H