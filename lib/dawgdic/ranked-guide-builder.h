#ifndef DAWGDIC_RANKED_GUIDE_BUILDER_H
#define DAWGDIC_RANKED_GUIDE_BUILDER_H

#include "dawg.h"
#include "dictionary.h"
#include "ranked-guide.h"
#include "ranked-guide-link.h"

#include <algorithm>
#include <functional>
#include <vector>

namespace dawgdic {

class RankedGuideBuilder {
 public:
  // Builds a dictionary for completing keys.
  static bool Build(const Dawg &dawg, const Dictionary &dic,
                    RankedGuide *guide) {
    return Build(dawg, dic, guide, std::less<ValueType>());
  }

  // Builds a dictionary for completing keys.
  template <typename VALUE_COMPARER_TYPE>
  static bool Build(const Dawg &dawg, const Dictionary &dic,
                    RankedGuide *guide, VALUE_COMPARER_TYPE value_comparer) {
    RankedGuideBuilder builder(dawg, dic, guide);
    return builder.BuildRankedGuide(value_comparer);
  }

 private:
  const Dawg &dawg_;
  const Dictionary &dic_;
  RankedGuide *guide_;

  std::vector<RankedGuideUnit> units_;
  std::vector<RankedGuideLink> links_;
  std::vector<UCharType> is_fixed_table_;

  // Disallows copies.
  RankedGuideBuilder(const RankedGuideBuilder &);
  RankedGuideBuilder &operator=(const RankedGuideBuilder &);

  RankedGuideBuilder(const Dawg &dawg, const Dictionary &dic,
                     RankedGuide *guide)
    : dawg_(dawg), dic_(dic), guide_(guide),
      units_(), links_(), is_fixed_table_() {}

  template <typename VALUE_COMPARER_TYPE>
  bool BuildRankedGuide(VALUE_COMPARER_TYPE value_comparer) {
    // Initializes units and flags.
    units_.resize(dic_.size());
    is_fixed_table_.resize(dic_.size() / 8, '\0');

    if (dawg_.size() <= 1) {
      return true;
    }

    ValueType max_value = -1;
    if (!BuildRankedGuide(dawg_.root(), dic_.root(),
                          &max_value, value_comparer)) {
      return false;
    }

    guide_->SwapUnitsBuf(&units_);
    return true;
  }

  // Builds a guide recursively.
  template <typename VALUE_COMPARER_TYPE>
  bool BuildRankedGuide(BaseType dawg_index, BaseType dic_index,
                        ValueType *max_value,
                        VALUE_COMPARER_TYPE value_comparer) {
    if (is_fixed(dic_index)) {
      return FindMaxValue(dic_index, max_value);
    }
    set_is_fixed(dic_index);

    SizeType initial_num_links = links_.size();

    // Enumerates links to the next states.
    if (!EnumerateLinks(dawg_index, dic_index, value_comparer)) {
      return false;
    }

    std::stable_sort(links_.begin() + initial_num_links, links_.end(),
      RankedGuideLink::MakeComparer(value_comparer));

    // Reflects links into units.
    if (!TurnLinksToUnits(dic_index, initial_num_links)) {
      return false;
    }

    *max_value = links_[initial_num_links].value();
    links_.resize(initial_num_links);

    return true;
  }

  // Finds the maximum value by using fixed units.
  bool FindMaxValue(BaseType dic_index, ValueType *max_value) const {
    while (units_[dic_index].child() != '\0') {
      UCharType child_label = units_[dic_index].child();
      if (!dic_.Follow(child_label, &dic_index)) {
        return false;
      }
    }
    if (!dic_.has_value(dic_index)) {
      return false;
    }
    *max_value = dic_.value(dic_index);
    return true;
  }

  // Enumerates links to the next states.
  template <typename VALUE_COMPARER_TYPE>
  bool EnumerateLinks(BaseType dawg_index, BaseType dic_index,
                      VALUE_COMPARER_TYPE value_comparer) {
    for (BaseType dawg_child_index = dawg_.child(dawg_index);
        dawg_child_index != 0;
        dawg_child_index = dawg_.sibling(dawg_child_index)) {
      ValueType value = -1;
      UCharType child_label = dawg_.label(dawg_child_index);
      if (child_label == '\0') {
        if (!dic_.has_value(dic_index)) {
          return false;
        }
        value = dic_.value(dic_index);
      } else {
        BaseType dic_child_index = dic_index;
        if (!dic_.Follow(child_label, &dic_child_index)) {
          return false;
        }

        if (!BuildRankedGuide(dawg_child_index, dic_child_index,
                              &value, value_comparer)) {
          return false;
        }
      }
      links_.push_back(RankedGuideLink(child_label, value));
    }

    return true;
  }

  // Modifies units.
  bool TurnLinksToUnits(BaseType dic_index, SizeType links_begin) {
    // The first child.
    UCharType first_label = links_[links_begin].label();
    units_[dic_index].set_child(first_label);
    BaseType dic_child_index = FollowWithoutCheck(dic_index, first_label);

    // Other children.
    for (SizeType i = links_begin + 1; i < links_.size(); ++i) {
      UCharType sibling_label = links_[i].label();

      BaseType dic_sibling_index =
          FollowWithoutCheck(dic_index, sibling_label);
      units_[dic_child_index].set_sibling(sibling_label);
      dic_child_index = dic_sibling_index;
    }

    return true;
  }

  // Follows a transition without any check.
  BaseType FollowWithoutCheck(BaseType index, UCharType label) const {
    return index ^ dic_.units()[index].offset() ^ label;
  }

  void set_is_fixed(BaseType index) {
    is_fixed_table_[index / 8] |= 1 << (index % 8);
  }

  bool is_fixed(BaseType index) const {
    return (is_fixed_table_[index / 8] & (1 << (index % 8))) != 0;
  }
};

}  // namespace dawgdic

#endif  // DAWGDIC_RANKED_GUIDE_BUILDER_H
