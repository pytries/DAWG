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


class TestDAWG(object):

    def test_sorted_iterable(self):

        sorted_data = ['bar', 'foo', 'foobar']
        contents = "\n".join(sorted_data).encode('utf8')
        with tempfile.NamedTemporaryFile() as f:
            f.write(contents)
            f.seek(0)

            words = (line.strip() for line in f)
            d = dawg.DAWG(words, input_is_sorted=True)

        assert 'bar' in d
        assert 'foo' in d

    def test_no_segfaults_on_invalid_file(self):
        d = dawg.DAWG()
        fd, path = tempfile.mkstemp()
        with open(path, 'w') as f:
            f.write('foo')

        with pytest.raises(IOError) as e:
            d.load(path)
            assert 'Invalid' in e.args[0]

        with open(path, 'rb') as f:
            with pytest.raises(IOError) as e:
                d.read(f)
                assert 'Invalid' in e.args[0]

    def test_no_segfaults_after_wrong_stream(self):
        d = dawg.DAWG()
        wrong_path = tempfile.mktemp()  # file doesn't exists

        with pytest.raises(IOError):
            d.load(wrong_path)

        assert 'random-key' not in d # there is possible segfault

    def test_build_errors(self):
        with pytest.raises(dawg.Error):
            data = [b'foo\x00bar', b'bar']
            dawg.DAWG(data)

    def test_contains_with_null_bytes(self):
        d = dawg.DAWG(['foo'])
        assert b'foo' in d
        assert b'foo\x00bar' not in d

    def test_unicode_sorting(self):
        key1 = '\U00010345\U0001033f\U00010337\U00010330\U0001033d'
        key2 = '\uff72\uff9c\uff90\uff7b\uff9e\uff9c'

        # This apparently depends on Python version:
        # assert key1 < key2
        # assert key1.encode('utf8') > key2.encode('utf8')

        # Constructor should sort data according to utf8 values,
        # not according to unicode sorting rules. It will raise an exception
        # if data is sorted according to unicode rules.
        dawg.DAWG([key1, key2])



class TestIntDAWG(object):

    IntDAWG = dawg.IntDAWG

    def dawg(self):
        payload = {'foo': 1, 'bar': 5, 'foobar': 3}
        d = self.IntDAWG(payload)
        return payload, d

    def test_getitem(self):
        payload, d = self.dawg()
        for key in payload:
            assert d[key] == payload[key]

        with pytest.raises(KeyError):
            d['fo']


    def test_dumps_loads(self):
        payload, d = self.dawg()
        data = d.tobytes()

        d2 = self.IntDAWG()
        d2.frombytes(data)
        for key, value in payload.items():
            assert key in d2
            assert d2[key] == value

    def test_dump_load(self):
        payload, _ = self.dawg()

        buf = BytesIO()
        self.IntDAWG(payload).write(buf)
        buf.seek(0)

        d = self.IntDAWG()
        d.read(buf)

        for key, value in payload.items():
            assert key in d
            assert d[key] == value

    def test_pickling(self):
        payload, d = self.dawg()

        data = pickle.dumps(d)
        d2 = pickle.loads(data)

        for key, value in payload.items():
            assert key in d2
            assert d[key] == value

    def test_int_value_ranges(self):
        for val in [0, 5, 2**16-1, 2**31-1]:
            d = self.IntDAWG({'f': val})
            assert d['f'] == val

        with pytest.raises(ValueError):
            self.IntDAWG({'f': -1})

        with pytest.raises(OverflowError):
            self.IntDAWG({'f': 2**32-1})


class TestIntCompletionDAWG(TestIntDAWG):
    IntDAWG = dawg.IntCompletionDAWG  # checks that all tests for IntDAWG pass


class TestCompletionDAWG(object):
    keys = ['f', 'bar', 'foo', 'foobar']

    def dawg(self):
        return dawg.CompletionDAWG(self.keys)

    def empty_dawg(self):
        return dawg.CompletionDAWG()

    def test_contains(self):
        d = self.dawg()
        for key in self.keys:
            assert key in d

    def test_keys(self):
        d = self.dawg()
        assert d.keys() == sorted(self.keys)

    def test_iterkeys(self):
        d = self.dawg()
        assert list(d.iterkeys()) == sorted(self.keys)
        assert list(d.iterkeys()) == d.keys()

    def test_prefixes(self):
        d = self.dawg()
        assert d.prefixes("foobarz") == ["f", "foo", "foobar"]
        assert d.prefixes("x") == []
        assert d.prefixes("bar") == ["bar"]

    def test_b_prefixes(self):
        d = self.dawg()
        assert d.b_prefixes(b"foobarz") == [b"f", b"foo", b"foobar"]
        assert d.b_prefixes(b"x") == []
        assert d.b_prefixes(b"bar") == [b"bar"]

    def test_iterprefixes(self):
        d = self.dawg()
        assert list(d.iterprefixes("foobarz")) == d.prefixes("foobarz")
        assert list(d.iterprefixes("x")) == d.prefixes("x")
        assert list(d.iterprefixes("bar")) == d.prefixes("bar")

    def test_completion(self):
        d = self.dawg()

        assert d.keys('z') == []
        assert d.keys('b') == ['bar']
        assert d.keys('foo') == ['foo', 'foobar']

    def test_has_keys_with_prefix(self):
        assert self.empty_dawg().has_keys_with_prefix('') == False

        d = self.dawg()
        assert d.has_keys_with_prefix('') == True
        assert d.has_keys_with_prefix('b') == True
        assert d.has_keys_with_prefix('fo') == True
        assert d.has_keys_with_prefix('bo') == False

    def test_completion_dawg_saveload(self):
        buf = BytesIO()
        self.dawg().write(buf)
        buf.seek(0)

        d = self.empty_dawg()
        d.read(buf)

        for key in self.keys:
            assert key in d

        assert d.keys('foo') == ['foo', 'foobar']
        assert d.keys('b') == ['bar']
        assert d.keys('z') == []

    def test_no_segfaults_on_invalid_file(self):
        d = self.dawg()
        fd, path = tempfile.mkstemp()
        with open(path, 'w') as f:
            f.write('foo')

        with pytest.raises(IOError) as e:
            d.load(path)
            assert "can't load _dawg.Dictionary" in e.args[0]

    def test_no_segfaults_on_empty_dawg(self):
        d = dawg.CompletionDAWG([])
        assert d.keys() == []


class TestIntCompletionDAWGComplete(TestCompletionDAWG):
    keys = ['f', 'bar', 'foo', 'foobar']

    def dawg(self):
        return dawg.IntCompletionDAWG((k, len(k)) for k in self.keys)

    def empty_dawg(self):
        return dawg.IntCompletionDAWG()

    def test_no_segfaults_on_empty_dawg(self):
        d = dawg.IntCompletionDAWG([])
        assert d.keys() == []

    def test_items(self):
        d = self.dawg()
        items = d.items()
        assert isinstance(items, list)
        for key, value in items:
            assert len(key) == value

    def test_iteritems(self):
        d = self.dawg()
        for key, value in d.iteritems():
            assert len(key) == value

    def test_items_prefix(self):
        d = self.dawg()
        assert d.items('fo') == [('foo', 3), ('foobar', 6)]
