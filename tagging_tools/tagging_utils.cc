#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>

namespace policy_engine {

class RangeFile {
private:
  int _fd;
  std::string _name;
public:
  RangeFile() : _name("/tmp/range_XXXXXX") {
    _fd = mkstemp((char*)_name.data());
  }

  size_t write_range(uint64_t start, uint64_t end, const std::string& tag);
  void finish();
  std::string name();
  int fd();
  void done();
  void print();

  ~RangeFile() {
    if (_fd >= 0) {
      done();
      finish();
    }
  }
};

size_t RangeFile::write_range(uint64_t start, uint64_t end, const std::string& tag) {
  char buf[tag.length() + 48];
  size_t size = std::sprintf(buf, "0x%016lx 0x%016lx %s\n", start, end, tag.c_str());
  return write(_fd, buf, size);
}

void RangeFile::finish() {
  close(_fd);
}

std::string RangeFile::name() {
  return _name;
}



void RangeFile::done() {
  if (_fd >= 0) {
    unlink(_name.c_str());
    _fd = -1;
  }
}

void RangeFile::print() {
  std::ifstream file(_name);
  std::string line;
  while (std::getline(file, line))
    std::cout << line << '\n';
}



} // namespace policy_engine