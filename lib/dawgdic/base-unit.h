#ifndef DAWGDIC_BASE_UNIT_H
#define DAWGDIC_BASE_UNIT_H

#include "base-types.h"

namespace dawgdic {

// Unit for building a dawg.
class BaseUnit {
 public:
  BaseUnit() : base_(0) {}

  // Writes values.
  void set_base(BaseType base) {
    base_ = base;
  }
  BaseType base() const {
    return base_;
  }

  // Reads values.
  BaseType child() const {
    return base_ >> 2;
  }
  bool has_sibling() const {
    return (base_ & 1) ? true : false;
  }
  ValueType value() const {
    return static_cast<ValueType>(base_ >> 1);
  }
  bool is_state() const {
    return (base_ & 2) ? true : false;
  }

 private:
  BaseType base_;

  // Copyable.
};

}  // namespace dawgdic

#endif  // DAWGDIC_BASE_UNIT_H
