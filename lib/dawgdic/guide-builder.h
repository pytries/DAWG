#ifndef DAWGDIC_GUIDE_BUILDER_H
#define DAWGDIC_GUIDE_BUILDER_H

#include "guide.h"
#include "dawg.h"
#include "dictionary.h"

#include <vector>

namespace dawgdic {

class GuideBuilder {
 public:
  // Builds a dictionary for completing keys.
  static bool Build(const Dawg &dawg, const Dictionary &dic, Guide *guide) {
    GuideBuilder builder(dawg, dic, guide);
    return builder.BuildGuide();
  }

 private:
  const Dawg &dawg_;
  const Dictionary &dic_;
  Guide *guide_;

  std::vector<GuideUnit> units_;
  std::vector<UCharType> is_fixed_table_;

  // Disallows copies.
  GuideBuilder(const GuideBuilder &);
  GuideBuilder &operator=(const GuideBuilder &);

  GuideBuilder(const Dawg &dawg, const Dictionary &dic, Guide *guide)
    : dawg_(dawg), dic_(dic), guide_(guide), units_(), is_fixed_table_() {}

  bool BuildGuide() {
    // Initializes units and flags.
    units_.resize(dic_.size());
    is_fixed_table_.resize(dic_.size() / 8, '\0');

    if (dawg_.size() <= 1) {
      return true;
    }

    if (!BuildGuide(dawg_.root(), dic_.root())) {
      return false;
    }

    guide_->SwapUnitsBuf(&units_);
    return true;
  }

  // Builds a guide recursively.
  bool BuildGuide(BaseType dawg_index, BaseType dic_index) {
    if (is_fixed(dic_index)) {
      return true;
    }
    set_is_fixed(dic_index);

    // Finds the first non-terminal child.
    BaseType dawg_child_index = dawg_.child(dawg_index);
    if (dawg_.label(dawg_child_index) == '\0') {
      dawg_child_index = dawg_.sibling(dawg_child_index);
      if (dawg_child_index == 0) {
        return true;
      }
    }
    units_[dic_index].set_child(dawg_.label(dawg_child_index));

    do {
      UCharType child_label = dawg_.label(dawg_child_index);
      BaseType dic_child_index = dic_index;
      if (!dic_.Follow(child_label, &dic_child_index)) {
        return false;
      }

      if (!BuildGuide(dawg_child_index, dic_child_index)) {
        return false;
      }

      BaseType dawg_sibling_index = dawg_.sibling(dawg_child_index);
      UCharType sibling_label = dawg_.label(dawg_sibling_index);
      if (dawg_sibling_index != 0) {
        units_[dic_child_index].set_sibling(sibling_label);
      }

      dawg_child_index = dawg_sibling_index;
    } while (dawg_child_index != 0);

    return true;
  }

  void set_is_fixed(BaseType index) {
    is_fixed_table_[index / 8] |= 1 << (index % 8);
  }

  bool is_fixed(BaseType index) const {
    return (is_fixed_table_[index / 8] & (1 << (index % 8))) != 0;
  }
};

}  // namespace dawgdic

#endif  // DAWGDIC_GUIDE_BUILDER_H
