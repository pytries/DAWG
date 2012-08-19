#ifndef DAWGDIC_DICTIONARY_UNIT_H
#define DAWGDIC_DICTIONARY_UNIT_H

#include "base-types.h"

namespace dawgdic {

// Unit of a dictionary.
class DictionaryUnit
{
 public:
  static const BaseType OFFSET_MAX = static_cast<BaseType>(1) << 21;
  static const BaseType IS_LEAF_BIT = static_cast<BaseType>(1) << 31;
  static const BaseType HAS_LEAF_BIT = static_cast<BaseType>(1) << 8;
  static const BaseType EXTENSION_BIT = static_cast<BaseType>(1) << 9;

  DictionaryUnit() : base_(0) {}

  // Sets a flag to show that a unit has a leaf as a child.
  void set_has_leaf() {
    base_ |= HAS_LEAF_BIT;
  }
  // Sets a value to a leaf unit.
  void set_value(ValueType value) {
    base_ = static_cast<BaseType>(value) | IS_LEAF_BIT;
  }
  // Sets a label to a non-leaf unit.
  void set_label(UCharType label) {
    base_ = (base_ & ~static_cast<BaseType>(0xFF)) | label;
  }
  // Sets an offset to a non-leaf unit.
  bool set_offset(BaseType offset) {
    if (offset >= (OFFSET_MAX << 8)) {
      return false;
    }

    base_ &= IS_LEAF_BIT | HAS_LEAF_BIT | 0xFF;
    if (offset < OFFSET_MAX) {
      base_ |= (offset << 10);
    } else {
      base_ |= (offset << 2) | EXTENSION_BIT;
    }
    return true;
  }

  // Checks if a unit has a leaf as a child or not.
  bool has_leaf() const {
    return (base_ & HAS_LEAF_BIT) ? true : false;
  }
  // Checks if a unit corresponds to a leaf or not.
  ValueType value() const {
    return static_cast<ValueType>(base_ & ~IS_LEAF_BIT);
  }
  // Reads a label with a leaf flag from a non-leaf unit.
  BaseType label() const {
    return base_ & (IS_LEAF_BIT | 0xFF);
  }
  // Reads an offset to child units from a non-leaf unit.
  BaseType offset() const {
    return (base_ >> 10) << ((base_ & EXTENSION_BIT) >> 6);
  }

 private:
  BaseType base_;

  // Copyable.
};

}  // namespace dawgdic

#endif  // DAWGDIC_DICTIONARY_UNIT_H
