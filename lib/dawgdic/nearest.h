// Contributed by Bernhard Liebl, April 2019.
// Based on Steven Hanov's blog article http://stevehanov.ca/blog/?id=114

#include <stack>

namespace dawgdic {

class Nearest {
	typedef int16_t CostType;

	const Dictionary *dic_;
	const Guide *guide_;
	std::stack<UCharType> stack_;
	std::vector<UCharType> key_;
	BaseType index_;
	SizeType columns_;
	std::vector<CostType> rows_;
	std::string word_;
	CostType max_cost_;
	CostType found_cost_;

	enum {
		NEXT_SIBLING,
		NEXT_CHILD
	} state_;

	CostType *GetRow(int n) {
		rows_.resize((n + 1) * columns_);
		return rows_.data() + n * columns_;
	}

	bool Dfs(BaseType index, UCharType letter) {
		const int depth = stack_.size();
		assert(depth >= 1);

		CostType *current_row = GetRow(depth);
		CostType *previous_row = current_row - columns_;
		current_row[0] = previous_row[0] + 1;

		/*for (int i = 0; i < depth; i++) {
			printf("  ");
		}
		printf("%c\n", (char)letter);*/

		CostType smallest = std::numeric_limits<CostType>::max();
		for (SizeType column = 1; column < columns_; column++) {
			int insert_cost = current_row[column - 1] + 1;
			int delete_cost = previous_row[column] + 1;
			int replace_cost;
			if (word_[column - 1] != letter) {
				replace_cost = previous_row[column - 1] + 1;
			} else {
				replace_cost = previous_row[column - 1];
			}
			const CostType cost = std::min(
				insert_cost, std::min(delete_cost, replace_cost));
			current_row[column] = cost;
			smallest = std::min(smallest, cost);
		}

		const CostType best_cost = current_row[columns_ - 1];
		if (best_cost <= max_cost_ && dic_->has_value(index)) {
			found_cost_ = best_cost;
		} else {
			found_cost_ = -1;
		}

		return smallest <= max_cost_; // descend further?
	}

	inline bool Follow(UCharType label, BaseType *index) {
		stack_.push(*index);

		if (!dic_->Follow(label, index)) {
			stack_.pop();
			return false;
		}

		key_.back() = label;
		key_.push_back('\0');

		return true;
	}

public:
	Nearest() :
		dic_(NULL), guide_(NULL) {
	}

	Nearest(const Dictionary &dic, const Guide &guide) :
		dic_(&dic), guide_(&guide) {
	}

	void set_dic(const Dictionary &dic) {
		dic_ = &dic;
	}
	void set_guide(const Guide &guide) {
		guide_ = &guide;
	}

	// These member functions are available only when Next() returns true.
	const char *key() const {
		return reinterpret_cast<const char *>(&key_[0]);
	}
	SizeType length() const {
		return key_.size() - 1;
	}
	ValueType value() const {
		return dic_->value(index_);
	}
	int cost() const {
		return found_cost_;
	}

	void Start(const char *s, int max_cost = 0) {
		word_ = s;
		const SizeType length = word_.length();
		columns_ = length + 1;

		state_ = NEXT_CHILD;
		index_ = dic_->root();

		max_cost_ = CostType(std::max(0, max_cost));
		const int k = int(std::min(max_cost_, CostType(8))) + 1;
		key_.reserve(length + k);
		rows_.reserve((length + k) * columns_);

	    key_.resize(1);
	    key_[0] = '\0';

		int16_t *costs = GetRow(0);
		for (SizeType i = 0; i < columns_; i++) {
			costs[i] = i;
		}
	}

	bool Next() {
		while (true) {
			switch (state_) {
				case NEXT_SIBLING: {
					while (true) {
						const UCharType sibling_label = guide_->sibling(index_);

						if (stack_.empty()) {
							return false;							
						} else {
							index_ = stack_.top();
							stack_.pop();
						}

						key_.pop_back();
						key_.back() = '\0';

						if (sibling_label != '\0') {
							// Follows a transition to the next sibling.
						    if (!Follow(sibling_label, &index_)) {
						    	return false;
						    }
							Dfs(index_, sibling_label);
					    	state_ = NEXT_CHILD;
					    	break;
						}
				    	break;
					}
				} break;
				case NEXT_CHILD: {
					const UCharType child_label = guide_->child(index_);
					if (child_label == '\0') {
						state_ = NEXT_SIBLING;
						break;					
					}
					if (Follow(child_label, &index_)) {
						if (!Dfs(index_, child_label)) {
							state_ = NEXT_SIBLING;
						}
						if (found_cost_ >= 0) {
							return true;
						}
					} else {
						return false;
					}
				} break;
			}
		}

		return false;
	}
};

}
