#ifndef DAWGDIC_DICTIONARY_EXTRA_UNIT_H
#define DAWGDIC_DICTIONARY_EXTRA_UNIT_H

#include "base-types.h"

namespace dawgdic {

// Extra unit for building a dictionary.
class DictionaryExtraUnit {
 public:
  DictionaryExtraUnit() : lo_values_(0), hi_values_(0) {}

  void clear() {
    lo_values_ = hi_values_ = 0;
  }

  // Sets if a unit is fixed or not.
  void set_is_fixed() {
    lo_values_ |= 1;
  }
  // Sets an index of the next unused unit.
  void set_next(BaseType next) {
    lo_values_ = (lo_values_ & 1) | (next << 1);
  }
  // Sets if an index is used as an offset or not.
  void set_is_used() {
    hi_values_ |= 1;
  }
  // Sets an index of the previous unused unit.
  void set_prev(BaseType prev) {
    hi_values_ = (hi_values_ & 1) | (prev << 1);
  }

  // Reads if a unit is fixed or not.
  bool is_fixed() const {
    return (lo_values_ & 1) == 1;
  }
  // Reads an index of the next unused unit.
  BaseType next() const {
    return lo_values_ >> 1;
  }
  // Reads if an index is used as an offset or not.
  bool is_used() const {
    return (hi_values_ & 1) == 1;
  }
  // Reads an index of the previous unused unit.
  BaseType prev() const {
    return hi_values_ >> 1;
  }

 private:
  BaseType lo_values_;
  BaseType hi_values_;

  // Copyable.
};

}  // namespace dawgdic

#endif  // DAWGDIC_DICTIONARY_EXTRA_UNIT_H
