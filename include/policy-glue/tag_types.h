#ifndef TAG_TYPES_H
#define TAG_TYPES_H

#include <stdint.h>

// create a shared types file for pex and kernel
  /**
     This type is used primarily to identify where we store specific tagged values that we use to paint
     things with in the kernel.  The concept is to keep the locations in the code where we hold these
     values clear.  The uses should be quite rare.
   */
  typedef uintptr_t tagged_value_t;


#endif
