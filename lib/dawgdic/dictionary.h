#ifndef DAWGDIC_DICTIONARY_H
#define DAWGDIC_DICTIONARY_H

#include <iostream>
#include <vector>

#include "base-types.h"
#include "dictionary-unit.h"

namespace dawgdic {

// Dictionary class for retrieval and binary I/O.
class Dictionary {
 public:
  Dictionary() : units_(NULL), size_(0), units_buf_() {}

  const DictionaryUnit *units() const {
    return units_;
  }
  SizeType size() const {
    return size_;
  }
  SizeType total_size() const {
    return sizeof(DictionaryUnit) * size_;
  }
  SizeType file_size() const {
    return sizeof(BaseType) + total_size();
  }

  // Root index.
  BaseType root() const {
    return 0;
  }

  // Checks if a given index is related to the end of a key.
  bool has_value(BaseType index) const {
    return units_[index].has_leaf();
  }
  // Gets a value from a given index.
  ValueType value(BaseType index) const {
    return units_[index ^ units_[index].offset()].value();
  }

  // Reads a dictionary from an input stream.
  bool Read(std::istream *input) {
    BaseType base_size;
    if (!input->read(reinterpret_cast<char *>(&base_size), sizeof(BaseType))) {
      return false;
    }

    SizeType size = static_cast<SizeType>(base_size);
    std::vector<DictionaryUnit> units_buf(size);
    if (!input->read(reinterpret_cast<char *>(&units_buf[0]),
                     sizeof(DictionaryUnit) * size)) {
      return false;
    }

    SwapUnitsBuf(&units_buf);
    return true;
  }

  // Writes a dictionry to an output stream.
  bool Write(std::ostream *output) const {
    BaseType base_size = static_cast<BaseType>(size_);
    if (!output->write(reinterpret_cast<const char *>(&base_size),
                       sizeof(BaseType))) {
      return false;
    }

    if (!output->write(reinterpret_cast<const char *>(units_),
                       sizeof(DictionaryUnit) * size_)) {
      return false;
    }

    return true;
  }

  // Exact matching.
  bool Contains(const CharType *key) const {
    BaseType index = root();
    if (!Follow(key, &index)) {
      return false;
    }
    return has_value(index);
  }
  bool Contains(const CharType *key, SizeType length) const {
    BaseType index = root();
    if (!Follow(key, length, &index)) {
      return false;
    }
    return has_value(index);
  }

  // Exact matching.
  ValueType Find(const CharType *key) const {
    BaseType index = root();
    if (!Follow(key, &index)) {
      return -1;
    }
    return has_value(index) ? value(index) : -1;
  }
  ValueType Find(const CharType *key, SizeType length) const {
    BaseType index = root();
    if (!Follow(key, length, &index)) {
      return -1;
    }
    return has_value(index) ? value(index) : -1;
  }
  bool Find(const CharType *key, ValueType *value) const {
    BaseType index = root();
    if (!Follow(key, &index) || !has_value(index)) {
      return false;
    }
    *value = this->value(index);
    return true;
  }
  bool Find(const CharType *key, SizeType length, ValueType *value) const {
    BaseType index = root();
    if (!Follow(key, length, &index) || !has_value(index)) {
      return false;
    }
    *value = this->value(index);
    return true;
  }

  // Follows a transition.
  bool Follow(CharType label, BaseType *index) const {
    BaseType next_index =
        *index ^ units_[*index].offset() ^ static_cast<UCharType>(label);
    if (units_[next_index].label() != static_cast<UCharType>(label)) {
      return false;
    }
    *index = next_index;
    return true;
  }

  // Follows transitions.
  bool Follow(const CharType *s, BaseType *index) const {
    while (*s != '\0' && Follow(*s, index)) {
      ++s;
    }
    return *s == '\0';
  }
  bool Follow(const CharType *s, BaseType *index, SizeType *count) const {
    while (*s != '\0' && Follow(*s, index)) {
      ++s, ++*count;
    }
    return *s == '\0';
  }

  // Follows transitions.
  bool Follow(const CharType *s, SizeType length, BaseType *index) const {
    for (SizeType i = 0; i < length; ++i) {
      if (!Follow(s[i], index)) {
        return false;
      }
    }
    return true;
  }
  bool Follow(const CharType *s, SizeType length, BaseType *index,
              SizeType *count) const {
    for (SizeType i = 0; i < length; ++i, ++*count) {
      if (!Follow(s[i], index)) {
        return false;
      }
    }
    return true;
  }

  // Maps memory with its size.
  void Map(const void *address) {
    Clear();
    units_ = reinterpret_cast<const DictionaryUnit *>(
        static_cast<const BaseType *>(address) + 1);
    size_ = *static_cast<const BaseType *>(address);
  }
  void Map(const void *address, SizeType size) {
    Clear();
    units_ = static_cast<const DictionaryUnit *>(address);
    size_ = size;
  }

  // Initializes a dictionary.
  void Clear() {
    units_ = NULL;
    size_ = 0;
    std::vector<DictionaryUnit>(0).swap(units_buf_);
  }

  // Swaps dictionaries.
  void Swap(Dictionary *dic) {
    std::swap(units_, dic->units_);
    std::swap(size_, dic->size_);
    units_buf_.swap(dic->units_buf_);
  }

  // Shrinks a vector.
  void Shrink() {
    if (units_buf_.size() == units_buf_.capacity()) {
      return;
    }

    std::vector<DictionaryUnit> units_buf(units_buf_);
    SwapUnitsBuf(&units_buf);
  }

public:
  // Following member function is called from DawgBuilder.

  // Swaps buffers for units.
  void SwapUnitsBuf(std::vector<DictionaryUnit> *units_buf) {
    units_ = &(*units_buf)[0];
    size_ = static_cast<BaseType>(units_buf->size());
    units_buf_.swap(*units_buf);
  }

 private:
  const DictionaryUnit *units_;
  SizeType size_;
  std::vector<DictionaryUnit> units_buf_;

  // Disallows copies.
  Dictionary(const Dictionary &);
  Dictionary &operator=(const Dictionary &);
};

}  // namespace dawgdic

#endif  // DAWGDIC_DICTIONARY_H
