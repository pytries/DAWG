# -*- coding: utf-8 -*-
from __future__ import absolute_import, unicode_literals

import pytest
import dawg

class TestBytesDAWG(object):

    DATA = (
        ('foo', b'data3'),
        ('bar', b'data2'),
        ('foo', b'data1'),
        ('foobar', b'data4')
    )

    DATA_KEYS = list(zip(*DATA))[0]

    def dawg(self, **kwargs):
        return dawg.BytesDAWG(self.DATA, **kwargs)

    def test_contains(self):
        d = self.dawg()
        for key, val in self.DATA:
            assert key in d

        assert 'food' not in d
        assert 'x' not in d
        assert 'fo' not in d


    def test_getitem(self):
        d = self.dawg()

        assert d['foo'] == [b'data1', b'data3']
        assert d['bar'] == [b'data2']
        assert d['foobar'] == [b'data4']

        with pytest.raises(KeyError):
            d['f']

        with pytest.raises(KeyError):
            d['food']

        with pytest.raises(KeyError):
            d['foobarz']

        with pytest.raises(KeyError):
            d['x']

    def test_prefixes(self):
        d = self.dawg()
        assert d.prefixes("foobarz") == ["foo", "foobar"]
        assert d.prefixes("x") == []
        assert d.prefixes("bar") == ["bar"]

    def test_keys(self):
        d = self.dawg()
        assert d.keys() == sorted(self.DATA_KEYS)

    def test_keys_ordering(self):
        data = [('foo', b'v1'), ('foobar', b'v2'), ('bar', b'v3')]

        d = dawg.BytesDAWG(data, payload_separator=b'\xff')
        assert d.keys() == ['bar', 'foobar', 'foo']

        d2 = dawg.BytesDAWG(data, payload_separator=b'\x01')
        assert d2.keys() == ['bar', 'foo', 'foobar']

    def test_iterkeys(self):
        d = self.dawg()
        assert list(d.iterkeys()) == d.keys()
        assert list(d.iterkeys()) == sorted(self.DATA_KEYS)

    def test_items(self):
        d = self.dawg()
        assert d.items() == sorted(self.DATA)

    def test_iteritems(self):
        d = self.dawg()
        assert list(d.iteritems()) == d.items()

    def test_build_error(self):
        with pytest.raises(dawg.Error):
            self.dawg(payload_separator=b'f')



class TestRecordDAWG(object):

    STRUCTURED_DATA = (
        ('foo',     (3, 2, 256)),
        ('bar',     (3, 1, 0)),
        ('foo',     (3, 2, 1)),
        ('foobar',  (6, 3, 0))
    )

    def dawg(self):
        return dawg.RecordDAWG(">3H", self.STRUCTURED_DATA)

    def test_record_getitem(self):
        d = self.dawg()
        assert d['foo'] == [(3, 2, 1), (3, 2, 256)]
        assert d['bar'] == [(3, 1, 0)]
        assert d['foobar'] == [(6, 3, 0)]

    def test_record_items(self):
        d = self.dawg()
        assert d.items() == sorted(self.STRUCTURED_DATA)

    def test_record_keys(self):
        d = self.dawg()
        assert d.keys() == ['bar', 'foo', 'foo', 'foobar',]

    def test_record_iterkeys(self):
        d = self.dawg()
        assert list(d.iterkeys()) == d.keys()

    def test_record_iteritems(self):
        d = self.dawg()
        assert list(d.iteritems()) == d.items()

    def test_record_keys_prefix(self):
        d = self.dawg()
        assert d.keys('fo') == ['foo', 'foo', 'foobar']
        assert d.keys('bar') == ['bar']
        assert d.keys('barz') == []

    def test_prefixes(self):
        d = self.dawg()
        assert d.prefixes("foobarz") == ["foo", "foobar"]
        assert d.prefixes("x") == []
        assert d.prefixes("bar") == ["bar"]
