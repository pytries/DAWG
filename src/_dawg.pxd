from _base_types cimport BaseType, SizeType, ValueType, UCharType

cdef extern from "../lib/dawgdic/dawg.h" namespace "dawgdic":

    cdef cppclass Dawg:
        Dawg()

        # The root index.
        BaseType root() nogil

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

        # Reads values.
        BaseType child(BaseType index) nogil

        BaseType sibling(BaseType index) nogil

        ValueType value(BaseType index) nogil

        bint is_leaf(BaseType index) nogil

        UCharType label(BaseType index) nogil

        bint is_merging(BaseType index) nogil

        # Clears object pools.
        void Clear() nogil

        # Swaps dawgs.
        void Swap(Dawg *dawg) nogil
