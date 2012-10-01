#ifndef DAWGDIC_COMPLETER_H
#define DAWGDIC_COMPLETER_H

#include "dictionary.h"
#include "guide.h"

#include <vector>

namespace dawgdic {

class Completer {
 public:
  Completer()
    : dic_(NULL), guide_(NULL), key_(), index_stack_(), last_index_(0) {}
  Completer(const Dictionary &dic, const Guide &guide)
    : dic_(&dic), guide_(&guide), key_(), index_stack_(), last_index_(0) {}

  void set_dic(const Dictionary &dic) {
    dic_ = &dic;
  }
  void set_guide(const Guide &guide) {
    guide_ = &guide;
  }

  const Dictionary &dic() const {
    return *dic_;
  }
  const Guide &guide() const {
    return *guide_;
  }

  // These member functions are available only when Next() returns true.
  const char *key() const {
    return reinterpret_cast<const char *>(&key_[0]);
  }
  SizeType length() const {
    return key_.size() - 1;
  }
  ValueType value() const {
    return dic_->value(last_index_);
  }

  // Starts completing keys from given index and prefix.
  void Start(BaseType index, const char *prefix = "") {
    SizeType length = 0;
    for (const char *p = prefix; *p != '\0'; ++p) {
      ++length;
    }
    Start(index, prefix, length);
  }
  void Start(BaseType index, const char *prefix, SizeType length) {
    key_.resize(length + 1);
    for (SizeType i = 0; i < length; ++i) {
      key_[i] = prefix[i];
    }
    key_[length] = '\0';

    index_stack_.clear();
    if (guide_->size() != 0) {
      index_stack_.push_back(index);
      last_index_ = dic_->root();
    }
  }

  // Gets the next key.
  bool Next() {
    if (index_stack_.empty()) {
      return false;
    }
    BaseType index = index_stack_.back();

    if (last_index_ != dic_->root()) {
      UCharType child_label = guide_->child(index);
      if (child_label != '\0') {
        // Follows a transition to the first child.
        if (!Follow(child_label, &index))
          return false;
      } else {
        for ( ; ; ) {
          UCharType sibling_label = guide_->sibling(index);

          // Moves to the previous node.
          if (key_.size() > 1) {
            key_.resize(key_.size() - 1);
            key_.back() = '\0';
          }
          index_stack_.resize(index_stack_.size() - 1);
          if (index_stack_.empty()) {
            return false;
          }

          index = index_stack_.back();
          if (sibling_label != '\0') {
            // Follows a transition to the next sibling.
            if (!Follow(sibling_label, &index)) {
              return false;
            }
            break;
          }
        }
      }
    }

    // Finds a terminal.
    return FindTerminal(index);
  }

 private:
  const Dictionary *dic_;
  const Guide *guide_;
  std::vector<UCharType> key_;
  std::vector<BaseType> index_stack_;
  BaseType last_index_;

  // Disallows copies.
  Completer(const Completer &);
  Completer &operator=(const Completer &);

  // Follows a transition.
  bool Follow(UCharType label, BaseType *index) {
    if (!dic_->Follow(label, index)) {
      return false;
    }

    key_.back() = label;
    key_.push_back('\0');
    index_stack_.push_back(*index);
    return true;
  }

  // Finds a terminal.
  bool FindTerminal(BaseType index) {
    while (!dic_->has_value(index)) {
      UCharType label = guide_->child(index);
      if (!dic_->Follow(label, &index)) {
        return false;
      }

      key_.back() = label;
      key_.push_back('\0');
      index_stack_.push_back(index);
    }

    last_index_ = index;
    return true;
  }
};

}  // namespace dawgdic

#endif  // DAWGDIC_COMPLETER_H
