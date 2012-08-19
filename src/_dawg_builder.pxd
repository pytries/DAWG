from _base_types cimport BaseType, SizeType, ValueType, UCharType, CharType
from _dawg cimport Dawg

cdef extern from "../lib/dawgdic/dawg-builder.h" namespace "dawgdic":
    cdef cppclass DawgBuilder:

        DawgBuilder() nogil  #(SizeType initial_hash_table_size = DEFAULT_INITIAL_HASH_TABLE_SIZE)

        # Number of units.
        SizeType size() nogil

        # Number of transitions.
        SizeType num_of_transitions() nogil

        # Number of states.
        SizeType num_of_states() nogil

        # Number of merged transitions.
        SizeType num_of_merged_transitions() nogil

        # Number of merged states.
        SizeType num_of_merged_states() nogil

        # Number of merging states.
        SizeType num_of_merging_states() nogil

        # Initializes a builder.
        void Clear() nogil

        # Inserts a key.
        bint Insert(CharType *key)
        bint Insert(CharType *key, ValueType value)
        bint Insert(CharType *key, SizeType length, ValueType value)

        # Finishes building a dawg.
        bint Finish(Dawg *dawg)
