#ifndef DAWGDIC_BIT_POOL_H
#define DAWGDIC_BIT_POOL_H

#include "object-pool.h"

namespace dawgdic {

// This class works as an array of bit flags with compact memory management.
template <SizeType BLOCK_SIZE = 1 << 10>
class BitPool {
 public:
  BitPool() : pool_(), size_(0) {}

  // Accessors.
  void set(SizeType index, bool bit) {
    SizeType pool_index = PoolIndex(index);
    UCharType bit_flag = BitFlag(index);
    if (bit) {
      pool_[pool_index] |= bit_flag;
    } else {
      pool_[pool_index] &= ~bit_flag;
    }
  }
  bool get(SizeType index) const {
    SizeType pool_index = PoolIndex(index);
    UCharType bit_flag = BitFlag(index);
    return (pool_[pool_index] & bit_flag) ? true : false;
  }

  // Deletes all bits and frees memory.
  void Clear() {
    pool_.Clear();
    size_ = 0;
  }

  // Swaps bit pools.
  void Swap(BitPool *bit_pool) {
    pool_.Swap(&bit_pool->pool_);
  }

  // Allocates memory for a new bit and returns its ID.
  // Note: Allocated bits are filled with false.
  SizeType Allocate() {
    SizeType pool_index = PoolIndex(size_);
    if (pool_index == pool_.size()) {
      pool_.Allocate();
      pool_[pool_index] = '\0';
    }
    return size_++;
  }

 private:
  ObjectPool<UCharType> pool_;
  SizeType size_;

  // Disallows copies.
  BitPool(const BitPool &);
  BitPool &operator=(const BitPool &);

  static SizeType PoolIndex(SizeType index) {
    return index / 8;
  }
  static UCharType BitFlag(BaseType index) {
    return static_cast<UCharType>(1) << (index % 8);
  }
};

}  // namespace dawgdic

#endif  // DAWGDIC_BIT_POOL_H
