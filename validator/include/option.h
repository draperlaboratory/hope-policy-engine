#ifndef __OPTION_H__
#define __OPTION_H__

namespace policy_engine {

template<class T>
class option {
private:
  T value;
public:
  const bool exists;

  option() : exists(false) {}
  option(T v) : value(v), exists(true) {}
  operator T() const { if (exists) return value; else throw std::logic_error("not defined"); }
};

template<class T> static const option<T> none() { return option<T>(); }
template<class T> static const option<T> some(T t) { return option<T>(t); }
template<class T> static const option<T> when(bool b, T t) { return b ? some(t) : none<T>(); }

}

#endif // __OPTION_H__