from _base_types cimport BaseType, SizeType, ValueType, UCharType, CharType
from _ranked_guide_unit cimport RankedGuideUnit
from iostream cimport istream, ostream

cdef extern from "../lib/dawgdic/ranked-guide.h" namespace "dawgdic":
    cdef cppclass RankedGuide:

        RankedGuide()

        RankedGuideUnit *units()
        SizeType size()
        SizeType total_size()
        SizeType file_size()

        # The root index
        BaseType root()

        UCharType childe(BaseType index)
        UCharType sibling(BaseType index)

        # Reads a dictionary from an input stream.
        bint Read(istream *input)

        # Writes a dictionry to an output stream.
        bint Write(ostream *output)

        # Maps memory with its size.
        void Map(void *address)

        # Swaps Guides.
        void Swap(RankedGuide *guide)

        # Initializes a Guide.
        void Clear()
