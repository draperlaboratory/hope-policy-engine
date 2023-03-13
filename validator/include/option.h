#ifndef __OPTION_H__
#define __OPTION_H__

#ifdef __cplusplus

#include <stdexcept>

namespace policy_engine {

template<class T>
class option {
private:
  T value;

public:
  const bool exists;

  option() : exists(false) {}
  option(T v) : value(v), exists(true) {}

  const T& get() const { if (exists) return value; else throw std::logic_error("not defined"); }
  const T& getOrElse(const T& v) const noexcept { if (exists) return value; else return v; }

  operator T() const { return get(); }
  explicit operator bool() const noexcept { return exists; }
};

template<class T> static const option<T> none() { return option<T>(); }
template<class T> static const option<T> some(T t) { return option<T>(t); }
template<class T> static const option<T> when(bool b, T t) { return b ? some(t) : none<T>(); }

}

#endif // __cplusplus
#endif // __OPTION_H__