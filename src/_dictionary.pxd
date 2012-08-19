from _base_types cimport BaseType, SizeType, ValueType, UCharType, CharType
from _dictionary_unit cimport DictionaryUnit
from iostream cimport istream, ostream

cdef extern from "../lib/dawgdic/dictionary.h" namespace "dawgdic":
    cdef cppclass Dictionary:

        Dictionary() nogil

        DictionaryUnit *units() nogil
        SizeType size() nogil
        SizeType total_size() nogil
        SizeType file_size() nogil

        # Root index.
        BaseType root() nogil

        # Checks if a given index is related to the end of a key.
        bint has_value(BaseType index) nogil

        # Gets a value from a given index.
        ValueType value(BaseType index) nogil

        # Reads a dictionary from an input stream.
        bint Read(istream *input) nogil except +

        # Writes a dictionry to an output stream.
        bint Write(ostream *output) nogil except +

        # Exact matching.
        bint Contains(CharType *key) nogil
        bint Contains(CharType *key, SizeType length) nogil

        # Exact matching.
        ValueType Find(CharType *key) nogil
        ValueType Find(CharType *key, SizeType length) nogil
        bint Find(CharType *key, ValueType *value) nogil
        bint Find(CharType *key, SizeType length, ValueType *value) nogil

        # Follows a transition.
        bint Follow(CharType label, BaseType *index) nogil

        # Follows transitions.
        bint Follow(CharType *s, BaseType *index) nogil
        bint Follow(CharType *s, BaseType *index, SizeType *count) nogil

        # Follows transitions.
        bint Follow(CharType *s, SizeType length, BaseType *index) nogil
        bint Follow(CharType *s, SizeType length, BaseType *index, SizeType *count) nogil

        # Maps memory with its size.
        void Map(void *address) nogil
        void Map(void *address, SizeType size) nogil

        # Initializes a dictionary.
        void Clear() nogil

        # Swaps dictionaries.
        void Swap(Dictionary *dic) nogil
        # Shrinks a vector.
        void Shrink() nogil