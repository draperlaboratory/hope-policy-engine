#ifndef __TAGGING_UTILS_H__
#define __TAGGING_UTILS_H__

namespace policy_engine {

template<class V, class M>
static constexpr V round_up(const V& value, const M& multiple) { return ((value + multiple - 1)/multiple)*multiple; }

}

#endif // __TAGGING_UTILS_H__