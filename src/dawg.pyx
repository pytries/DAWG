from libcpp.string cimport string
from iostream cimport stringstream, istream, ostream

cimport _dawg
from _dawg_builder cimport DawgBuilder
from _dictionary cimport Dictionary
cimport _dictionary_builder

import operator
import collections


cdef class IntDict:
    cdef Dictionary dct

    def __dealloc__(self):
        self.dct.Clear()

    def __init__(self, arg=None):
        if arg is None:
            arg = []

        if isinstance(arg, collections.Mapping):
            iterable = ((key, arg[key]) for key in arg)
        else:
            iterable = arg

        iterable = sorted(iterable, key=operator.itemgetter(0))
        self._build_from_iterable(iterable)


    cpdef _build_from_iterable(self, iterable):
        cdef DawgBuilder dawg_builder

        cdef bytes b_key
        for key, value in iterable:
            b_key = key.encode('utf8')
            dawg_builder.Insert(b_key, value)

        cdef _dawg.Dawg dawg
        dawg_builder.Finish(&dawg)
        _dictionary_builder.Build(dawg, &(self.dct))


    def __contains__(self, key):
        return self.has_key(key)

    def __getitem__(self, key):
        cdef int res = self._get(key)
        if res == -1:
            raise KeyError(key)
        return res

    cpdef bint has_key(self, unicode key) except -1:
        cdef bytes b_key = key.encode('utf8')
        return self.dct.Contains(b_key)

    cdef int _get(self, unicode key):
        cdef bytes b_key = key.encode('utf8')
        return self.dct.Find(b_key)

    cpdef bytes dumps(self) except +:
        """
        Returns raw DAWG content as bytes.
        """
        cdef stringstream stream
        self.dct.Write(<ostream *> &stream)
        cdef bytes res = stream.str()
        return res

    cpdef loads(self, bytes data) except +:
        """
        Loads DAWG from bytes ``data``.
        """
        cdef stringstream* stream = new stringstream(data)
        try:
            self.dct.Read(<istream *> stream)
        finally:
            del stream

    def load(self, f):
        """
        Loads DAWG from a file-like object.
        """
        self.loads(f.read())

    def dump(self, f):
        """
        Dumps DAWG to a file-like object.
        """
        f.write(self.dumps())


    # pickling support
    def __reduce__(self):
        return self.__class__, tuple(), self.dumps()

    def __setstate__(self, state):
        self.loads(state)


    # some half-internal methods
    def info_size(self):
        return self.dct.size()

    def _total_size(self):
        return self.dct.total_size()

    def _file_size(self):
        return self.dct.file_size()

