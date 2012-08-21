from _base_types cimport BaseType
from _dawg cimport Dawg
from _dictionary cimport Dictionary
from _guide cimport Guide

cdef extern from "../lib/dawgdic/guide-builder.h" namespace "dawgdic::GuideBuilder":
    cdef bint Build (Dawg &dawg, Dictionary &dic, Guide* guide) nogil

