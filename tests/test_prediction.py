# -*- coding: utf-8 -*-
from __future__ import absolute_import, unicode_literals
import pytest
import dawg

class TestPrediction(object):
    DATA = ['ЁЖИК', 'ЁЖИКЕ', 'ЁЖ', 'ДЕРЕВНЯ', 'ДЕРЁВНЯ', 'ЕМ', 'ОЗЕРА', 'ОЗЁРА', 'ОЗЕРО']
    LENGTH_DATA = list(zip(DATA, ((len(w),) for w in DATA)))

    REPLACES = dawg.DAWG.compile_replaces({'Е': 'Ё'})

    SUITE = [
        ('УЖ', []),
        ('ЕМ', ['ЕМ']),
        ('ЁМ', []),
        ('ЁЖ', ['ЁЖ']),
        ('ЕЖ', ['ЁЖ']),
        ('ЁЖИК', ['ЁЖИК']),
        ('ЕЖИКЕ', ['ЁЖИКЕ']),
        ('ДЕРЕВНЯ', ['ДЕРЕВНЯ', 'ДЕРЁВНЯ']),
        ('ДЕРЁВНЯ', ['ДЕРЁВНЯ']),
        ('ОЗЕРА', ['ОЗЕРА', 'ОЗЁРА']),
        ('ОЗЕРО', ['ОЗЕРО']),
    ]

    SUITE_ITEMS = [
        (
            it[0], # key
            [
                (w, [(len(w),)]) # item, value pair
                for w in it[1]
            ]
        )
        for it in SUITE
    ]

    SUITE_VALUES = [
        (
            it[0], # key
            [[(len(w),)] for w in it[1]]
        )
        for it in SUITE
    ]


    @pytest.mark.parametrize(("word", "prediction"), SUITE)
    def test_dawg_prediction(self, word, prediction):
        d = dawg.DAWG(self.DATA)
        assert d.similar_keys(word, self.REPLACES) == prediction

    @pytest.mark.parametrize(("word", "prediction"), SUITE)
    def test_record_dawg_prediction(self, word, prediction):
        d = dawg.RecordDAWG(str("=H"), self.LENGTH_DATA)
        assert d.similar_keys(word, self.REPLACES) == prediction

    @pytest.mark.parametrize(("word", "prediction"), SUITE_ITEMS)
    def test_record_dawg_items(self, word, prediction):
        d = dawg.RecordDAWG(str("=H"), self.LENGTH_DATA)
        assert d.similar_items(word, self.REPLACES) == prediction

    @pytest.mark.parametrize(("word", "prediction"), SUITE_VALUES)
    def test_record_dawg_items_values(self, word, prediction):
        d = dawg.RecordDAWG(str("=H"), self.LENGTH_DATA)
        assert d.similar_item_values(word, self.REPLACES) == prediction
