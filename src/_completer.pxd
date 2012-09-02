from _base_types cimport BaseType, SizeType, ValueType
from _dawg cimport Dawg
from _dictionary cimport Dictionary
from _guide cimport Guide

cdef extern from "../lib/dawgdic/completer.h" namespace "dawgdic" nogil:
    cdef cppclass Completer:
        Completer()
        Completer(Dictionary &dic, Guide &guide)

        void set_dic(Dictionary &dic)
        void set_guide(Guide &guide)

        Dictionary &dic()
        Guide &guide()

        # These member functions are available only when Next() returns true.
        char *key()
        SizeType length()
        ValueType value()

        # Starts completing keys from given index and prefix.
        void Start(BaseType index)
        void Start(BaseType index, char *prefix)
        void Start(BaseType index, char *prefix, SizeType length)

        # Gets the next key.
        bint Next()