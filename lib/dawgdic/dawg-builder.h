#ifndef DAWGDIC_DAWG_BUILDER_H
#define DAWGDIC_DAWG_BUILDER_H

#include <algorithm>
#include <stack>
#include <vector>

#include "dawg.h"
#include "dawg-unit.h"

namespace dawgdic {

// DAWG builder.
class DawgBuilder {
 public:
  explicit DawgBuilder(SizeType initial_hash_table_size =
                       DEFAULT_INITIAL_HASH_TABLE_SIZE)
    : initial_hash_table_size_(initial_hash_table_size),
      base_pool_(), label_pool_(), flag_pool_(), unit_pool_(),
      hash_table_(), unfixed_units_(), unused_units_(), num_of_states_(1),
      num_of_merged_transitions_(0), num_of_merging_states_(0) {}

  // Number of units.
  SizeType size() const {
    return base_pool_.size();
  }
  // Number of transitions.
  SizeType num_of_transitions() const {
    return base_pool_.size() - 1;
  }
  // Number of states.
  SizeType num_of_states() const {
    return num_of_states_;
  }
  // Number of merged transitions.
  SizeType num_of_merged_transitions() const {
    return num_of_merged_transitions_;
  }
  // Number of merged states.
  SizeType num_of_merged_states() const {
    return num_of_transitions()
        + num_of_merged_transitions() + 1 - num_of_states();
  }
  // Number of merging states.
  SizeType num_of_merging_states() const {
    return num_of_merging_states_;
  }

  // Initializes a builder.
  void Clear() {
    base_pool_.Clear();
    label_pool_.Clear();
    flag_pool_.Clear();
    unit_pool_.Clear();

    std::vector<BaseType>(0).swap(hash_table_);
    while (!unfixed_units_.empty()) {
      unfixed_units_.pop();
    }
    while (!unused_units_.empty()) {
      unused_units_.pop();
    }

    num_of_states_ = 1;
    num_of_merged_transitions_ = 0;
    num_of_merging_states_ = 0;
  }

  // Inserts a key.
  bool Insert(const CharType *key, ValueType value = 0) {
    if (key == NULL || *key == '\0' || value < 0) {
      return false;
    }
    SizeType length = 1;
    while (key[length]) {
      ++length;
    }
    return InsertKey(key, length, value);
  }

  // Inserts a key.
  bool Insert(const CharType *key, SizeType length, ValueType value) {
    if (key == NULL || length <= 0 || value < 0) {
      return false;
    }
    for (SizeType i = 0; i < length; ++i) {
      if (key[i] == '\0') {
        return false;
      }
    }
    return InsertKey(key, length, value);
  }

  // Finishes building a dawg.
  bool Finish(Dawg *dawg) {
    // Initializes a builder if not initialized.
    if (hash_table_.empty()) {
      Init();
    }

    FixUnits(0);
    base_pool_[0].set_base(unit_pool_[0].base());
    label_pool_[0] = unit_pool_[0].label();

    dawg->set_num_of_states(num_of_states_);
    dawg->set_num_of_merged_transitions(num_of_merged_transitions_);
    dawg->set_num_of_merged_states(num_of_merged_states());
    dawg->set_num_of_merging_states(num_of_merging_states_);

    dawg->SwapBasePool(&base_pool_);
    dawg->SwapLabelPool(&label_pool_);
    dawg->SwapFlagPool(&flag_pool_);

    Clear();
    return true;
  }

 private:
  enum {
    DEFAULT_INITIAL_HASH_TABLE_SIZE = 1 << 8
  };

  const SizeType initial_hash_table_size_;
  ObjectPool<BaseUnit> base_pool_;
  ObjectPool<UCharType> label_pool_;
  BitPool<> flag_pool_;
  ObjectPool<DawgUnit> unit_pool_;
  std::vector<BaseType> hash_table_;
  std::stack<BaseType> unfixed_units_;
  std::stack<BaseType> unused_units_;
  SizeType num_of_states_;
  SizeType num_of_merged_transitions_;
  SizeType num_of_merging_states_;

  // Disallows copies.
  DawgBuilder(const DawgBuilder &);
  DawgBuilder &operator=(const DawgBuilder &);

  // Inserts a key.
  bool InsertKey(const CharType *key, SizeType length, ValueType value) {
    // Initializes a builder if not initialized.
    if (hash_table_.empty()) {
      Init();
    }

    BaseType index = 0;
    SizeType key_pos = 0;

    // Finds a separate unit.
    for ( ; key_pos <= length; ++key_pos) {
      BaseType child_index = unit_pool_[index].child();
      if (!child_index) {
        break;
      }

      UCharType key_label = static_cast<UCharType>(
          (key_pos < length) ? key[key_pos] : '\0');
      UCharType unit_label = unit_pool_[child_index].label();

      // Checks the order of keys.
      if (key_label < unit_label) {
        return false;
      } else if (key_label > unit_label) {
        unit_pool_[child_index].set_has_sibling(true);
        FixUnits(child_index);
        break;
      }

      index = child_index;
    }

    // Adds new units.
    for ( ; key_pos <= length; ++key_pos) {
      UCharType key_label = static_cast<UCharType>(
          (key_pos < length) ? key[key_pos] : '\0');
      BaseType child_index = AllocateUnit();

      if (!unit_pool_[index].child()) {
        unit_pool_[child_index].set_is_state(true);
      }
      unit_pool_[child_index].set_sibling(unit_pool_[index].child());
      unit_pool_[child_index].set_label(key_label);
      unit_pool_[index].set_child(child_index);
      unfixed_units_.push(child_index);

      index = child_index;
    }
    unit_pool_[index].set_value(value);
    return true;
  }

  // Initializes an object.
  void Init() {
    hash_table_.resize(initial_hash_table_size_, 0);
    AllocateUnit();
    AllocateTransition();
    unit_pool_[0].set_label(0xFF);
    unfixed_units_.push(0);
  }

  // Fixes units corresponding to the last inserted key.
  // Also, some of units are merged into equivalent transitions.
  void FixUnits(BaseType index) {
    while (unfixed_units_.top() != index) {
      BaseType unfixed_index = unfixed_units_.top();
      unfixed_units_.pop();

      if (num_of_states_ >= hash_table_.size() - (hash_table_.size() >> 2)) {
        ExpandHashTable();
      }

      BaseType num_of_siblings = 0;
      for (BaseType i = unfixed_index; i != 0; i = unit_pool_[i].sibling()) {
        ++num_of_siblings;
      }

      BaseType hash_id;
      BaseType matched_index = FindUnit(unfixed_index, &hash_id);
      if (matched_index != 0) {
        num_of_merged_transitions_ += num_of_siblings;

        // Records a merging state.
        if (flag_pool_.get(matched_index) == false) {
          ++num_of_merging_states_;
          flag_pool_.set(matched_index, true);
        }
      } else {
        // Fixes units into pairs of base values and labels.
        BaseType transition_index = 0;
        for (BaseType i = 0; i < num_of_siblings; ++i) {
          transition_index = AllocateTransition();
        }
        for (BaseType i = unfixed_index; i != 0; i = unit_pool_[i].sibling()) {
          base_pool_[transition_index].set_base(unit_pool_[i].base());
          label_pool_[transition_index] = unit_pool_[i].label();
          --transition_index;
        }
        matched_index = transition_index + 1;
        hash_table_[hash_id] = matched_index;
        ++num_of_states_;
      }

      // Deletes fixed units.
      for (BaseType current = unfixed_index, next;
           current != 0; current = next) {
        next = unit_pool_[current].sibling();
        FreeUnit(current);
      }

      unit_pool_[unfixed_units_.top()].set_child(matched_index);
    }
    unfixed_units_.pop();
  }

  // Expands a hash table.
  void ExpandHashTable() {
    SizeType hash_table_size = hash_table_.size() << 1;
    std::vector<BaseType>(0).swap(hash_table_);
    hash_table_.resize(hash_table_size, 0);

    // Builds a new hash table.
    BaseType count = 0;
    for (SizeType i = 1; i < base_pool_.size(); ++i) {
      BaseType index = static_cast<BaseType>(i);
      if (label_pool_[index] == '\0' || base_pool_[index].is_state()) {
        BaseType hash_id;
        FindTransition(index, &hash_id);
        hash_table_[hash_id] = index;
        ++count;
      }
    }
  }

  // Finds a transition from a hash table.
  BaseType FindTransition(BaseType index, BaseType *hash_id) const {
    *hash_id = HashTransition(index) % hash_table_.size();
    for ( ; ; *hash_id = (*hash_id + 1) % hash_table_.size()) {
      BaseType transition_id = hash_table_[*hash_id];
      if (transition_id == 0) {
        break;
      }

      // There must not be the same base value.
    }
    return 0;
  }

  // Finds a unit from a hash table.
  BaseType FindUnit(BaseType unit_index, BaseType *hash_id) const {
    *hash_id = HashUnit(unit_index) % hash_table_.size();
    for ( ; ; *hash_id = (*hash_id + 1) % hash_table_.size()) {
      BaseType transition_id = hash_table_[*hash_id];
      if (transition_id == 0) {
        break;
      }

      if (AreEqual(unit_index, transition_id)) {
        return transition_id;
      }
    }
    return 0;
  }

  // Compares a unit and a transition.
  bool AreEqual(BaseType unit_index, BaseType transition_index) const {
    // Compares the numbers of transitions.
    for (BaseType i = unit_pool_[unit_index].sibling(); i != 0;
         i = unit_pool_[i].sibling()) {
      if (base_pool_[transition_index].has_sibling() == false) {
        return false;
      }
      ++transition_index;
    }
    if (base_pool_[transition_index].has_sibling() == true) {
      return false;
    }

    // Compares out-transitions.
    for (BaseType i = unit_index; i;
         i = unit_pool_[i].sibling(), --transition_index) {
      if (unit_pool_[i].base() != base_pool_[transition_index].base() ||
          unit_pool_[i].label() != label_pool_[transition_index]) {
        return false;
      }
    }
    return true;
  }

  // Calculates a hash value from a transition.
  BaseType HashTransition(BaseType index) const {
    BaseType hash_value = 0;
    for ( ; index != 0; ++index) {
      BaseType base = base_pool_[index].base();
      UCharType label = label_pool_[index];
      hash_value ^= Hash((label << 24) ^ base);

      if (base_pool_[index].has_sibling() == false) {
        break;
      }
    }
    return hash_value;
  }

  // Calculates a hash value from a unit.
  BaseType HashUnit(BaseType index) const {
    BaseType hash_value = 0;
    for ( ; index != 0; index = unit_pool_[index].sibling()) {
      BaseType base = unit_pool_[index].base();
      UCharType label = unit_pool_[index].label();
      hash_value ^= Hash((label << 24) ^ base);
    }
    return hash_value;
  }

  // 32-bit mix function.
  // http://www.concentric.net/~Ttwang/tech/inthash.htm
  static BaseType Hash(BaseType key) {
    key = ~key + (key << 15);  // key = (key << 15) - key - 1;
    key = key ^ (key >> 12);
    key = key + (key << 2);
    key = key ^ (key >> 4);
    key = key * 2057;  // key = (key + (key << 3)) + (key << 11);
    key = key ^ (key >> 16);
    return key;
  }

  // Gets a transition from object pools.
  BaseType AllocateTransition() {
    flag_pool_.Allocate();
    base_pool_.Allocate();
    return static_cast<BaseType>(label_pool_.Allocate());
  }

  // Gets a unit from an object pool.
  BaseType AllocateUnit() {
    BaseType index = 0;
    if (unused_units_.empty()) {
      index = static_cast<BaseType>(unit_pool_.Allocate());
    } else {
      index = unused_units_.top();
      unused_units_.pop();
    }
    unit_pool_[index].Clear();
    return index;
  }

  // Returns a unit to an object pool.
  void FreeUnit(BaseType index) {
    unused_units_.push(index);
  }
};

}  // namespace dawgdic

#endif  // DAWGDIC_DAWG_BUILDER_H
