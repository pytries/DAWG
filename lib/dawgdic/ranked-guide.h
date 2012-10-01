#ifndef DAWGDIC_RANKED_GUIDE_H
#define DAWGDIC_RANKED_GUIDE_H

#include "dictionary.h"
#include "ranked-guide-unit.h"

#include <iostream>
#include <vector>

namespace dawgdic {

class RankedGuide {
 public:
  RankedGuide() : units_(NULL), size_(0), units_buf_() {}

  const RankedGuideUnit *units() const {
    return units_;
  }
  SizeType size() const {
    return size_;
  }
  SizeType total_size() const {
    return sizeof(RankedGuideUnit) * size_;
  }
  SizeType file_size() const {
    return sizeof(BaseType) + total_size();
  }

  // The root index.
  BaseType root() const {
    return 0;
  }

  UCharType child(BaseType index) const {
    return units_[index].child();
  }
  UCharType sibling(BaseType index) const {
    return units_[index].sibling();
  }

  // Reads a dictionary from an input stream.
  bool Read(std::istream *input) {
    BaseType base_size;
    if (!input->read(reinterpret_cast<char *>(&base_size), sizeof(BaseType))) {
      return false;
    }

    SizeType size = static_cast<SizeType>(base_size);
    std::vector<RankedGuideUnit> units_buf(size);
    if (!input->read(reinterpret_cast<char *>(&units_buf[0]),
                     sizeof(RankedGuideUnit) * size)) {
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
                       sizeof(RankedGuideUnit) * size_)) {
      return false;
    }

    return true;
  }

  // Maps memory with its size.
  void Map(const void *address) {
    Clear();
    units_ = reinterpret_cast<const RankedGuideUnit *>(
        static_cast<const BaseType *>(address) + 1);
    size_ = *static_cast<const BaseType *>(address);
  }
  void Map(const void *address, SizeType size) {
    Clear();
    units_ = static_cast<const RankedGuideUnit *>(address);
    size_ = size;
  }

  // Swaps RankedGuides.
  void Swap(RankedGuide *guide) {
    std::swap(units_, guide->units_);
    std::swap(size_, guide->size_);
    units_buf_.swap(guide->units_buf_);
  }

  // Initializes a RankedGuide.
  void Clear() {
    units_ = NULL;
    size_ = 0;
    std::vector<RankedGuideUnit>(0).swap(units_buf_);
  }

 public:
  // Following member function is called from DawgBuilder.

  // Swaps buffers for units.
  void SwapUnitsBuf(std::vector<RankedGuideUnit> *units_buf) {
    units_ = &(*units_buf)[0];
    size_ = static_cast<BaseType>(units_buf->size());
    units_buf_.swap(*units_buf);
  }

 private:
  const RankedGuideUnit *units_;
  SizeType size_;
  std::vector<RankedGuideUnit> units_buf_;

  // Disables copies.
  RankedGuide(const RankedGuide &);
  RankedGuide &operator=(const RankedGuide &);
};

}  // namespace dawgdic

#endif  // DAWGDIC_RANKED_GUIDE_H
