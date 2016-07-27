from _base_types cimport BaseType, SizeType, ValueType
from _dawg cimport Dawg
from _dictionary cimport Dictionary
from _ranked_guide cimport RankedGuide

cdef extern from "../lib/dawgdic/ranked-completer.h" namespace "dawgdic" nogil:
    cdef cppclass RankedCompleter:
        RankedCompleter()
        RankedCompleter(Dictionary &dic, RankedGuide &guide)

        void set_dic(Dictionary &dic)
        void set_guide(RankedGuide &guide)

        Dictionary &dic()
        RankedGuide &guide()

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
