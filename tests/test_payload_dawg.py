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
    d = dawg.PayloadDAWG(DATA)
    for key, val in DATA:
        assert key in d

    assert 'food' not in d
    assert 'x' not in d
    assert 'fo' not in d


def test_getitem():
    d = dawg.PayloadDAWG(DATA)

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


def test_strutured_getitem():
    d = dawg.StructuredDAWG("=3H", STRUCTURED_DATA)
    assert d['foo'] == [(3, 2, 0), (3, 2, 1)]
    assert d['bar'] == [(3, 1, 0)]
    assert d['foobar'] == [(6, 3, 0)]
