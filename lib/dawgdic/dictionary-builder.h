#ifndef DAWGDIC_DICTIONARY_BUILDER_H
#define DAWGDIC_DICTIONARY_BUILDER_H

#include <vector>

#include "dawg.h"
#include "dictionary.h"
#include "dictionary-extra-unit.h"
#include "link-table.h"

namespace dawgdic {

class DictionaryBuilder {
 public:
  enum {
    // Number of units in a block.
    BLOCK_SIZE = 256,
    // Number of blocks kept unfixed.
    NUM_OF_UNFIXED_BLOCKS = 16,
    // Number of units kept unfixed.
    UNFIXED_SIZE = BLOCK_SIZE * NUM_OF_UNFIXED_BLOCKS
  };

  // Builds a dictionary from a list-form dawg.
  static bool Build(const Dawg &dawg, Dictionary *dic,
                    BaseType *num_of_unused_units = NULL) {
    DictionaryBuilder builder(dawg, dic);
    if (!builder.BuildDictionary()) {
      return false;
    }
    if (num_of_unused_units != NULL) {
      *num_of_unused_units = builder.num_of_unused_units_;
    }
    return true;
  }

 private:
  const Dawg &dawg_;
  Dictionary *dic_;

  std::vector<DictionaryUnit> units_;
  std::vector<DictionaryExtraUnit *> extras_;
  std::vector<UCharType> labels_;
  LinkTable link_table_;
  BaseType unfixed_index_;
  BaseType num_of_unused_units_;

  // Masks for offsets.
  static const BaseType UPPER_MASK = ~(DictionaryUnit::OFFSET_MAX - 1);
  static const BaseType LOWER_MASK = 0xFF;

  // Disallows copies.
  DictionaryBuilder(const DictionaryBuilder &);
  DictionaryBuilder &operator=(const DictionaryBuilder &);

  DictionaryBuilder(const Dawg &dawg, Dictionary *dic)
    : dawg_(dawg), dic_(dic), units_(), extras_(), labels_(),
      link_table_(), unfixed_index_(), num_of_unused_units_(0) {}
  ~DictionaryBuilder() {
    for (SizeType i = 0; i < extras_.size(); ++i) {
      delete [] extras_[i];
    }
  }

  // Accesses units.
  DictionaryUnit &units(BaseType index) {
    return units_[index];
  }
  const DictionaryUnit &units(BaseType index) const {
    return units_[index];
  }
  DictionaryExtraUnit &extras(BaseType index) {
    return extras_[index / BLOCK_SIZE][index % BLOCK_SIZE];
  }
  const DictionaryExtraUnit &extras(BaseType index) const {
    return extras_[index / BLOCK_SIZE][index % BLOCK_SIZE];
  }

  // Number of units.
  BaseType num_of_units() const {
    return static_cast<BaseType>(units_.size());
  }
  // Number of blocks.
  BaseType num_of_blocks() const {
    return static_cast<BaseType>(extras_.size());
  }

  // Builds a dictionary from a list-form dawg.
  bool BuildDictionary() {
    link_table_.Init(dawg_.num_of_merging_states() +
        (dawg_.num_of_merging_states() >> 1));

    ReserveUnit(0);
    extras(0).set_is_used();
    units(0).set_offset(1);
    units(0).set_label('\0');

    if (dawg_.size() > 1) {
      if (!BuildDictionary(dawg_.root(), 0)) {
        return false;
      }
    }

    FixAllBlocks();

    dic_->SwapUnitsBuf(&units_);
    return true;
  }

  // Builds a dictionary from a dawg.
  bool BuildDictionary(BaseType dawg_index, BaseType dic_index) {
    if (dawg_.is_leaf(dawg_index)) {
      return true;
    }

    // Uses an existing offset if available.
    BaseType dawg_child_index = dawg_.child(dawg_index);
    if (dawg_.is_merging(dawg_child_index)) {
      BaseType offset = link_table_.Find(dawg_child_index);
      if (offset != 0) {
        offset ^= dic_index;
        if (!(offset & UPPER_MASK) || !(offset & LOWER_MASK)) {
          if (dawg_.is_leaf(dawg_child_index)) {
            units(dic_index).set_has_leaf();
          }
          units(dic_index).set_offset(offset);
          return true;
        }
      }
    }

    // Finds a good offset and arranges child nodes.
    BaseType offset = ArrangeChildNodes(dawg_index, dic_index);
    if (offset == 0) {
      return false;
    }

    if (dawg_.is_merging(dawg_child_index))
      link_table_.Insert(dawg_child_index, offset); {
    }

    // Builds a double-array in depth-first order.
    do {
      BaseType dic_child_index = offset ^ dawg_.label(dawg_child_index);
      if (!BuildDictionary(dawg_child_index, dic_child_index)) {
        return false;
      }
      dawg_child_index = dawg_.sibling(dawg_child_index);
    } while (dawg_child_index != 0);

    return true;
  }

  // Arranges child nodes.
  BaseType ArrangeChildNodes(BaseType dawg_index, BaseType dic_index) {
    labels_.clear();

    BaseType dawg_child_index = dawg_.child(dawg_index);
    while (dawg_child_index != 0) {
      labels_.push_back(dawg_.label(dawg_child_index));
      dawg_child_index = dawg_.sibling(dawg_child_index);
    }

    // Finds a good offset.
    BaseType offset = FindGoodOffset(dic_index);
    if (!units(dic_index).set_offset(dic_index ^ offset)) {
      return 0;
    }

    dawg_child_index = dawg_.child(dawg_index);
    for (SizeType i = 0; i < labels_.size(); ++i) {
      BaseType dic_child_index = offset ^ labels_[i];
      ReserveUnit(dic_child_index);

      if (dawg_.is_leaf(dawg_child_index)) {
        units(dic_index).set_has_leaf();
        units(dic_child_index).set_value(dawg_.value(dawg_child_index));
      } else {
        units(dic_child_index).set_label(labels_[i]);
      }

      dawg_child_index = dawg_.sibling(dawg_child_index);
    }
    extras(offset).set_is_used();

    return offset;
  }

  // Finds a good offset.
  BaseType FindGoodOffset(BaseType index) const {
    if (unfixed_index_ >= num_of_units()) {
      return num_of_units() | (index & 0xFF);
    }

    // Scans unused units to find a good offset.
    BaseType unfixed_index = unfixed_index_;
    do {
      BaseType offset = unfixed_index ^ labels_[0];
      if (IsGoodOffset(index, offset)) {
        return offset;
      }
      unfixed_index = extras(unfixed_index).next();
    } while (unfixed_index != unfixed_index_);

    return num_of_units() | (index & 0xFF);
  }

  // Checks if a given offset is valid or not.
  bool IsGoodOffset(BaseType index, BaseType offset) const {
    if (extras(offset).is_used()) {
      return false;
    }

    BaseType relative_offset = index ^ offset;
    if ((relative_offset & LOWER_MASK) && (relative_offset & UPPER_MASK)) {
      return false;
    }

    // Finds a collision.
    for (SizeType i = 1; i < labels_.size(); ++i) {
      if (extras(offset ^ labels_[i]).is_fixed()) {
        return false;
      }
    }

    return true;
  }

  // Reserves an unused unit.
  void ReserveUnit(BaseType index) {
    if (index >= num_of_units()) {
      ExpandDictionary();
    }

    // Removes an unused unit from a circular linked list.
    if (index == unfixed_index_) {
      unfixed_index_ = extras(index).next();
      if (unfixed_index_ == index) {
        unfixed_index_ = num_of_units();
      }
    }
    extras(extras(index).prev()).set_next(extras(index).next());
    extras(extras(index).next()).set_prev(extras(index).prev());
    extras(index).set_is_fixed();
  }

  // Expands a dictionary.
  void ExpandDictionary() {
    BaseType src_num_of_units = num_of_units();
    BaseType src_num_of_blocks = num_of_blocks();

    BaseType dest_num_of_units = src_num_of_units + BLOCK_SIZE;
    BaseType dest_num_of_blocks = src_num_of_blocks + 1;

    // Fixes an old block.
    if (dest_num_of_blocks > NUM_OF_UNFIXED_BLOCKS) {
      FixBlock(src_num_of_blocks - NUM_OF_UNFIXED_BLOCKS);
    }

    units_.resize(dest_num_of_units);
    extras_.resize(dest_num_of_blocks, 0);

    // Allocates memory to a new block.
    if (dest_num_of_blocks > NUM_OF_UNFIXED_BLOCKS) {
      BaseType block_id = src_num_of_blocks - NUM_OF_UNFIXED_BLOCKS;
      std::swap(extras_[block_id], extras_.back());
      for (BaseType i = src_num_of_units; i < dest_num_of_units; ++i) {
        extras(i).clear();
      }
    } else {
      extras_.back() = new DictionaryExtraUnit[BLOCK_SIZE];
    }

    // Creates a circular linked list for a new block.
    for (BaseType i = src_num_of_units + 1; i < dest_num_of_units; ++i) {
      extras(i - 1).set_next(i);
      extras(i).set_prev(i - 1);
    }

    extras(src_num_of_units).set_prev(dest_num_of_units - 1);
    extras(dest_num_of_units - 1).set_next(src_num_of_units);

    // Merges 2 circular linked lists.
    extras(src_num_of_units).set_prev(extras(unfixed_index_).prev());
    extras(dest_num_of_units - 1).set_next(unfixed_index_);

    extras(extras(unfixed_index_).prev()).set_next(src_num_of_units);
    extras(unfixed_index_).set_prev(dest_num_of_units - 1);
  }

  // Fixes all blocks to avoid invalid transitions.
  void FixAllBlocks() {
    BaseType begin = 0;
    if (num_of_blocks() > NUM_OF_UNFIXED_BLOCKS) {
      begin = num_of_blocks() - NUM_OF_UNFIXED_BLOCKS;
    }
    BaseType end = num_of_blocks();

    for (BaseType block_id = begin; block_id != end; ++block_id) {
      FixBlock(block_id);
    }
  }

  // Adjusts labels of unused units in a given block.
  void FixBlock(BaseType block_id) {
    BaseType begin = block_id * BLOCK_SIZE;
    BaseType end = begin + BLOCK_SIZE;

    // Finds an unused offset.
    BaseType unused_offset_for_label = 0;
    for (BaseType offset = begin; offset != end; ++offset) {
      if (!extras(offset).is_used()) {
        unused_offset_for_label = offset;
        break;
      }
    }

    // Labels of unused units are modified.
    for (BaseType index = begin; index != end; ++index) {
      if (!extras(index).is_fixed()) {
        ReserveUnit(index);
        units(index).set_label(
            static_cast<UCharType>(index ^ unused_offset_for_label));
        ++num_of_unused_units_;
      }
    }
  }
};

}  // namespace dawgdic

#endif  // DAWGDIC_DICTIONARY_BUILDER_H
