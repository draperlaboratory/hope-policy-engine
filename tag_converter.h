#ifndef TAG_CONVERTER_H
#define TAG_CONVERTER_H

#include "platform_types.h"
#include "policy_meta_set.h"
#include "tag_utils.h"

/**
   The rule caches in tag based validators deal in tags.  The policy evaluation function
   deals in meta_set_t structures.  While many implementations may have a one to one mapping
   of tag to meta_set_t, it is possible that there will be a conversion.  This interface
   preserves that possibility, or at least indicates the points at which that conversion
   would need to be supported in code at the interface boundary between the policy
   evaluation function(s) and the rest of the world.
*/
struct tag_converter_t {
  meta_set_t *t_to_m(tag_t t) { return (meta_set_t *)t; }
  tag_t m_to_t(meta_set_t *ms) { return (tag_t)ms; }
};

#endif
