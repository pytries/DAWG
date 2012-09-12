# -*- coding: utf-8 -*-
from __future__ import absolute_import, unicode_literals
import pytest
import dawg

class TestPrediction(object):
    DATA = ['ЁЖИК', 'ЁЖИКЕ', 'ЁЖ', 'ДЕРЕВНЯ', 'ДЕРЁВНЯ', 'ЕМ', 'ОЗЕРА', 'ОЗЁРА', 'ОЗЕРО']
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

    @pytest.mark.parametrize(("word", "prediction"), SUITE)
    def test_dawg_prediction(self, word, prediction):
        d = dawg.DAWG(self.DATA)
        assert d.similar_keys(word, self.REPLACES) == prediction

    @pytest.mark.parametrize(("word", "prediction"), SUITE)
    def test_record_dawg_prediction(self, word, prediction):
        format = "=H"
        data = zip(self.DATA, ((len(w),) for w in self.DATA))
        d = dawg.RecordDAWG(str(format), data)
        assert d.similar_keys(word, self.REPLACES) == prediction
