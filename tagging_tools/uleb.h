#ifndef ULEB_H
#define ULEB_H

#include <stdint.h>

/**
   ULEB encoding functions.  These are generics that allow you to specify a reader type and
   a value type.
*/

/**
   Reads a ULEB encoded unsigned value.  The <code>StreamType</code> type parameter specifies
   a type that must support the following API:

   bool read_byte(uint8_t &b)
   Returns <code>true</code> on success, <code>false</code> on failure.

   The ValueType type parameter will be the integral type to be read.  E.g. \a uint32_t.

   \return Returns true on success and false on failure.
*/
template <typename StreamType, typename ValueType> bool read_uleb(StreamType *stream, ValueType &v) {
  v = 0;
  int shift = 0;
  uint8_t b;
  do {
    if (!stream->read_byte(b))
      return false;
    v = v | ((((ValueType)b) & 0x7f) << shift);
    shift += 7;
  } while (b & 0x80);
  return true;
}

/**
   Writes a ULEB encoded unsigned value.  The <code>StreamType</code> type parameter specifies
   a type that must support the following API:

   bool write_byte(uint8_t b)
   Returns <code>true</code> on success, <code>false</code> on failure.

   The ValueType type parameter will be the integral type to be written.  E.g. \a uint32_t.

   \return Returns true on success and false on failure.
*/
template <typename StreamType, typename ValueType> bool write_uleb(StreamType *stream, ValueType v) {
  do {
    uint8_t b = v & 0x7f;
    v = v >> 7;
    if (v)
      b |= 0x80;
    if (!stream->write_byte(b))
      return false;
  } while (v != 0);
  return true;
}

#endif
