#ifndef DAWGDIC_OBJECT_POOL_H
#define DAWGDIC_OBJECT_POOL_H

#include <vector>

#include "base-types.h"

namespace dawgdic {

// This class works like an array of objects with compact memory management.
template <typename OBJECT_TYPE, SizeType BLOCK_SIZE = 1 << 10>
class ObjectPool {
 public:
  typedef OBJECT_TYPE ObjectType;

  ObjectPool() : blocks_(), size_(0) {}
  ~ObjectPool() {
    Clear();
  }

  // Accessors.
  ObjectType &operator[](SizeType index) {
    return blocks_[index / BLOCK_SIZE][index % BLOCK_SIZE];
  }
  const ObjectType &operator[](SizeType index) const {
    return blocks_[index / BLOCK_SIZE][index % BLOCK_SIZE];
  }

  // Number of allocated objects.
  SizeType size() const {
    return size_;
  }

  // Deletes all objects and frees memory.
  void Clear() {
    for (SizeType i = 0; i < blocks_.size(); ++i) {
      delete [] blocks_[i];
    }

    std::vector<ObjectType *>(0).swap(blocks_);
    size_ = 0;
  }

  // Swaps object pools.
  void Swap(ObjectPool *pool) {
    blocks_.swap(pool->blocks_);
    std::swap(size_, pool->size_);
  }

  // Allocates memory for a new object and returns its ID.
  SizeType Allocate() {
    if (size_ == BLOCK_SIZE * blocks_.size()) {
      blocks_.push_back(new ObjectType[BLOCK_SIZE]);
    }
    return size_++;
  }

 private:
  std::vector<ObjectType *> blocks_;
  SizeType size_;

  // Disallows copies.
  ObjectPool(const ObjectPool &);
  ObjectPool &operator=(const ObjectPool &);
};

}  // namespace dawgdic

#endif  // DAWGDIC_OBJECT_POOL_H
