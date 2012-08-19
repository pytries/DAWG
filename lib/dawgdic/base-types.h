#ifndef DAWGDIC_BASE_TYPES_H
#define DAWGDIC_BASE_TYPES_H

#include <cstddef>

namespace dawgdic {

// 8-bit characters.
typedef char CharType;
typedef unsigned char UCharType;

// 32-bit integer.
typedef int ValueType;

// 32-bit unsigned integer.
typedef unsigned int BaseType;

// 32 or 64-bit unsigned integer.
typedef std::size_t SizeType;

}  // namespace dawgdic

#endif  // DAWGDIC_BASE_TYPES_H
