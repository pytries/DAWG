# cython: profile=False
# cython: embedsignature=True
from __future__ import unicode_literals
from libcpp.string cimport string
from libcpp.vector cimport vector
from iostream cimport stringstream, istream, ostream, ifstream
cimport iostream

cimport _dawg
from _dawg_builder cimport DawgBuilder
from _dictionary cimport Dictionary
from _guide cimport Guide
from _completer cimport Completer
from _base_types cimport BaseType, SizeType, CharType
cimport _guide_builder
cimport _dictionary_builder
cimport b64_decode

try:
    from collections.abc import Mapping
except ImportError:
    # Python 2.7
    from collections import Mapping
import struct
import sys
from binascii import b2a_base64


class Error(Exception):
    pass


cdef class DAWG:
    """
    Base DAWG wrapper.
    """
    cdef Dictionary dct
    cdef _dawg.Dawg dawg

    def __init__(self, arg=None, input_is_sorted=False):
        if arg is None:
            arg = []
        if not input_is_sorted:
            arg = [
                (<unicode>key).encode('utf8') if isinstance(key, unicode) else key
                for key in arg
            ]
            arg.sort()
        self._build_from_iterable(arg)

    def __dealloc__(self):
        self.dct.Clear()
        self.dawg.Clear()

    def _build_from_iterable(self, iterable):
        cdef DawgBuilder dawg_builder
        cdef bytes b_key
        cdef int value

        for key in iterable:
            if isinstance(key, tuple) or isinstance(key, list):
                key, value = key
                if value < 0:
                    raise ValueError("Negative values are not supported")
            else:
                value = 0

            if isinstance(key, unicode):
                b_key = <bytes>(<unicode>key).encode('utf8')
            else:
                b_key = key

            if not dawg_builder.Insert(b_key, len(b_key), value):
                raise Error("Can't insert key %r (with value %r)" % (b_key, value))

        if not dawg_builder.Finish(&self.dawg):
            raise Error("dawg_builder.Finish error")

        if not _dictionary_builder.Build(self.dawg, &self.dct):
            raise Error("Can't build dictionary")

    def __contains__(self, key):
        if isinstance(key, unicode):
            return self.has_key(<unicode>key)
        return self.b_has_key(key)

    cpdef bint has_key(self, unicode key) except -1:
        return self.b_has_key(<bytes>key.encode('utf8'))

    cpdef bint b_has_key(self, bytes key) except -1:
        return self.dct.Contains(key, len(key))

    cpdef bytes tobytes(self) except +:
        """
        Return raw DAWG content as bytes.
        """
        cdef stringstream stream
        self.dct.Write(<ostream *> &stream)
        cdef bytes res = stream.str()
        return res

    cpdef frombytes(self, bytes data):
        """
        Load DAWG from bytes ``data``.

        FIXME: it seems there is a memory leak here (DAWG uses 3x memory
        when loaded using ``.frombytes`` compared to DAWG loaded
        using ``.load``).
        """
        cdef string s_data = data
        cdef stringstream* stream = new stringstream(s_data)

        try:
            res = self.dct.Read(<istream *> stream)

            if not res:
                self.dct.Clear()
                raise IOError("Invalid data format")

            return self
        finally:
            del stream

    def read(self, f):
        """
        Load DAWG from a file-like object.

        FIXME: this method should'n read the whole stream.
        """
        self.frombytes(f.read())

    def write(self, f):
        """
        Write DAWG to a file-like object.
        """
        f.write(self.tobytes())

    def load(self, path):
        """
        Load DAWG from a file.
        """
        if isinstance(path, unicode):
            path = path.encode(sys.getfilesystemencoding())

        cdef ifstream stream
        stream.open(path, iostream.binary)
        if stream.fail():
            raise IOError("It's not possible to read file stream")

        res = self.dct.Read(<istream*> &stream)

        stream.close()

        if not res:
            self.dct.Clear()
            raise IOError("Invalid data format")

        return self

    def save(self, path):
        """
        Save DAWG to a file.
        """
        with open(path, 'wb') as f:
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

    cdef bint _has_value(self, BaseType index):
        return  self.dct.has_value(index)

    cdef list _similar_keys(self, unicode current_prefix, unicode key, BaseType cur_index, dict replace_chars):
        cdef BaseType next_index, index = cur_index
        cdef unicode prefix, u_replace_char, found_key
        cdef bytes b_step, b_replace_char
        cdef list res = []
        cdef list extra_keys

        cdef int start_pos = len(current_prefix)
        cdef int end_pos = len(key)
        cdef int word_pos = start_pos

        while word_pos < end_pos:
            b_step = <bytes>(key[word_pos].encode('utf8'))

            if b_step in replace_chars:
                next_index = index
                b_replace_char, u_replace_char = <tuple>replace_chars[b_step]

                if self.dct.Follow(b_replace_char, &next_index):
                    prefix = current_prefix + key[start_pos:word_pos] + u_replace_char
                    extra_keys = self._similar_keys(prefix, key, next_index, replace_chars)
                    res.extend(extra_keys)

            if not self.dct.Follow(b_step, &index):
                break
            word_pos += 1

        else:
            if self._has_value(index):
                found_key = current_prefix + key[start_pos:]
                res.insert(0, found_key)

        return res

    cpdef list similar_keys(self, unicode key, dict replaces):
        """
        Return all variants of ``key`` in this DAWG according to
        ``replaces``.

        ``replaces`` is an object obtained from
        ``DAWG.compile_replaces(mapping)`` where mapping is a dict
        that maps single-char unicode sitrings to another single-char
        unicode strings.

        This may be useful e.g. for handling single-character umlauts.
        """
        return self._similar_keys("", key, self.dct.root(), replaces)

    cpdef list prefixes(self, unicode key):
        '''
        Return a list with keys of this DAWG that are prefixes of the ``key``.
        '''
        return [p.decode('utf8') for p in self.b_prefixes(<bytes>key.encode('utf8'))]

    cpdef list b_prefixes(self, bytes b_key):
        cdef list res = []
        cdef BaseType index = self.dct.root()
        cdef int pos = 1
        cdef CharType ch

        for ch in b_key:
            if not self.dct.Follow(ch, &index):
                break
            if self._has_value(index):
                res.append(b_key[:pos])
            pos += 1

        return res

    def iterprefixes(self, unicode key):
        '''
        Return a generator with keys of this DAWG that are prefixes of the ``key``.
        '''
        cdef BaseType index = self.dct.root()
        cdef bytes b_key = <bytes>key.encode('utf8')
        cdef int pos = 1
        cdef CharType ch

        for ch in b_key:
            if not self.dct.Follow(ch, &index):
                return
            if self._has_value(index):
                yield b_key[:pos].decode('utf8')
            pos += 1

    @classmethod
    def compile_replaces(cls, replaces):

        for k,v in replaces.items():
            if len(k) != 1 or len(v) != 1:
                raise ValueError("Keys and values must be single-char unicode strings.")

        return dict(
            (
                k.encode('utf8'),
                (v.encode('utf8'), unicode(v))
            )
            for k, v in replaces.items()
        )


cdef void init_completer(Completer& completer, Dictionary& dic, Guide& guide):
    completer.set_dic(dic)
    completer.set_guide(guide)


cdef class CompletionDAWG(DAWG):
    """
    DAWG with key completion support.
    """
    cdef Guide guide

    def __init__(self, arg=None, input_is_sorted=False):
        super(CompletionDAWG, self).__init__(arg, input_is_sorted)
        if not _guide_builder.Build(self.dawg, self.dct, &self.guide):
            raise Error("Error building completion information")

    def __dealloc__(self):
        self.guide.Clear()

    cpdef list keys(self, unicode prefix=""):
        cdef bytes b_prefix = prefix.encode('utf8')
        cdef BaseType index = self.dct.root()
        cdef list res = []

        if not self.dct.Follow(b_prefix, &index):
            return res

        cdef Completer completer
        init_completer(completer, self.dct, self.guide)
        completer.Start(index, b_prefix)

        while completer.Next():
            key = (<char*>completer.key()).decode('utf8')
            res.append(key)

        return res

    def iterkeys(self, unicode prefix=""):
        cdef bytes b_prefix = prefix.encode('utf8')
        cdef BaseType index = self.dct.root()

        if not self.dct.Follow(b_prefix, &index):
            return

        cdef Completer completer
        init_completer(completer, self.dct, self.guide)
        completer.Start(index, b_prefix)

        while completer.Next():
            key = (<char*>completer.key()).decode('utf8')
            yield key

    def has_keys_with_prefix(self, unicode prefix):
        cdef bytes b_prefix = prefix.encode('utf8')
        cdef BaseType index = self.dct.root()

        if not self.dct.Follow(b_prefix, &index):
            return False

        cdef Completer completer
        init_completer(completer, self.dct, self.guide)
        completer.Start(index, b_prefix)

        return completer.Next()

    cpdef bytes tobytes(self) except +:
        """
        Return raw DAWG content as bytes.
        """
        cdef stringstream stream
        self.dct.Write(<ostream *> &stream)
        self.guide.Write(<ostream *> &stream)
        cdef bytes res = stream.str()
        return res

    cpdef frombytes(self, bytes data):
        """
        Load DAWG from bytes ``data``.

        FIXME: it seems there is memory leak here (DAWG uses 3x memory when
        loaded using frombytes vs load).
        """
        cdef char* c_data = data
        cdef stringstream stream
        stream.write(c_data, len(data))
        stream.seekg(0)

        res = self.dct.Read(<istream*> &stream)
        if not res:
            self.dct.Clear()
            raise IOError("Invalid data format: can't load _dawg.Dictionary")

        res = self.guide.Read(<istream*> &stream)
        if not res:
            self.guide.Clear()
            self.dct.Clear()
            raise IOError("Invalid data format: can't load _dawg.Guide")

        return self

    def load(self, path):
        """
        Load DAWG from a file.
        """
        if isinstance(path, unicode):
            path = path.encode(sys.getfilesystemencoding())

        cdef ifstream stream
        stream.open(path, iostream.binary)
        if stream.fail():
            raise IOError("It's not possible to read file stream")

        try:
            res = self.dct.Read(<istream*> &stream)
            if not res:
                self.dct.Clear()
                raise IOError("Invalid data format: can't load _dawg.Dictionary")

            res = self.guide.Read(<istream*> &stream)
            if not res:
                self.guide.Clear()
                self.dct.Clear()
                raise IOError("Invalid data format: can't load _dawg.Guide")

        finally:
            stream.close()

        return self

    def _transitions(self):
        transitions = set()
        cdef BaseType index, prev_index, completer_index
        cdef char* key

        cdef Completer completer
        init_completer(completer, self.dct, self.guide)
        completer.Start(self.dct.root())

        while completer.Next():
            key = <char*>completer.key()

            index = self.dct.root()

            for i in range(completer.length()):
                prev_index = index
                self.dct.Follow(&(key[i]), 1, &index)
                transitions.add(
                    (prev_index, <unsigned char>key[i], index)
                )

        return sorted(list(transitions))


# The following symbol is not allowed in utf8 so it is safe to use
# as a separator between utf8-encoded string and binary payload.
# It has drawbacks however: sorting of utf8-encoded keys changes:
# ('foo' becomes greater than 'foox' because strings are compared as
# 'foo<sep>' and 'foox<sep>' and ord(<sep>)==255 is greater than
# ord(<any other character>).
# DEF PAYLOAD_SEPARATOR = b'\xff'

# That's why chr(1) is used as separator by default: this is the lowest allowed
# character and so it will preserve keys alphabetical order.
# It is not strictly correct to use chr(1) as separator because chr(1)
# is a valid UTF8 character. But I think in practice this won't be an issue:
# such control character is very unlikely in text keys, and binary keys
# are not supported anyway because dawgdic doesn't support keys containing
# chr(0).
cdef bytes PAYLOAD_SEPARATOR = b'\x01'

DEF MAX_VALUE_SIZE = 32768

cdef class BytesDAWG(CompletionDAWG):
    """
    DAWG that is able to transparently store extra binary payload in keys;
    there may be several payloads for the same key.

    In other words, this class implements read-only DAWG-based
    {unicode -> list of bytes objects} mapping.
    """

    cdef bytes _b_payload_separator
    cdef CharType _c_payload_separator
    cdef Completer* _completer

    def __init__(self, arg=None, input_is_sorted=False, bytes payload_separator=PAYLOAD_SEPARATOR):
        """
        ``arg`` must be an iterable of tuples (unicode_key, bytes_payload).
        """
        if arg is None:
            arg = []

        self._b_payload_separator = payload_separator
        self._c_payload_separator = <unsigned int>ord(payload_separator)

        keys = (self._raw_key(d[0], d[1]) for d in arg)
        super(BytesDAWG, self).__init__(keys, input_is_sorted)

        self._update_completer()

    def __dealloc__(self):
        if self._completer:
            del self._completer

    cpdef bytes _raw_key(self, unicode key, bytes payload):
        cdef bytes b_key = <bytes>key.encode('utf8')

        if self._b_payload_separator in b_key:
            raise Error("Payload separator (%r) is found within utf8-encoded key ('%s')" % (self._b_payload_separator, key))

        cdef bytes encoded_payload = b2a_base64(payload)
        return b_key + self._b_payload_separator + encoded_payload

    cdef _update_completer(self):
        if self._completer:
            del self._completer
        self._completer = new Completer(self.dct, self.guide)

    def load(self, path):
        res = super(BytesDAWG, self).load(path)
        self._update_completer()
        return res

    cpdef frombytes(self, bytes data):
        res = super(BytesDAWG, self).frombytes(data)
        self._update_completer()
        return res

    cpdef bint b_has_key(self, bytes key) except -1:
        cdef BaseType index
        return self._follow_key(key, &index)

    def __getitem__(self, key):
        res = self.get(key)
        if res is None:
            raise KeyError(key)
        return res

    cpdef get(self, key, default=None):
        """
        Return a list of payloads (as byte objects) for a given key
        or ``default`` if the key is not found.
        """
        if isinstance(key, unicode):
            res = self.get_value(<unicode>key)
        else:
            res = self.b_get_value(key)

        if not res:
            return default
        return res

    cdef bint _follow_key(self, bytes key, BaseType* index):
        index[0] = self.dct.root()
        if not self.dct.Follow(key, len(key), index):
            return False
        return self.dct.Follow(self._c_payload_separator, index)

    cpdef list get_value(self, unicode key):
        return self.b_get_value(<bytes>key.encode('utf8'))

    cdef list _value_for_index(self, BaseType index):

        # We want to use shared Completer instance because allocating
        # a Completer makes this function (and thus __getitem__) 2x slower.
        # This could be not thread-safe; GIL helps us, but we should be careful
        # not to occasionally switch to an another thread by iteracting
        # with Python interpreter in any way (switch happens
        # between bytecode instructions).

        cdef int key_len
        cdef b64_decode.decoder b64_decoder
        cdef char[MAX_VALUE_SIZE] b64_decoder_storage
        cdef vector[string] results

        self._completer.Start(index)

        while self._completer.Next():
            b64_decoder.init()
            key_len = b64_decoder.decode(
                self._completer.key(),
                self._completer.length(),
                b64_decoder_storage
            )
            results.push_back(string(<char*>b64_decoder_storage, key_len))

        return results

    cpdef list b_get_value(self, bytes key):
        cdef BaseType index
        if not self._follow_key(key, &index):
            return []
        return self._value_for_index(index)

    cpdef list items(self, unicode prefix=""):
        cdef bytes b_prefix = prefix.encode('utf8')
        cdef bytes value
        cdef int i
        cdef list res = []
        cdef char* raw_key
        cdef char* raw_value
        cdef int raw_value_len

        cdef BaseType index = self.dct.root()
        if not self.dct.Follow(b_prefix, &index):
            return res

        cdef int _len
        cdef b64_decode.decoder _b64_decoder
        cdef char[MAX_VALUE_SIZE] _b64_decoder_storage

        cdef Completer completer
        init_completer(completer, self.dct, self.guide)
        completer.Start(index, b_prefix)

        while completer.Next():
            raw_key = <char*>completer.key()

            for i in range(0, completer.length()):
                if raw_key[i] == self._c_payload_separator:
                    break

            raw_value = &(raw_key[i])
            raw_value_len = completer.length() - i

            _b64_decoder.init()
            _len = _b64_decoder.decode(raw_value, raw_value_len, _b64_decoder_storage)
            value = _b64_decoder_storage[:_len]

            u_key = raw_key[:i].decode('utf8')
            res.append(
                (u_key, value)
            )

        return res

    def iteritems(self, unicode prefix=""):
        cdef bytes b_prefix = prefix.encode('utf8')
        cdef bytes value
        cdef int i
        cdef char* raw_key
        cdef char* raw_value
        cdef int raw_value_len

        cdef BaseType index = self.dct.root()
        if not self.dct.Follow(b_prefix, &index):
            return

        cdef int _len
        cdef b64_decode.decoder _b64_decoder
        cdef char[MAX_VALUE_SIZE] _b64_decoder_storage

        cdef Completer completer
        init_completer(completer, self.dct, self.guide)
        completer.Start(index, b_prefix)

        while completer.Next():
            raw_key = <char*>completer.key()

            for i in range(0, completer.length()):
                if raw_key[i] == self._c_payload_separator:
                    break

            raw_value = &(raw_key[i])
            raw_value_len = completer.length() - i

            _b64_decoder.init()
            _len = _b64_decoder.decode(raw_value, raw_value_len, _b64_decoder_storage)
            value = _b64_decoder_storage[:_len]

            u_key = raw_key[:i].decode('utf8')
            yield (u_key, value)

    cpdef list keys(self, unicode prefix=""):
        cdef bytes b_prefix = prefix.encode('utf8')
        cdef int i
        cdef list res = []
        cdef char* raw_key

        cdef BaseType index = self.dct.root()
        if not self.dct.Follow(b_prefix, &index):
            return res

        cdef Completer completer
        init_completer(completer, self.dct, self.guide)
        completer.Start(index, b_prefix)

        while completer.Next():
            raw_key = <char*>completer.key()

            for i in range(0, completer.length()):
                if raw_key[i] == self._c_payload_separator:
                    break

            u_key = raw_key[:i].decode('utf8')
            res.append(u_key)
        return res

    def iterkeys(self, unicode prefix=""):
        cdef bytes b_prefix = prefix.encode('utf8')
        cdef int i
        cdef char* raw_key

        cdef BaseType index = self.dct.root()
        if not self.dct.Follow(b_prefix, &index):
            return

        cdef Completer completer
        init_completer(completer, self.dct, self.guide)
        completer.Start(index, b_prefix)

        while completer.Next():
            raw_key = <char*>completer.key()

            for i in range(0, completer.length()):
                if raw_key[i] == self._c_payload_separator:
                    break

            u_key = raw_key[:i].decode('utf8')
            yield u_key

    cdef bint _has_value(self, BaseType index):
        cdef BaseType _index = index
        return self.dct.Follow(self._c_payload_separator, &_index)

    cdef list _similar_items(self, unicode current_prefix, unicode key, BaseType cur_index, dict replace_chars):
        cdef BaseType next_index, index = cur_index
        cdef unicode prefix, u_replace_char, found_key
        cdef bytes b_step, b_replace_char
        cdef list res = []
        cdef list extra_items, value

        cdef int start_pos = len(current_prefix)
        cdef int end_pos = len(key)
        cdef int word_pos = start_pos

        while word_pos < end_pos:
            b_step = <bytes>(key[word_pos].encode('utf8'))

            if b_step in replace_chars:
                next_index = index
                b_replace_char, u_replace_char = <tuple>replace_chars[b_step]

                if self.dct.Follow(b_replace_char, &next_index):
                    prefix = current_prefix + key[start_pos:word_pos] + u_replace_char
                    extra_items = self._similar_items(prefix, key, next_index, replace_chars)
                    res.extend(extra_items)

            if not self.dct.Follow(b_step, &index):
                break
            word_pos += 1

        else:
            if self.dct.Follow(self._c_payload_separator, &index):
                found_key = current_prefix + key[start_pos:]
                value = self._value_for_index(index)
                res.insert(0, (found_key, value))

        return res

    cpdef list similar_items(self, unicode key, dict replaces):
        """
        Return a list of (key, value) tuples for all variants of ``key``
        in this DAWG according to ``replaces``.

        ``replaces`` is an object obtained from
        ``DAWG.compile_replaces(mapping)`` where mapping is a dict
        that maps single-char unicode sitrings to another single-char
        unicode strings.
        """
        return self._similar_items("", key, self.dct.root(), replaces)

    cdef list _similar_item_values(self, int start_pos, unicode key, BaseType cur_index, dict replace_chars):
        cdef BaseType next_index, index = cur_index
        cdef unicode prefix, u_replace_char, found_key
        cdef bytes b_step, b_replace_char
        cdef list res = []
        cdef list extra_items, value

        #cdef int start_pos = len(current_prefix)
        cdef int end_pos = len(key)
        cdef int word_pos = start_pos

        while word_pos < end_pos:
            b_step = <bytes>(key[word_pos].encode('utf8'))

            if b_step in replace_chars:
                next_index = index
                b_replace_char, u_replace_char = <tuple>replace_chars[b_step]

                if self.dct.Follow(b_replace_char, &next_index):
                    extra_items = self._similar_item_values(word_pos+1, key, next_index, replace_chars)
                    res.extend(extra_items)

            if not self.dct.Follow(b_step, &index):
                break
            word_pos += 1

        else:
            if self.dct.Follow(self._c_payload_separator, &index):
                value = self._value_for_index(index)
                res.insert(0, value)

        return res

    cpdef list similar_item_values(self, unicode key, dict replaces):
        """
        Return a list of values for all variants of the ``key``
        in this DAWG according to ``replaces``.

        ``replaces`` is an object obtained from
        ``DAWG.compile_replaces(mapping)`` where mapping is a dict
        that maps single-char unicode sitrings to another single-char
        unicode strings.
        """
        return self._similar_item_values(0, key, self.dct.root(), replaces)



cdef class RecordDAWG(BytesDAWG):
    """
    DAWG that is able to transparently store binary payload in keys;
    there may be several payloads for the same key.

    The payload format must be defined at creation time using ``fmt``
    constructor argument; it has the same meaning as ``fmt`` argument
    for functions from ``struct`` module; take a look at
    http://docs.python.org/library/struct.html#format-strings for the
    specification.

    In other words, this class implements read-only DAWG-based
    {unicode -> list of tuples} mapping where all tuples are of the
    same structure and may be packed with the same format string.
    """
    cdef _struct

    def __init__(self, fmt, arg=None, input_is_sorted=False, bytes payload_separator=PAYLOAD_SEPARATOR):
        """
        ``arg`` must be an iterable of tuples (unicode_key, data_tuple).
        data tuples will be converted to bytes with
        ``struct.pack(fmt, *data_tuple)``.

        Take a look at
        http://docs.python.org/library/struct.html#format-strings for the
        format string specification.
        """
        self._struct = struct.Struct(str(fmt))

        if arg is None:
            arg = []

        keys = ((d[0], self._struct.pack(*d[1])) for d in arg)
        super(RecordDAWG, self).__init__(keys, input_is_sorted, payload_separator)

    cdef list _value_for_index(self, BaseType index):
        cdef list value = BytesDAWG._value_for_index(self, index)
        return [self._struct.unpack(val) for val in value]

    cpdef list items(self, unicode prefix=""):
        cdef list items = BytesDAWG.items(self, prefix)
        return [(key, self._struct.unpack(val)) for (key, val) in items]

    def iteritems(self, unicode prefix=""):
        for key, val in BytesDAWG.iteritems(self, prefix):
            yield (key, self._struct.unpack(val))


def _iterable_from_argument(arg):
    if arg is None:
        arg = []

    if isinstance(arg, Mapping):
        return ((key, arg[key]) for key in arg)
    else:
        return arg

DEF LOOKUP_ERROR = -1

cdef class IntDAWG(DAWG):
    """
    Dict-like class based on DAWG.
    It can store integer values for unicode keys.
    """
    def __init__(self, arg=None, input_is_sorted=False):
        """
        ``arg`` must be an iterable of tuples (unicode_key, int_value)
        or a dict {unicode_key: int_value}.
        """
        iterable = _iterable_from_argument(arg)
        super(IntDAWG, self).__init__(iterable, input_is_sorted)

    def __getitem__(self, key):
        cdef int res = self.get(key, LOOKUP_ERROR)
        if res == LOOKUP_ERROR:
            raise KeyError(key)
        return res

    cpdef get(self, key, default=None):
        """
        Return value for the given key or ``default`` if the key is not found.
        """
        cdef int res

        if isinstance(key, unicode):
            res = self.get_value(<unicode>key)
        else:
            res = self.b_get_value(key)

        if res == LOOKUP_ERROR:
            return default
        return res

    cpdef int get_value(self, unicode key):
        cdef bytes b_key = <bytes>key.encode('utf8')
        return self.dct.Find(b_key)

    cpdef int b_get_value(self, bytes key):
        return self.dct.Find(key)


# FIXME: code duplication.
cdef class IntCompletionDAWG(CompletionDAWG):
    """
    Dict-like class based on DAWG.
    It can store integer values for unicode keys and support key completion.
    """

    def __init__(self, arg=None, input_is_sorted=False):
        """
        ``arg`` must be an iterable of tuples (unicode_key, int_value)
        or a dict {unicode_key: int_value}.
        """
        iterable = _iterable_from_argument(arg)
        super(IntCompletionDAWG, self).__init__(iterable, input_is_sorted)

    def __getitem__(self, key):
        cdef int res = self.get(key, LOOKUP_ERROR)
        if res == LOOKUP_ERROR:
            raise KeyError(key)
        return res

    cpdef get(self, key, default=None):
        """
        Return value for the given key or ``default`` if the key is not found.
        """
        cdef int res

        if isinstance(key, unicode):
            res = self.get_value(<unicode>key)
        else:
            res = self.b_get_value(key)

        if res == LOOKUP_ERROR:
            return default
        return res

    cpdef int get_value(self, unicode key):
        cdef bytes b_key = <bytes>key.encode('utf8')
        return self.dct.Find(b_key)

    cpdef int b_get_value(self, bytes key):
        return self.dct.Find(key)

    cpdef list items(self, unicode prefix=""):
        cdef bytes b_prefix = prefix.encode('utf8')
        cdef BaseType index = self.dct.root()
        cdef list res = []
        cdef int value

        if not self.dct.Follow(b_prefix, &index):
            return res

        cdef Completer completer
        init_completer(completer, self.dct, self.guide)
        completer.Start(index, b_prefix)

        while completer.Next():
            key = (<char*>completer.key()).decode('utf8')
            value = completer.value()
            res.append((key, value))

        return res

    def iteritems(self, unicode prefix=""):
        cdef bytes b_prefix = prefix.encode('utf8')
        cdef BaseType index = self.dct.root()
        cdef int value

        if not self.dct.Follow(b_prefix, &index):
            return

        cdef Completer completer
        init_completer(completer, self.dct, self.guide)
        completer.Start(index, b_prefix)

        while completer.Next():
            key = (<char*>completer.key()).decode('utf8')
            value = completer.value()
            yield key, value
