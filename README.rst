DAWG
====

This package provides DAWG-based dictionary-like
read-only object for Python (2.x and 3.x).

Based on `dawgdic` C++ library.

.. _dawgdic: https://code.google.com/p/dawgdic/

Installation
============

TODO

Usage
=====

Create a new DAWG::

    >>> import dawg
    >>> d = dawg.IntDict({u'key1': value1, u'key2': value2, u'key3': value3})

TODO

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


Running tests and benchmarks
----------------------------

Make sure `tox`_ is installed and run

::

    $ tox

from the source checkout. Tests should pass under python 2.6, 2.7, 3.2 and 3.3.

.. note::

    At the moment of writing the latest pip release (1.1) does not
    support Python 3.3; in order to run tox tests under Python 3.3
    find the "virtualenv_support" directory in site-packages
    (of the env you run tox from) and place an sdist zip/tarball of the newer
    pip (from github) there.

.. _cython: http://cython.org
.. _tox: http://tox.testrun.org

Authors & Contributors
----------------------

* Mikhail Korobov <kmike84@gmail.com>

This module is based on `dawgdic`_ C++ library by
Susumu Yata & contributors.

License
=======

Wrapper code is licensed under MIT License.
Bundled `dawgdic`_ C++ library is licensed under BSD license.
