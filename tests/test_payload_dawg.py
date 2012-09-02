# -*- coding: utf-8 -*-
from __future__ import absolute_import, unicode_literals
import pickle
from io import BytesIO

import pytest
import dawg

DATA = (
    ('foo', b'data1'),
    ('bar', b'data2'),
    ('foo', b'data3'),
    ('foobar', b'data4')
)

STRUCTURED_DATA = (  # payload is (length, vowels count, index) tuple
    ('foo',     (3, 2, 0)),
    ('bar',     (3, 1, 0)),
    ('foo',     (3, 2, 1)),
    ('foobar',  (6, 3, 0))
)

def test_contains():
    d = dawg.BytesDAWG(DATA)
    for key, val in DATA:
        assert key in d

    assert 'food' not in d
    assert 'x' not in d
    assert 'fo' not in d


def test_getitem():
    d = dawg.BytesDAWG(DATA)

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


def test_record_getitem():
    d = dawg.RecordDAWG("=3H", STRUCTURED_DATA)
    assert d['foo'] == [(3, 2, 0), (3, 2, 1)]
    assert d['bar'] == [(3, 1, 0)]
    assert d['foobar'] == [(6, 3, 0)]

def test_record_items():
    d = dawg.RecordDAWG("=3H", STRUCTURED_DATA)
    assert sorted(d.items()) == sorted(STRUCTURED_DATA)

def test_record_keys():
    d = dawg.RecordDAWG("=3H", STRUCTURED_DATA)
    assert sorted(d.keys()) == ['bar', 'foo', 'foo', 'foobar',]

def test_record_keys_prefix():
    d = dawg.RecordDAWG("=3H", STRUCTURED_DATA)
    assert sorted(d.keys('fo')) == ['foo', 'foo', 'foobar']
    assert d.keys('bar') == ['bar']
    assert d.keys('barz') == []
