# -*- coding: utf-8 -*-
from __future__ import absolute_import, unicode_literals
import pickle
from io import BytesIO

import pytest
import dawg

def test_contains():
    d = dawg.IntDict({'foo': 1, 'bar': 2, 'foobar': 3})

    assert 'foo' in d
    assert 'bar' in d
    assert 'foobar' in d
    assert 'fo' not in d
    assert 'x' not in d

def test_getitem():
    d = dawg.IntDict({'foo': 1, 'bar': 5, 'foobar': 3})
    assert d['foo'] == 1
    assert d['bar'] == 5
    assert d['foobar'] == 3

    with pytest.raises(KeyError):
        d['fo']


def test_dumps_loads():
    payload = {'foo': 1, 'bar': 5, 'foobar': 3}
    d = dawg.IntDict(payload)
    data = d.dumps()

    d2 = dawg.IntDict()
    d2.loads(data)
    for key, value in payload.items():
        assert key in d2
        assert d2[key] == value

def test_dump_load():
    payload = {'foo': 1, 'bar': 5, 'foobar': 3}

    buf = BytesIO()
    dawg.IntDict(payload).dump(buf)
    buf.seek(0)

    d = dawg.IntDict()
    d.load(buf)

    for key, value in payload.items():
        assert key in d
        assert d[key] == value

def test_pickling():
    payload = {'foo': 1, 'bar': 5, 'foobar': 3}
    d = dawg.IntDict(payload)

    data = pickle.dumps(d)
    d2 = pickle.loads(data)

    for key, value in payload.items():
        assert key in d2
        assert d[key] == value
