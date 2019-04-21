from _base_types cimport BaseType, SizeType, ValueType
from _dawg cimport Dawg
from _dictionary cimport Dictionary
from _guide cimport Guide

cdef extern from "../lib/dawgdic/nearest.h" namespace "dawgdic" nogil:
    cdef cppclass Nearest:
        Nearest()
        Nearest(Dictionary &dic, Guide &guide)

        void set_dic(Dictionary &dic)
        void set_guide(Guide &guide)

        Dictionary &dic()
        Guide &guide()

        # These member functions are available only when Next() returns true.
        char *key()
        SizeType length()
        ValueType value()
        int cost()

        # Starts completing keys from given index and prefix.
        void Start(char *s, int max_cost)

        # Gets the next key.
        bint Next()