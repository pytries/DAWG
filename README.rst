DAWG
====

This package provides DAWG-based dictionary-like
read-only object for Python (2.x and 3.x).

String data in a DAWG may take up to 50x-100x less
memory than in a standard Python dict; the raw lookup
speed is comparable; DAWG also provides fast
advanced methods like prefix search.

Based on `dawgdic` C++ library.

.. _dawgdic: https://code.google.com/p/dawgdic/

Installation
============

pip install DAWG

Usage
=====

There are several DAWG classes in this package:

* ``dawg.DAWG`` - basic DAWG wrapper; it can store unicode keys
  and do exact and prefix lookups;

* ``dawg.CompletionDAWG`` - ``dawg.DAWG`` subclass that supports
  key completion (but requires more memory);

* ``dawg.BytesDAWG`` - ``dawg.CompletionDAWG`` subclass that
  maps unicode keys to lists of ``bytes`` objects.

* ``dawg.RecordDAWG`` - ``dawg.BytesDAWG`` subclass that
  maps unicode keys to lists of data tuples.
  All tuples must be of the same format (the data is packed
  using python ``struct`` module).

* ``dawg.IntDAWG`` - ``dawg.DAWG`` subclass that maps unicode keys
  to integer values.

TODO: detailed usage? 

Benchmarks
==========

For a list of 3000000 (3 million) Russian words memory consumption
with different data structures (under Python 2.7):

* dict(unicode words -> word lenghts): about 600M
* list(unicode words) : about 300M
* ``marisa_trie.RecordTrie`` : 11M
* ``marisa_trie.Trie``: 7M
* ``dawg.DAWG``: 8M
* ``dawg.CompletionDAWG``: 10M
* ``dawg.IntDAWG``: 11M
* ``dawg.RecordDAWG``: 16M


.. note::

    Lengths of words were not stored as values in ``dawg.DAWG``,
    ``dawg.CompletionDAWG`` and ``marisa_trie.Trie`` because they don't
    support this.

Benchmark results (100k unicode words, integer values (lenghts of the words),
Python 3.2, macbook air i5 1.8 Ghz)::

    dict __getitem__ (hits):        4.102M ops/sec
    DAWG __getitem__ (hits):        not supported
    BytesDAWG __getitem__ (hits):   0.543M ops/sec
    RecordDAWG __getitem__ (hits):  0.440M ops/sec
    IntDAWG __getitem__ (hits):     2.835M ops/sec
    dict get() (hits):              3.053M ops/sec
    DAWG get() (hits):              not supported
    BytesDAWG get() (hits):         0.523M ops/sec
    RecordDAWG get() (hits):        0.426M ops/sec
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

    RecordDAWG.keys(prefix="xxx"), avg_len(res)==415:       5.943K ops/sec
    RecordDAWG.keys(prefix="xxxxx"), avg_len(res)==17:      113.025K ops/sec
    RecordDAWG.keys(prefix="xxxxxxxx"), avg_len(res)==3:    323.485K ops/sec
    RecordDAWG.keys(prefix="xxxxx..xx"), avg_len(res)==1.4: 431.099K ops/sec
    RecordDAWG.keys(prefix="xxx"), NON_EXISTING:            3032.758K ops/sec


Please take this benchmark results with a grain of salt; this
is a very simple benchmark on a single data set.


Current limitations
===================

* The library is not tested under Windows;
* ``read()`` method reads the whole stream (DAWG must be the last or the
  only item in a stream if it is read with ``read()`` method) - pickling
  doesn't have this limitation;
* iterator versions of methods are not always implemented;
* there are ``keys()`` and ``items()`` methods but no ``values()`` method.

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

from the source checkout. Tests should pass under python 2.6, 2.7 and 3.2.

In order to run benchmarks, type

::

    $ tox -c bench.ini

.. _cython: http://cython.org
.. _tox: http://tox.testrun.org

Authors & Contributors
----------------------

* Mikhail Korobov <kmike84@gmail.com>

This module is based on `dawgdic`_ C++ library by
Susumu Yata & contributors.

base64 decoder is based on libb64_ by Chris Venter.

.. _libb64: http://libb64.sourceforge.net/

License
=======

Wrapper code is licensed under MIT License.
Bundled `dawgdic`_ C++ library is licensed under BSD license.
libb64_ is Public Domain.