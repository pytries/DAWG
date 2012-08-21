from libcpp.string cimport string
from iostream cimport stringstream, istream, ostream

cimport _dawg
from _dawg_builder cimport DawgBuilder
from _dictionary cimport Dictionary
cimport _dictionary_builder

import operator
import collections

cdef class DAWG:
    """
    Base DAWG wrapper.
    """
    cdef Dictionary dct

    def __dealloc__(self):
        self.dct.Clear()

    def __init__(self, arg=None):
        if arg is None:
            arg = []
        self._build_from_keys_iterable(sorted(list(arg)))


    cpdef _build_from_keys_iterable(self, iterable):
        cdef DawgBuilder dawg_builder

        cdef bytes b_key
        for key in iterable:
            if isinstance(key, unicode):
                b_key = key.encode('utf8')
            else:
                b_key = key
            dawg_builder.Insert(b_key)

        cdef _dawg.Dawg dawg
        dawg_builder.Finish(&dawg)
        _dictionary_builder.Build(dawg, &(self.dct))

    def __contains__(self, key):
        if isinstance(key, unicode):
            return self.has_key(key)
        return self.b_has_key(key)

    cpdef bint has_key(self, unicode key) except -1:
        cdef bytes b_key = key.encode('utf8')
        return self.dct.Contains(b_key)

    cpdef bint b_has_key(self, bytes key) except -1:
        return self.dct.Contains(key)

    cpdef bytes tobytes(self) except +:
        """
        Returns raw DAWG content as bytes.
        """
        cdef stringstream stream
        self.dct.Write(<ostream *> &stream)
        cdef bytes res = stream.str()
        return res

    cpdef frombytes(self, bytes data) except +:
        """
        Loads DAWG from bytes ``data``.
        """
        cdef stringstream* stream = new stringstream(data)
        try:
            self.dct.Read(<istream *> stream)
        finally:
            del stream
        return self

    def read(self, f):
        """
        Loads DAWG from a file-like object.

        FIXME: this method should'n read the whole stream.
        """
        self.frombytes(f.read())

    def write(self, f):
        """
        Writes DAWG to a file-like object.
        """
        f.write(self.tobytes())

    def load(self, path):
        """
        Loads DAWG from a file.
        """
        with open(path, 'r') as f:
            self.read(f)

    def save(self, path):
        """
        Saves DAWG to a file.
        """
        with open(path, 'w') as f:
            self.write(f)


    # pickling support
    def __reduce__(self):
        return self.__class__, tuple(), self.tobytes()

    def __setstate__(self, state):
        self.frombytes(state)

    # half-internal methods
    def _size(self):
        return self.dct.size()

    def _total_size(self):
        return self.dct.total_size()

    def _file_size(self):
        return self.dct.file_size()



cdef class IntDict(DAWG):
    """
    Dict-like class based on DAWG.
    It can store integer values for unicode keys.
    """
    def __init__(self, arg=None):
        if arg is None:
            arg = []

        if isinstance(arg, collections.Mapping):
            iterable = ((key, arg[key]) for key in arg)
        else:
            iterable = arg

        iterable = sorted(iterable, key=operator.itemgetter(0))
        self._build_from_key_value_iterable(iterable)


    cpdef _build_from_key_value_iterable(self, iterable):
        cdef DawgBuilder dawg_builder

        cdef bytes b_key
        for key, value in iterable:
            b_key = key.encode('utf8')
            dawg_builder.Insert(b_key, value)

        cdef _dawg.Dawg dawg
        dawg_builder.Finish(&dawg)
        _dictionary_builder.Build(dawg, &(self.dct))


    def __getitem__(self, key):
        cdef int res

        if isinstance(key, unicode):
            res = self.get_value(key)
        else:
            res = self.b_get_value(key)

        if res == -1:
            raise KeyError(key)
        return res


    cpdef int get_value(self, unicode key):
        cdef bytes b_key = key.encode('utf8')
        return self.dct.Find(b_key)

    cpdef int b_get_value(self, bytes key):
        return self.dct.Find(key)