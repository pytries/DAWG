from _base_types cimport BaseType, SizeType, ValueType, UCharType, CharType

cdef extern from "../lib/dawgdic/dictionary-unit.h" namespace "dawgdic":
    cdef cppclass DictionaryUnit:

        DictionaryUnit() nogil

        # Sets a flag to show that a unit has a leaf as a child.
        void set_has_leaf() nogil

        # Sets a value to a leaf unit.
        void set_value(ValueType value) nogil

        # Sets a label to a non-leaf unit.
        void set_label(UCharType label) nogil

        # Sets an offset to a non-leaf unit.
        bint set_offset(BaseType offset) nogil


        # Checks if a unit has a leaf as a child or not.
        bint has_leaf() nogil

        # Checks if a unit corresponds to a leaf or not.
        ValueType value() nogil

        # Reads a label with a leaf flag from a non-leaf unit.
        BaseType label() nogil

        # Reads an offset to child units from a non-leaf unit.
        BaseType offset() nogil
