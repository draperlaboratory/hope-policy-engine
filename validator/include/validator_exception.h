#ifndef VALIDATOR_EXCEPTION_H
#define VALIDATOR_EXCEPTION_H

#include <string>
#include <exception>

namespace policy_engine {

class exception_t : public std::exception {
  std::string msg;
  public:
  exception_t() : msg("unknown exception") { }
  exception_t(std::string msg) : msg(msg) { }
  virtual std::string what() { return msg; }
};

class configuration_exception_t : public exception_t {
  public:
  configuration_exception_t(std::string msg) : exception_t(msg) { }
};

class runtime_exception_t : public exception_t {
  public:
  runtime_exception_t(std::string msg) : exception_t(msg) { }
};

} // namespace policy_engine

#endif
