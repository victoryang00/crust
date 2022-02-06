#ifndef _CRUST_TEST_RAII_CHECKER_HPP
#define _CRUST_TEST_RAII_CHECKER_HPP


#include <unordered_map>
#include <memory>

#include "utility.hpp"


namespace test {
struct RAIITypeInfo {};

class RAIIRecorder {
private:
  struct Record {
    const RAIITypeInfo *type_info;
  };

  std::unordered_map<void *, Record> record;

public:
  RAIIRecorder() noexcept : record{} {}

  void construct(const RAIITypeInfo *type_info, void *self) {
    crust_assert(record.find(self) == record.end());
    record.emplace(self, Record{type_info});
  }

  void deconstruct(const RAIITypeInfo *type_info, void *self) {
    auto ptr = record.find(self);
    crust_assert(ptr != record.end() && ptr->second.type_info == type_info);
    record.erase(ptr);
  }

  ~RAIIRecorder() { crust_assert(record.empty()); }
};

template<class Self>
class RAIIChecker {
private:
  static const RAIITypeInfo TYPE_INFO;

  std::shared_ptr<RAIIRecorder> recorder;

public:
  explicit RAIIChecker(std::shared_ptr<RAIIRecorder> recorder) noexcept :
      recorder{std::move(recorder)}
  {
    this->recorder->construct(&TYPE_INFO, this);
  }

  RAIIChecker(const RAIIChecker &other) noexcept : recorder{other.recorder} {
    this->recorder->construct(&TYPE_INFO, this);
  }

  RAIIChecker(RAIIChecker &&other) noexcept : recorder{other.recorder} {
    this->recorder->construct(&TYPE_INFO, this);
  }

  RAIIChecker &operator=(const RAIIChecker &other) {
    crust_assert(recorder == other.recorder);
    return *this;
  }

  RAIIChecker &operator=(RAIIChecker &&other) {
    crust_assert(recorder == other.recorder);
    return *this;
  }

  ~RAIIChecker() { recorder->deconstruct(&TYPE_INFO, this); }
};

template<class Self>
const RAIITypeInfo RAIIChecker<Self>::TYPE_INFO{};
}


#endif //_CRUST_TEST_RAII_CHECKER_HPP
