#ifndef DAWGDIC_RANKED_COMPLETER_NODE_H
#define DAWGDIC_RANKED_COMPLETER_NODE_H

#include "base-types.h"

namespace dawgdic {

class RankedCompleterNode {
 public:
  RankedCompleterNode()
    : dic_index_(0), prev_node_index_(0),
      label_('\0'), is_queued_(false), has_terminal_(false) {}

  void set_dic_index(BaseType dic_index) {
    dic_index_ = dic_index;
  }
  void set_prev_node_index(BaseType prev_node_index) {
    prev_node_index_ = prev_node_index;
  }
  void set_label(UCharType label) {
    label_ = label;
  }
  void set_is_queued() {
    is_queued_ = true;
  }
  void set_has_terminal(bool has_terminal) {
    has_terminal_ = has_terminal;
  }

  BaseType dic_index() const {
    return dic_index_;
  }
  BaseType prev_node_index() const {
    return prev_node_index_;
  }
  UCharType label() const {
    return label_;
  }
  bool is_queued() const {
    return is_queued_;
  }
  bool has_terminal() const {
    return has_terminal_;
  }

 private:
  BaseType dic_index_;
  BaseType prev_node_index_;
  UCharType label_;
  bool is_queued_;
  bool has_terminal_;

  // Copyable.
};

}  // namespace dawgdic

#endif  // DAWGDIC_RANKED_COMPLETER_NODE_H
