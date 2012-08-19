#ifndef DAWGDIC_DAWG_H
#define DAWGDIC_DAWG_H

#include "base-unit.h"
#include "bit-pool.h"
#include "object-pool.h"

namespace dawgdic {

class Dawg {
 public:
  Dawg()
    : base_pool_(), label_pool_(), flag_pool_(),
      num_of_states_(0), num_of_merged_transitions_(0),
      num_of_merged_states_(0), num_of_merging_states_(0) {}

  // The root index.
  BaseType root() const {
    return 0;
  }

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
    return num_of_merged_states_;
  }
  // Number of merging states.
  SizeType num_of_merging_states() const {
    return num_of_merging_states_;
  }

  // Reads values.
  BaseType child(BaseType index) const {
    return base_pool_[index].child();
  }
  BaseType sibling(BaseType index) const {
    return base_pool_[index].has_sibling() ? (index + 1) : 0;
  }
  ValueType value(BaseType index) const {
    return base_pool_[index].value();
  }

  bool is_leaf(BaseType index) const {
    return label(index) == '\0';
  }
  UCharType label(BaseType index) const {
    return label_pool_[index];
  }
  bool is_merging(BaseType index) const {
    return flag_pool_.get(index);
  }

  // Clears object pools.
  void Clear() {
    base_pool_.Clear();
    label_pool_.Clear();
    flag_pool_.Clear();
    num_of_states_ = 0;
    num_of_merged_states_ = 0;
  }

  // Swaps dawgs.
  void Swap(Dawg *dawg) {
    base_pool_.Swap(&dawg->base_pool_);
    label_pool_.Swap(&dawg->label_pool_);
    flag_pool_.Swap(&dawg->flag_pool_);
    std::swap(num_of_states_, dawg->num_of_states_);
    std::swap(num_of_merged_transitions_, dawg->num_of_merged_transitions_);
    std::swap(num_of_merged_states_, dawg->num_of_merged_states_);
    std::swap(num_of_merging_states_, dawg->num_of_merging_states_);
  }

 public:
  // Following member functions are called from DawgBuilder.

  // Sets the number of states.
  void set_num_of_states(SizeType num_of_states) {
    num_of_states_ = num_of_states;
  }
  // Sets the number of merged transitions.
  void set_num_of_merged_transitions(SizeType num_of_merged_transitions) {
    num_of_merged_transitions_ = num_of_merged_transitions;
  }
  // Sets the number of merged states.
  void set_num_of_merged_states(SizeType num_of_merged_states) {
    num_of_merged_states_ = num_of_merged_states;
  }
  // Sets the number of merging states.
  void set_num_of_merging_states(SizeType num_of_merging_states) {
    num_of_merging_states_ = num_of_merging_states;
  }

  // Swaps base pools.
  void SwapBasePool(ObjectPool<BaseUnit> *base_pool) {
    base_pool_.Swap(base_pool);
  }
  // Swaps label pools.
  void SwapLabelPool(ObjectPool<UCharType> *label_pool) {
    label_pool_.Swap(label_pool);
  }
  // Swaps flag pools.
  void SwapFlagPool(BitPool<> *flag_pool) {
    flag_pool_.Swap(flag_pool);
  }

 private:
  ObjectPool<BaseUnit> base_pool_;
  ObjectPool<UCharType> label_pool_;
  BitPool<> flag_pool_;
  SizeType num_of_states_;
  SizeType num_of_merged_transitions_;
  SizeType num_of_merged_states_;
  SizeType num_of_merging_states_;

  // Disallows copies.
  Dawg(const Dawg &);
  Dawg &operator=(const Dawg &);
};

}  // namespace dawgdic

#endif  // DAWGDIC_DAWG_H
