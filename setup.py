#!/usr/bin/env python

from distutils.core import setup, Extension

module1 = Extension(
    'snappy',
    sources = ['snappy.c', 'capture.c', 'debug.c', 'logging.c']
)

setup(
    name = "snappy",
    version= "1.0",
    description = "Take pictures with webcam",
    ext_modules = [module1]
)
