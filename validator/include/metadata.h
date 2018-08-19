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

#ifndef METADATA_H
#define METADATA_H

#include <assert.h>
#include <stdint.h>

#include <cstdlib>
#include <iterator>
#include <vector>

#include "policy_types.h"
#include "policy_meta_set.h"

namespace policy_engine {

  class metadata_t {
  
  std::size_t hash;
  
  public:

  meta_set_t tags;

  struct hasher_t {
    std::size_t operator()(const metadata_t &k) const {
      return k.hash;
    }
  };

  struct equal_t {
    bool operator()(metadata_t const &l, metadata_t const &r) const {
      return ms_eq(&l.tags, &r.tags);
    }
  };

  metadata_t() { ms_zero(&tags); }
  
  size_t size() const { return ms_count(&tags); }

  metadata_t(const metadata_t& rhs) { ms_zero(&tags); ms_union(&tags, &rhs.tags); hash = rhs.hash; }
  metadata_t& operator=(const metadata_t& rhs) { ms_zero(&tags); ms_union(&tags, &rhs.tags); hash = rhs.hash; }
  
  bool operator ==(const metadata_t &rhs) const {
    return ms_eq(&tags, &rhs.tags);
  }

  bool operator !=(const metadata_t &rhs) const { return !ms_eq(&tags, &rhs.tags); }

  void insert(const meta_t &rhs) {
    hash += rhs;
    ms_bit_add(&tags, rhs);
  }

  void insert(const metadata_t *rhs) {
    for ( int t = 0; t <= MAX_TAG; t++ ) {
      if ( ms_contains(&tags, t) )
	hash += t;
    }
    ms_union(&tags, &rhs->tags);
  }

  std::vector<meta_t> pull_metadata() const {
    std::vector<meta_t> md;
    for ( int t = 0; t <= MAX_TAG; t++ ) {
      if ( ms_contains(&tags, t) )
	md.push_back((meta_t)t);
    }

    return md;
  }
  
};

} // namespace policy_engine

#endif
