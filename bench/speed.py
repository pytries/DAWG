#!/usr/bin/env python
# -*- coding: utf-8 -*-
from __future__ import absolute_import, unicode_literals, division
import random
import string
import timeit
import os
import zipfile
import struct
#import pstats
#import cProfile

import dawg

def words100k():
    zip_name = os.path.join(
        os.path.abspath(os.path.dirname(__file__)),
        'words100k.txt.zip'
    )
    zf = zipfile.ZipFile(zip_name)
    txt = zf.open(zf.namelist()[0]).read().decode('utf8')
    return txt.splitlines()

def random_words(num):
    russian = 'абвгдеёжзиклмнопрстуфхцчъыьэюя'
    alphabet = '%s%s' % (russian, string.ascii_letters)
    return [
        "".join([random.choice(alphabet) for x in range(random.randint(1,15))])
        for y in range(num)
    ]

def truncated_words(words):
    return [word[:3] for word in words]

def prefixes1k(words, prefix_len):
    words = [w for w in words if len(w) >= prefix_len]
    every_nth = int(len(words)/1000)
    _words = [w[:prefix_len] for w in words[::every_nth]]
    return _words[:1000]

def leet_words(words, replaces):
    for key, value in replaces.items():
        words = [w.replace(key, value) for w in words]
    return words


WORDS100k = words100k()
MIXED_WORDS100k = truncated_words(WORDS100k)
NON_WORDS100k = random_words(100000)
PREFIXES_3_1k = prefixes1k(WORDS100k, 3)
PREFIXES_5_1k = prefixes1k(WORDS100k, 5)
PREFIXES_8_1k = prefixes1k(WORDS100k, 8)
PREFIXES_15_1k = prefixes1k(WORDS100k, 15)

LEET_REPLACES = {
    'o': '0',
    'O': '0',
    'u': '0',
    'l': '1',
    'i': '1',
    'e': '3',
    'E': '3',
    'A': '4',
    'a': '4',
    'h': '4',
    's': 'z',
}
LEET_50k = leet_words(WORDS100k[:50000], LEET_REPLACES)

def format_result(key, value, text_width):
    key = key.ljust(text_width)
    print("    %s %s" % (key, value))


def bench(name, timer, descr='M ops/sec', op_count=0.1, repeats=3, runs=5,
          text_width=33):
    try:
        times = []
        for x in range(runs):
            times.append(timer.timeit(repeats))

        def op_time(time):
            return op_count*repeats / time

        val = "%0.3f%s" % (op_time(min(times)), descr)
        format_result(name, val, text_width)
    except (AttributeError, TypeError) as e:
        format_result(name, "not supported", text_width)

def create_dawg():
    words = words100k()
    return dawg.DAWG(words)

def create_bytes_dawg():
    words = words100k()
    values = [struct.pack(str('<H'), len(word)) for word in words]
    return dawg.BytesDAWG(zip(words, values))

def create_record_dawg():
    words = words100k()
    values = [ [len(word)] for word in words]
    return dawg.RecordDAWG(str('<H'), zip(words, values))

def create_int_dawg():
    words = words100k()
    values = [len(word) for word in words]
    return dawg.IntDAWG(zip(words, values))

def create_leet_dawg():
    return dawg.DAWG(LEET_50k)


def benchmark():
    print('\n====== Benchmarks (100k unique unicode words) =======\n')

    tests = [
        ('__getitem__ (hits)', "for word in WORDS100k: data[word]", 'M ops/sec', 0.1, 3),
        ('get() (hits)', "for word in WORDS100k: data.get(word)", 'M ops/sec', 0.1, 3),
        ('get() (misses)', "for word in NON_WORDS_10k: data.get(word)", 'M ops/sec', 0.01, 5),
        ('__contains__ (hits)', "for word in WORDS100k: word in data", 'M ops/sec', 0.1, 3),
        ('__contains__ (misses)', "for word in NON_WORDS100k: word in data", 'M ops/sec', 0.1, 3),
        ('items()', 'list(data.items())', ' ops/sec', 1, 1),
        ('keys()', 'list(data.keys())', ' ops/sec', 1, 1),
#        ('values()', 'list(data.values())', ' ops/sec', 1, 1),
    ]

    common_setup = """
from __main__ import create_dawg, create_bytes_dawg, create_record_dawg, create_int_dawg, create_leet_dawg
from __main__ import WORDS100k, NON_WORDS100k, MIXED_WORDS100k
from __main__ import PREFIXES_3_1k, PREFIXES_5_1k, PREFIXES_8_1k, PREFIXES_15_1k
from __main__ import LEET_50k, LEET_REPLACES
NON_WORDS_10k = NON_WORDS100k[:10000]
NON_WORDS_1k = ['ыва', 'xyz', 'соы', 'Axx', 'avы']*200
"""
    dict_setup = common_setup + 'data = dict((word, len(word)) for word in WORDS100k);'
    dawg_setup = common_setup + 'data = create_dawg(); repl = data.compile_replaces(LEET_REPLACES);'
    bytes_dawg_setup = common_setup + 'data = create_bytes_dawg();'
    record_dawg_setup = common_setup + 'data = create_record_dawg();'
    int_dawg_setup = common_setup + 'data = create_int_dawg();'
    leet_dawg_setup = common_setup + 'data = create_leet_dawg(); repl = data.compile_replaces(LEET_REPLACES);'

    structures = [
        ('dict', dict_setup),
        ('DAWG', dawg_setup),
        ('BytesDAWG', bytes_dawg_setup),
        ('RecordDAWG', record_dawg_setup),
        ('IntDAWG', int_dawg_setup),
    ]
    for test_name, test, descr, op_count, repeats in tests:
        for name, setup in structures:
            timer = timeit.Timer(test, setup)
            full_test_name = "%s %s" % (name, test_name)
            bench(full_test_name, timer, descr, op_count, repeats, 9)

    # DAWG-specific benchmarks

    # benchmark for similar_keys
    bench(
        "DAWG.similar_keys  (no replaces)",
        timeit.Timer(
            "for word in WORDS100k[:50000]: data.similar_keys(word, repl)",
            setup=dawg_setup,
        ),
        op_count=0.05
    )
    bench(
        "DAWG.similar_keys  (l33t)",
        timeit.Timer(
            "for word in WORDS100k[:50000]: data.similar_keys(word, repl)",
            setup=leet_dawg_setup,
        ),
        op_count=0.05
    )

    for struct_name, setup in structures[1:]:

        # prefixes of a given key
        _bench_data = [
            ('hits', 'WORDS100k'),
            ('mixed', 'MIXED_WORDS100k'),
            ('misses', 'NON_WORDS100k'),
        ]

        for meth in ['prefixes']:
            for name, data in _bench_data:
                bench(
                    '%s.%s (%s)' % (struct_name, meth, name),
                    timeit.Timer(
                        "for word in %s:\n"
                        "   data.%s(word)" % (data, meth),
                        setup
                    ),
                    runs=3,
                )

        for meth in ['iterprefixes']:
            for name, data in _bench_data:
                bench(
                    '%s.%s (%s)' % (struct_name, meth, name),
                    timeit.Timer(
                        "for word in %s:\n"
                        "   list(data.%s(word))" % (data, meth),
                        setup
                    ),
                    runs=3,
                )

        # keys with a given prefix
        _bench_data = [
            ('xxx', 'avg_len(res)==415', 'PREFIXES_3_1k'),
            ('xxxxx', 'avg_len(res)==17', 'PREFIXES_5_1k'),
            ('xxxxxxxx', 'avg_len(res)==3', 'PREFIXES_8_1k'),
            ('xxxxx..xx', 'avg_len(res)==1.4', 'PREFIXES_15_1k'),
            ('xxx', 'NON_EXISTING', 'NON_WORDS_1k'),
        ]
        for xxx, avg, data in _bench_data:
            for meth in ['keys', 'items']:
                bench(
                    '%s.%s(prefix="%s"), %s' % (struct_name, meth, xxx, avg),
                    timeit.Timer(
                        "for word in %s: data.%s(word)" % (data, meth),
                        setup
                    ),
                    'K ops/sec',
                    op_count=1,
                    runs=3,
                    text_width=60,
                )
            for meth in ['iterkeys', 'iteritems']:
                bench(
                    '%s.%s(prefix="%s"), %s' % (struct_name, meth, xxx, avg),
                    timeit.Timer(
                        "for word in %s: list(data.%s(word))" % (data, meth),
                        setup
                    ),
                    'K ops/sec',
                    op_count=1,
                    runs=3,
                    text_width=60,
                )


def check_dawg(trie, words):
    value = 0
    for word in words:
        value += trie[word]
    if value != len(words):
        raise Exception()

def profiling():
    import pstats
    import cProfile
    print('\n====== Profiling =======\n')
    d = create_bytes_dawg()
    WORDS = words100k()

    def check_getitem(trie, words):
        for word in words:
            trie[word]

    cProfile.runctx("check_getitem(d, WORDS)", globals(), locals(), "Profile.prof")

#    def check_prefixes(trie, words):
#        for word in words:
#            trie.keys(word)
#    cProfile.runctx("check_prefixes(d, NON_WORDS_1k)", globals(), locals(), "Profile.prof")
#
    #cProfile.runctx("check_trie(d, WORDS)", globals(), locals(), "Profile.prof")

    s = pstats.Stats("Profile.prof")
    s.strip_dirs().sort_stats("time").print_stats(20)


if __name__ == '__main__':

    benchmark()
    #profiling()
    print('\n~~~~~~~~~~~~~~\n')