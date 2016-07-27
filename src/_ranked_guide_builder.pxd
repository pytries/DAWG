from _base_types cimport BaseType
from _dawg cimport Dawg
from _dictionary cimport Dictionary
from _ranked_guide cimport RankedGuide

cdef extern from "../lib/dawgdic/ranked-guide-builder.h" namespace "dawgdic::RankedGuideBuilder":
    cdef bint Build(Dawg &dawg, Dictionary &dic, RankedGuide* guide) nogil
