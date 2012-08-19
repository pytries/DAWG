#ifndef DAWGDIC_DAWG_UNIT_H
#define DAWGDIC_DAWG_UNIT_H

#include "base-types.h"

namespace dawgdic {

// Unit for building a dawg.
class DawgUnit {
 public:
  DawgUnit()
    : child_(0), sibling_(0), label_('\0'),
      is_state_(false), has_sibling_(false) {}

  // Writes values.
  void set_child(BaseType child) {
    child_ = child;
  }
  void set_sibling(BaseType sibling) {
    sibling_ = sibling;
  }
  void set_value(ValueType value) {
    child_ = value;
  }
  void set_label(UCharType label) {
    label_ = label;
  }
  void set_is_state(bool is_state) {
    is_state_ = is_state;
  }
  void set_has_sibling(bool has_sibling) {
    has_sibling_ = has_sibling;
  }

  // Reads values.
  BaseType child() const {
    return child_;
  }
  BaseType sibling() const {
    return sibling_;
  }
  ValueType value() const {
    return static_cast<ValueType>(child_);
  }
  UCharType label() const {
    return label_;
  }
  bool is_state() const {
    return is_state_;
  }
  bool has_sibling() const {
    return has_sibling_;
  }

  // Calculates a base value of a unit.
  BaseType base() const {
    if (label_ == '\0') {
      return (child_ << 1) | (has_sibling_ ? 1 : 0);
    }
    return (child_ << 2) | (is_state_ ? 2 : 0) | (has_sibling_ ? 1 : 0);
  }

  // Initializes a unit.
  void Clear() {
    child_ = 0;
    sibling_ = 0;
    label_ = '\0';
    is_state_ = false;
    has_sibling_ = false;
  }

 private:
  BaseType child_;
  BaseType sibling_;
  UCharType label_;
  bool is_state_;
  bool has_sibling_;

  // Copyable.
};

}  // namespace dawgdic

#endif  // DAWGDIC_DAWG_UNIT_H
