#ifndef DAWGDIC_RANKED_COMPLETER_H
#define DAWGDIC_RANKED_COMPLETER_H

#include "dictionary.h"
#include "ranked-completer-candidate.h"
#include "ranked-completer-node.h"
#include "ranked-guide.h"

#include <algorithm>
#include <functional>
#include <queue>
#include <vector>

namespace dawgdic {

template <typename VALUE_COMPARER_TYPE = std::less<ValueType> >
class RankedCompleterBase {
 public:
  typedef VALUE_COMPARER_TYPE ValueComparerType;

  explicit RankedCompleterBase(
      ValueComparerType value_comparer = ValueComparerType())
    : dic_(NULL), guide_(NULL), key_(), prefix_length_(0), value_(-1),
      nodes_(), node_queue_(), candidate_queue_(
          RankedCompleterCandidate::MakeComparer(value_comparer)) {}
  RankedCompleterBase(const Dictionary &dic, const RankedGuide &guide,
      ValueComparerType value_comparer = ValueComparerType())
    : dic_(&dic), guide_(&guide), key_(), prefix_length_(0), value_(-1),
      nodes_(), node_queue_(), candidate_queue_(
          RankedCompleterCandidate::MakeComparer(value_comparer)) {}

  void set_dic(const Dictionary &dic) {
    dic_ = &dic;
  }
  void set_guide(const RankedGuide &guide) {
    guide_ = &guide;
  }

  const Dictionary &dic() const {
    return *dic_;
  }
  const RankedGuide &guide() const {
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
    return value_;
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
    key_.resize(length);
    for (SizeType i = 0; i < length; ++i) {
      key_[i] = prefix[i];
    }
    prefix_length_ = length;
    value_ = -1;

    nodes_.clear();
    node_queue_.clear();
    while (!candidate_queue_.empty()) {
      candidate_queue_.pop();
    }

    if (guide_->size() != 0) {
      CreateNode(index, 0, 'X');
      EnqueueNode(0);
    }
  }

  // Gets the next key.
  bool Next() {
    for (SizeType i = 0; i < node_queue_.size(); ++i) {
      BaseType node_index = node_queue_[i];
      if (value_ != -1 && !FindSibling(&node_index)) {
        continue;
      }
      node_index = FindTerminal(node_index);
      EnqueueCandidate(node_index);
    }
    node_queue_.clear();

    // Returns false if there is no candidate.
    if (candidate_queue_.empty()) {
      return false;
    }

    const RankedCompleterCandidate &candidate = candidate_queue_.top();

    BaseType node_index = candidate.node_index();
    EnqueueNode(node_index);
    node_index = nodes_[node_index].prev_node_index();

    key_.resize(prefix_length_);
    while (node_index != 0) {
      key_.push_back(nodes_[node_index].label());
      EnqueueNode(node_index);
      node_index = nodes_[node_index].prev_node_index();
    }
    std::reverse(key_.begin() + prefix_length_, key_.end());
    key_.push_back('\0');

    value_ = candidate.value();
    candidate_queue_.pop();

    return true;
  }

 private:
  const Dictionary *dic_;
  const RankedGuide *guide_;
  std::vector<UCharType> key_;
  SizeType prefix_length_;
  ValueType value_;

  std::vector<RankedCompleterNode> nodes_;
  std::vector<BaseType> node_queue_;
  std::priority_queue<RankedCompleterCandidate,
                      std::vector<RankedCompleterCandidate>,
                      RankedCompleterCandidate::Comparer<ValueComparerType> >
      candidate_queue_;

  // Disallows copies.
  RankedCompleterBase(const RankedCompleterBase &);
  RankedCompleterBase &operator=(const RankedCompleterBase &);

  // Pushes a node to queue.
  void EnqueueNode(BaseType node_index) {
    if (nodes_[node_index].is_queued()) {
      return;
    }

    node_queue_.push_back(node_index);
    nodes_[node_index].set_is_queued();
  }

  // Pushes a candidate to priority queue.
  void EnqueueCandidate(BaseType node_index) {
    RankedCompleterCandidate candidate;
    candidate.set_node_index(node_index);
    candidate.set_value(
        dic_->units()[nodes_[node_index].dic_index()].value());
    candidate_queue_.push(candidate);
  }

  // Finds a sibling of a given node.
  bool FindSibling(BaseType *node_index) {
    BaseType prev_node_index = nodes_[*node_index].prev_node_index();
    BaseType dic_index = nodes_[*node_index].dic_index();

    UCharType sibling_label = guide_->sibling(dic_index);
    if (sibling_label == '\0') {
      if (!nodes_[prev_node_index].has_terminal()) {
        return false;
      }
      nodes_[prev_node_index].set_has_terminal(false);
    }

    // Follows a transition to sibling and creates a node for the sibling.
    BaseType dic_prev_index = nodes_[prev_node_index].dic_index();
    dic_index = FollowWithoutCheck(dic_prev_index, sibling_label);
    *node_index = CreateNode(dic_index, prev_node_index, sibling_label);

    return true;
  }

  // Follows transitions and finds a terminal.
  BaseType FindTerminal(BaseType node_index) {
    while (nodes_[node_index].label() != '\0') {
      BaseType dic_index = nodes_[node_index].dic_index();
      UCharType child_label = guide_->child(dic_index);
      if (child_label == '\0') {
        nodes_[node_index].set_has_terminal(false);
      }

      // Follows a transition to child and creates a node for the child.
      dic_index = FollowWithoutCheck(dic_index, child_label);
      node_index = CreateNode(dic_index, node_index, child_label);
    }
    return node_index;
  }

  // Follows a transition without any check.
  BaseType FollowWithoutCheck(BaseType index, UCharType label) const {
    return index ^ dic_->units()[index].offset() ^ label;
  }

  // Creates a node.
  BaseType CreateNode(BaseType dic_index, BaseType prev_node_index,
                      UCharType label) {
    RankedCompleterNode node;
    node.set_dic_index(dic_index);
    node.set_prev_node_index(prev_node_index);
    node.set_label(label);
    if (node.label() != '\0') {
      node.set_has_terminal(dic_->has_value(node.dic_index()));
    }
    nodes_.push_back(node);

    return static_cast<BaseType>(nodes_.size() - 1);
  }
};

typedef RankedCompleterBase<> RankedCompleter;

}  // namespace dawgdic

#endif  // DAWGDIC_RANKED_COMPLETER_H
