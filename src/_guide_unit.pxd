from _base_types cimport BaseType, SizeType, ValueType, UCharType, CharType

cdef extern from "../lib/dawgdic/guide-unit.h" namespace "dawgdic":
    cdef cppclass GuideUnit:
        GuideUnit() nogil

        void set_child(UCharType child) nogil
        void set_sibling(UCharType sibling) nogil
        UCharType child() nogil
        UCharType sibling() nogil
