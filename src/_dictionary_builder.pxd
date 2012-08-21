from _base_types cimport BaseType
from _dawg cimport Dawg
from _dictionary cimport Dictionary

cdef extern from "../lib/dawgdic/dictionary-builder.h" namespace "dawgdic::DictionaryBuilder":
    cdef bint Build (Dawg &dawg, Dictionary *dic) nogil

