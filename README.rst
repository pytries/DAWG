DAWG
====

.. image:: https://travis-ci.org/kmike/DAWG.png?branch=master
    :target: https://travis-ci.org/kmike/DAWG

This package provides DAWG(DAFSA_)-based dictionary-like
read-only objects for Python (2.x and 3.x).

String data in a DAWG may take 200x less memory than in
a standard Python dict and the raw lookup speed is comparable;
it also provides fast advanced methods like prefix search.

.. _DAFSA: https://en.wikipedia.org/wiki/Deterministic_acyclic_finite_state_automaton

* Docs: http://dawg.readthedocs.org
* Source code: https://github.com/kmike/DAWG
* Issue tracker: https://github.com/kmike/DAWG/issues

License
=======

Wrapper code is licensed under MIT License.
Bundled `dawgdic`_ C++ library is licensed under BSD license.
Bundled libb64_ is Public Domain.

.. _dawgdic: https://code.google.com/p/dawgdic/
.. _libb64: http://libb64.sourceforge.net/
