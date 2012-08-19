#ifndef DAWGDIC_RANKED_COMPLETER_CANDIDATE_H
#define DAWGDIC_RANKED_COMPLETER_CANDIDATE_H

#include "base-types.h"

namespace dawgdic {

class RankedCompleterCandidate {
 public:
  RankedCompleterCandidate() : node_index_(0), value_(-1) {}

  void set_node_index(BaseType node_index) {
    node_index_ = node_index;
  }
  void set_value(ValueType value) {
    value_ = value;
  }

  BaseType node_index() const {
    return node_index_;
  }
  ValueType value() const {
    return value_;
  }

  template <typename VALUE_COMPARER_TYPE>
  class Comparer {
   public:
    typedef VALUE_COMPARER_TYPE ValueComparerType;

    explicit Comparer(ValueComparerType value_comparer)
      : value_comparer_(value_comparer) {}

    bool operator()(const RankedCompleterCandidate &lhs,
                    const RankedCompleterCandidate &rhs) const {
      if (lhs.value() != rhs.value()) {
        return value_comparer_(lhs.value(), rhs.value());
      }
      return lhs.node_index() > rhs.node_index();
    }

   private:
    ValueComparerType value_comparer_;
  };

  template <typename VALUE_COMPARER_TYPE>
  static Comparer<VALUE_COMPARER_TYPE> MakeComparer(
      VALUE_COMPARER_TYPE value_comparer) {
    return Comparer<VALUE_COMPARER_TYPE>(value_comparer);
  }

 private:
  BaseType node_index_;
  ValueType value_;

  // Copyable.
};

}  // namespace dawgdic

#endif  // DAWGDIC_RANKED_COMPLETER_CANDIDATE_H
