
0.3 (2012-09-13)
----------------

- ``similar_keys``, ``similar_items`` and ``similar_item_values`` methods
  for more permissive lookups (they may be useful e.g. for umlaut handling);
- ``load`` method returns self;
- Python 3.3 support.

0.2 (2012-09-08)
----------------

Greatly improved memory usage for DAWGs loaded with ``load`` method.

There is currently a bug somewhere in a wrapper so DAWGs loaded with
``read()`` method or unpickled DAWGs uses 3x-4x memory compared to DAWGs
loaded with ``load()`` method. ``load()`` is fixed in this release but
other methods are not.

0.1 (2012-09-08)
----------------

Initial release.