#!/usr/bin/env python

import os
from setuptools import setup

def read(fname):
    return open(os.path.join(os.path.dirname(__file__), fname)).read()

setup(name='hudkeywords',
      version='1.0',
      description='HUD keywords tool',
      author='Pete Woods',
      author_email='pete.woods@canonical.com',
      scripts=['bin/hudkeywords'],
      packages=['hudkeywords', 'tests'],
      url='http://launchpad.net/hud',
      long_description=read('README'),
      test_suite='tests'
     )