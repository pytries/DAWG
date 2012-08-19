cdef extern from "../lib/dawgdic/base-types.h" namespace "dawgdic":
    # 8-bit characters.
    ctypedef char CharType
    ctypedef unsigned char UCharType

    # 32-bit integer.
    ctypedef int ValueType

    # 32-bit unsigned integer.
    ctypedef unsigned int BaseType

    # 32 or 64-bit unsigned integer.
    ctypedef int SizeType

