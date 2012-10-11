#! /usr/bin/env python
import glob
from distutils.core import setup
from distutils.extension import Extension

setup(
    name="DAWG",
    version="0.5.1",
    description="Fast and memory efficient DAWG for Python",
    long_description = open('README.rst').read() + open('CHANGES.rst').read(),
    author='Mikhail Korobov',
    author_email='kmike84@gmail.com',
    url='https://github.com/kmike/DAWG/',

    ext_modules = [
        Extension(
            "dawg",
            sources = glob.glob('src/*.cpp') + glob.glob('lib/b64/*.c'),
            include_dirs=['lib'],
            language = "c++",
        )
    ],

    classifiers=[
        'Development Status :: 3 - Alpha',
        'Intended Audience :: Developers',
        'Intended Audience :: Science/Research',
        'License :: OSI Approved :: MIT License',
        'Programming Language :: Cython',
        'Programming Language :: Python',
        'Programming Language :: Python :: 2',
        'Programming Language :: Python :: 2.6',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.2',
        'Programming Language :: Python :: 3.3',
        'Programming Language :: Python :: Implementation :: CPython',
        'Topic :: Software Development :: Libraries :: Python Modules',
        'Topic :: Scientific/Engineering :: Information Analysis',
        'Topic :: Text Processing :: Linguistic',
    ],
)
