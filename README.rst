DAWG
====

This package provides DAWG-based dictionary-like
read-only objects for Python (2.x and 3.x).

String data in a DAWG (Directed Acyclic Word Graph) may take
200x less memory than in a standard Python dict or list and
the raw lookup speed is comparable. DAWG may be even faster than
built-in dict for some operations. It also provides fast
advanced methods like prefix search.

Based on `dawgdic`_ C++ library.

.. _dawgdic: https://code.google.com/p/dawgdic/

Installation
============

pip install DAWG

Usage
=====

There are several DAWG classes in this package:

* ``dawg.DAWG`` - basic DAWG wrapper; it can store unicode keys
  and do exact lookups;

* ``dawg.CompletionDAWG`` - ``dawg.DAWG`` subclass that supports
  key completion and prefix lookups (but requires more memory);

* ``dawg.BytesDAWG`` - ``dawg.CompletionDAWG`` subclass that
  maps unicode keys to lists of ``bytes`` objects.

* ``dawg.RecordDAWG`` - ``dawg.BytesDAWG`` subclass that
  maps unicode keys to lists of data tuples.
  All tuples must be of the same format (the data is packed
  using python ``struct`` module).

* ``dawg.IntDAWG`` - ``dawg.DAWG`` subclass that maps unicode keys
  to integer values.

DAWG and CompletionDAWG
-----------------------

``DAWG`` and ``CompletionDAWG`` are useful when you need
fast & memory efficient simple string storage. These classes
does not support assigning values to keys.

``DAWG`` and ``CompletionDAWG`` constructors accept an iterable with keys::

    >>> import dawg
    >>> words = [u'foo', u'bar', u'foobar', u'foö', u'bör']
    >>> base_dawg = dawg.DAWG(words)
    >>> completion_dawg = dawg.CompletionDAWG(words)

It is then possible to check if the key is in a DAWG::

    >>> u'foo' in base_dawg
    True
    >>> u'baz' in completion_dawg
    False

It is possible to find all keys that starts with a given
prefix in a ``CompletionDAWG``::

    >>> completion_dawg.keys(u'foo')
    >>> [u'foo', u'foobar']

and to find all prefixes of a given key::

    >>> base_dawg.prefixes(u'foobarz')
    [u'foo', u'foobar']

Iterator versions are also available::

    >>> for key in completion_dawg.iterkeys(u'foo'):
    ...     print(key)
    foo
    foobar
    >>> for prefix in base_dawg.iterprefixes(u'foobarz'):
    ...     print(prefix)
    foo
    foobar

It is possible to find all keys similar to a given key (using a one-way
char translation table)::

    >>> replaces = dawg.DAWG.compile_replaces({u'o': u'ö'})
    >>> base_dawg.similar_keys(u'foo', replaces)
    [u'foo', u'foö']
    >>> base_dawg.similar_keys(u'foö', replaces)
    [u'foö']
    >>> base_dawg.similar_keys(u'bor', replaces)
    [u'bör']

BytesDAWG
---------

``BytesDAWG`` is a ``CompletionDAWG`` subclass that can store
binary data for each key.

``BytesDAWG`` constructor accepts an iterable with
``(unicode_key, bytes_value)`` tuples::

    >>> data = [(u'key1', b'value1'), (u'key2', b'value2'), (u'key1', b'value3')]
    >>> bytes_dawg = dawg.BytesDAWG(data)

There can be duplicate keys; all unique values are stored in this case::

    >>> bytes_dawg[u'key1']
    [b'value1, b'value3']

For unique keys a list with a single value is returned for consistency::

    >>> bytes_dawg[u'key2']
    [b'value2']

``KeyError`` is raised for missing keys; use ``get`` method if you need
a default value instead::

    >>> bytes_dawg.get(u'foo', None)
    None

``BytesDAWG`` support ``keys``, ``items``, ``iterkeys`` and ``iteritems``
methods (they all accept optional key prefix). There is also support for
``similar_keys``, ``similar_items`` and ``similar_item_values`` methods.

.. note::

    Currently the order of keys returned by ``BytesDAWG`` is not the same
    as the order of keys returned by ``CompletionDAWG`` because
    of the way ``BytesDAWG`` is implemented: values are internally stored inside
    DAWG keys after a separator; separator is a chr(255) byte and thus
    ``'foo'`` key is greater than ``'foobar'`` key (values compared
    are ``'foo<sep>'`` and ``'foobar<sep>'``).

RecordDAWG
----------

``RecordDAWG`` is a ``BytesDAWG`` subclass that automatically
packs & unpacks the binary data from/to Python objects
using ``struct`` module from the standard library.

First, you have to define a format of the data. Consult Python docs
(http://docs.python.org/library/struct.html#format-strings) for the format
string specification.

For example, let's store 3 short unsigned numbers (in a Big-Endian byte order)
as values::

    >>> format = ">HHH"

``RecordDAWG`` constructor accepts an iterable with
``(unicode_key, value_tuple)``. Let's create such iterable
using ``zip`` function::

    >>> keys = [u'foo', u'bar', u'foobar', u'foo']
    >>> values = [(1, 2, 3), (2, 1, 0), (3, 3, 3), (2, 1, 5)]
    >>> data = zip(keys, values)
    >>> record_dawg = RecordDAWG(format, data)

As with ``BytesDAWG``, there can be several values for the same key::

    >>> record_dawg['foo']
    [(1, 2, 3), (2, 1, 5)]
    >>> record_dawg['foobar']
    [(3, 3, 3)]


IntDAWG
-------

``IntDAWG`` is a ``{unicode -> int}`` mapping. It is possible to
use ``RecordDAWG`` for this, but ``IntDAWG`` is natively
supported by dawgdic_ C++ library and so ``__getitem__`` is much faster.

Unlike ``BytesDAWG`` and ``RecordDAWG``, ``IntDAWG`` doesn't support
having several values for the same key.

``IntDAWG`` constructor accepts an iterable with (unicode_key, integer_value)
tuples::

    >>> data = [ (u'foo', 1), (u'bar', 2) ]
    >>> int_dawg = dawg.IntDAWG(data)

It is then possible to get a value from the IntDAWG::

    >>> int_dawg[u'foo']
    1

Persistence
-----------

All DAWGs support saving/loading and pickling/unpickling.

Write DAWG to a stream::

    >>> with open('words.dawg', 'wb') as f:
    ...     d.write(f)

Save DAWG to a file::

    >>> d.save('words.dawg')

Load DAWG from a file::

    >>> d = dawg.DAWG()
    >>> d.load('words.dawg')

.. warning::

    Reading DAWGs from streams and unpickling are currently using 3x memory
    compared to loading DAWGs using ``load`` method; please avoid them until
    the issue is fixed.

Read DAWG from a stream::

    >>> d = dawg.RecordDAWG(format_string)
    >>> with open('words.record-dawg', 'rb') as f:
    ...     d.read(f)

DAWG objects are picklable::

    >>> import pickle
    >>> data = pickle.dumps(d)
    >>> d2 = pickle.loads(data)

Benchmarks
==========

For a list of 3000000 (3 million) Russian words memory consumption
with different data structures (under Python 2.7):

* dict(unicode words -> word lenghts): about 600M
* list(unicode words) : about 300M
* ``marisa_trie.RecordTrie`` : 11M
* ``marisa_trie.Trie``: 7M
* ``dawg.DAWG``: 2M
* ``dawg.CompletionDAWG``: 3M
* ``dawg.IntDAWG``: 2.7M
* ``dawg.RecordDAWG``: 4M


.. note::

    Lengths of words were not stored as values in ``dawg.DAWG``,
    ``dawg.CompletionDAWG`` and ``marisa_trie.Trie`` because they don't
    support this.

Benchmark results (100k unicode words, integer values (lenghts of the words),
Python 3.2, macbook air i5 1.8 Ghz)::

    dict __getitem__ (hits):        4.102M ops/sec
    DAWG __getitem__ (hits):        not supported
    BytesDAWG __getitem__ (hits):   1.558M ops/sec
    RecordDAWG __getitem__ (hits):  0.950M ops/sec
    IntDAWG __getitem__ (hits):     2.835M ops/sec
    dict get() (hits):              3.053M ops/sec
    DAWG get() (hits):              not supported
    BytesDAWG get() (hits):         1.340M ops/sec
    RecordDAWG get() (hits):        0.882M ops/sec
    IntDAWG get() (hits):           2.370M ops/sec
    dict get() (misses):            3.250M ops/sec
    DAWG get() (misses):            not supported
    BytesDAWG get() (misses):       2.483M ops/sec
    RecordDAWG get() (misses):      2.249M ops/sec
    IntDAWG get() (misses):         2.806M ops/sec

    dict __contains__ (hits):           4.068M ops/sec
    DAWG __contains__ (hits):           3.065M ops/sec
    BytesDAWG __contains__ (hits):      2.627M ops/sec
    RecordDAWG __contains__ (hits):     2.613M ops/sec
    IntDAWG __contains__ (hits):        3.021M ops/sec

    dict __contains__ (misses):         3.471M ops/sec
    DAWG __contains__ (misses):         3.537M ops/sec
    BytesDAWG __contains__ (misses):    3.381M ops/sec
    RecordDAWG __contains__ (misses):   3.361M ops/sec
    IntDAWG __contains__ (misses):      3.540M ops/sec

    dict items():       58.754 ops/sec
    DAWG items():       not supported
    BytesDAWG items():  15.914 ops/sec
    RecordDAWG items(): 10.699 ops/sec
    IntDAWG items():    not supported

    dict keys():        214.499 ops/sec
    DAWG keys():        not supported
    BytesDAWG keys():   23.929 ops/sec
    RecordDAWG keys():  23.726 ops/sec
    IntDAWG keys():     not supported

    DAWG.prefixes (hits):    0.244M ops/sec
    DAWG.prefixes (mixed):   1.414M ops/sec
    DAWG.prefixes (misses):  2.156M ops/sec

    RecordDAWG.keys(prefix="xxx"), avg_len(res)==415:       6.057K ops/sec
    RecordDAWG.keys(prefix="xxxxx"), avg_len(res)==17:      130.680K ops/sec
    RecordDAWG.keys(prefix="xxxxxxxx"), avg_len(res)==3:    507.355K ops/sec
    RecordDAWG.keys(prefix="xxxxx..xx"), avg_len(res)==1.4: 745.566K ops/sec
    RecordDAWG.keys(prefix="xxx"), NON_EXISTING:            3032.758K ops/sec


Please take this benchmark results with a grain of salt; this
is a very simple benchmark on a single data set.


Current limitations
===================

* ``IntDAWG`` is currently a subclass of ``DAWG`` and so it doesn't
  support ``keys()`` and ``items()`` methods;
* ``read()`` method reads the whole stream (DAWG must be the last or the
  only item in a stream if it is read with ``read()`` method) - pickling
  doesn't have this limitation;
* DAWGs loaded with ``read()`` and unpickled DAWGs uses 3x-4x memory
  compared to DAWGs loaded with ``load()`` method;
* there are ``keys()`` and ``items()`` methods but no ``values()`` method;
* iterator versions of methods are not always implemented;
* ``BytesDAWG`` and ``RecordDAWG`` key order is different from
  ``CompletionDAWG`` key order;
* ``BytesDAWG`` and ``RecordDAWG`` has a limitation: values
  larger than 8KB are unsupported.

Contributions are welcome!


Contributing
============

Development happens at github and bitbucket:

* https://github.com/kmike/DAWG
* https://bitbucket.org/kmike/DAWG

The main issue tracker is at github: https://github.com/kmike/DAWG/issues

Feel free to submit ideas, bugs, pull requests (git or hg) or
regular patches.

If you found a bug in a C++ part please report it to the original
`bug tracker <https://code.google.com/p/dawgdic/issues/list>`_.

How is source code organized
----------------------------

There are 4 folders in repository:

* ``bench`` - benchmarks & benchmark data;
* ``lib`` - original unmodified `dawgdic`_ C++ library and
  a customized version of `libb64`_ library. They are bundled
  for easier distribution; if something is have to be fixed in these
  libraries consider fixing it in the original repositories;
* ``src`` - wrapper code; ``src/dawg.pyx`` is a wrapper implementation;
  ``src/*.pxd`` files are Cython headers for corresponding C++ headers;
  ``src/*.cpp`` files are the pre-built extension code and shouldn't be
  modified directly (they should be updated via ``update_cpp.sh`` script).
* ``tests`` - the test suite.


Running tests and benchmarks
----------------------------

Make sure `tox`_ is installed and run

::

    $ tox

from the source checkout. Tests should pass under python 2.6, 2.7, 3.2 and 3.3.

In order to run benchmarks, type

::

    $ tox -c bench.ini

.. _cython: http://cython.org
.. _tox: http://tox.testrun.org

Authors & Contributors
----------------------

* Mikhail Korobov <kmike84@gmail.com>;
* Dan Blanchard.

This module is based on `dawgdic`_ C++ library by
Susumu Yata & contributors.

base64 decoder is based on libb64_ by Chris Venter.

.. _libb64: http://libb64.sourceforge.net/

License
=======

Wrapper code is licensed under MIT License.
Bundled `dawgdic`_ C++ library is licensed under BSD license.
libb64_ is Public Domain.
