# -*- coding: utf-8 -*-
from __future__ import absolute_import, unicode_literals
import pickle
import tempfile
from io import BytesIO

import pytest
import dawg

def test_contains():
    d = dawg.IntDAWG({'foo': 1, 'bar': 2, 'foobar': 3})

    assert 'foo' in d
    assert 'bar' in d
    assert 'foobar' in d
    assert 'fo' not in d
    assert 'x' not in d

    assert b'foo' in d
    assert b'x' not in d

def test_sorted_iterable():

    sorted_data = ['bar', 'foo', 'foobar']
    contents = "\n".join(sorted_data).encode('utf8')
    with tempfile.NamedTemporaryFile() as f:
        f.write(contents)
        f.seek(0)

        words = (line.strip() for line in f)
        d = dawg.DAWG(words, input_is_sorted=True)

    assert 'bar' in d
    assert 'foo' in d


def test_getitem():
    d = dawg.IntDAWG({'foo': 1, 'bar': 5, 'foobar': 3})
    assert d['foo'] == 1
    assert d['bar'] == 5
    assert d['foobar'] == 3

    with pytest.raises(KeyError):
        d['fo']


def test_dumps_loads():
    payload = {'foo': 1, 'bar': 5, 'foobar': 3}
    d = dawg.IntDAWG(payload)
    data = d.tobytes()

    d2 = dawg.IntDAWG()
    d2.frombytes(data)
    for key, value in payload.items():
        assert key in d2
        assert d2[key] == value

def test_dump_load():
    payload = {'foo': 1, 'bar': 5, 'foobar': 3}

    buf = BytesIO()
    dawg.IntDAWG(payload).write(buf)
    buf.seek(0)

    d = dawg.IntDAWG()
    d.read(buf)

    for key, value in payload.items():
        assert key in d
        assert d[key] == value

def test_pickling():
    payload = {'foo': 1, 'bar': 5, 'foobar': 3}
    d = dawg.IntDAWG(payload)

    data = pickle.dumps(d)
    d2 = pickle.loads(data)

    for key, value in payload.items():
        assert key in d2
        assert d[key] == value

def test_completion():
    keys = ['f', 'bar', 'foo', 'foobar']
    d = dawg.CompletionDAWG(keys)
    for key in keys:
        assert key in d

    assert d.keys('foo') == ['foo', 'foobar']
    assert d.keys('b') == ['bar']
    assert d.keys('z') == []

def test_int_value_ranges():
    for val in [0, 5, 2**16-1, 2**31-1]:
        d = dawg.IntDAWG({'f': val})
        assert d['f'] == val

    with pytest.raises(ValueError):
        dawg.IntDAWG({'f': -1})

    with pytest.raises(OverflowError):
        dawg.IntDAWG({'f': 2**32-1})


#def test_int_keys():
#    payload = {'foo': 1, 'bar': 5, 'foobar': 3}
#    d = dawg.IntDAWG(payload)
#    assert d.keys() == payload.keys()

def test_completion_dawg_saveload():
    keys = ['f', 'bar', 'foo', 'foobar']

    buf = BytesIO()
    dawg.CompletionDAWG(keys).write(buf)
    buf.seek(0)

    d = dawg.CompletionDAWG()
    d.read(buf)

    for key in keys:
        assert key in d

    assert d.keys('foo') == ['foo', 'foobar']
    assert d.keys('b') == ['bar']
    assert d.keys('z') == []
