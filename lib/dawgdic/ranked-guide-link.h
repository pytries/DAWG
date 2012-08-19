#ifndef DAWGDIC_RANKED_GUIDE_LINK_H
#define DAWGDIC_RANKED_GUIDE_LINK_H

namespace dawgdic {

class RankedGuideLink {
 public:
  RankedGuideLink() : label_('\0'), value_(-1) {}
  RankedGuideLink(UCharType label, ValueType value)
    : label_(label), value_(value) {}

  void set_label(UCharType label) {
    label_ = label;
  }
  void set_value(ValueType value) {
    value_ = value;
  }

  UCharType label() const {
    return label_;
  }
  ValueType value() const {
    return value_;
  }

  // For sortings links in descending value order.
  template <typename VALUE_COMPARER_TYPE>
  class Comparer {
   public:
    typedef VALUE_COMPARER_TYPE ValueComparerType;

    explicit Comparer(ValueComparerType value_comparer)
      : value_comparer_(value_comparer) {}

    bool operator()(const RankedGuideLink &lhs,
                    const RankedGuideLink &rhs) const {
      if (lhs.value() != rhs.value()) {
        return value_comparer_(rhs.value(), lhs.value());
      }
      return lhs.label() < rhs.label();
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
  UCharType label_;
  ValueType value_;

  // Copyable.
};

}  // namespace dawgdic

#endif  // DAWGDIC_RANKED_GUIDE_LINK_H
