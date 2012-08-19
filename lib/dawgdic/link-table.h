#ifndef DAWGDIC_LINK_TABLE_H
#define DAWGDIC_LINK_TABLE_H

#include "base-types.h"
#include "dictionary-unit.h"

#include <vector>

namespace dawgdic {

class LinkTable {
 public:
  explicit LinkTable() : hash_table_() {}

  // Initializes a hash table.
  void Init(SizeType table_size) {
    PairType initial_pair(0, 0);
    std::vector<PairType> table(table_size, initial_pair);
    hash_table_.swap(table);
  }

  // Finds an offset that corresponds to a given index.
  BaseType Find(BaseType index) const {
    BaseType hash_id = FindId(index);
    return hash_table_[hash_id].second;
  }

  // Inserts an index with its offset.
  void Insert(BaseType index, BaseType offset) {
    BaseType hash_id = FindId(index);
    hash_table_[hash_id].first = index;
    hash_table_[hash_id].second = offset;
  }

 private:
  typedef std::pair<BaseType, BaseType> PairType;

  std::vector<PairType> hash_table_;

  // Disallows copies.
  LinkTable(const LinkTable &);
  LinkTable &operator=(const LinkTable &);

  // Finds an Id from an upper table.
  BaseType FindId(BaseType index) const {
    BaseType hash_id = Hash(index) % hash_table_.size();
    while (hash_table_[hash_id].first != 0) {
      if (index == hash_table_[hash_id].first) {
        return hash_id;
      }
      hash_id = (hash_id + 1) % hash_table_.size();
    }
    return hash_id;
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
};

}  // namespace dawgdic

#endif  // DAWGDIC_LINK_TABLE_H
