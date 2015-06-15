from distutils.core import setup
from distutils.core import Extension
from distutils.command.build_ext import build_ext as _build_ext

import sys

modules = [
	Extension('wiringX.gpio', sources=['wiringX/wiringx.c', '../src/wiringX.c', '../src/hummingboard.c', '../src/bananapi.c', '../src/radxa.c', '../src/raspberrypi.c', '../src/ci20.c'], include_dirs=['../src/'], extra_compile_args=['-Wformat=0']),
]

setup(
    name='wiringX',
    version='1.0',
    author='CurlyMo',
    author_email='curlymoo1@gmail.com',
    url='https://www.wiringx.org/',
    license='GPLv3',
    packages=['wiringX'],
    description='Cross-platform GPIO Interface',
    classifiers=['Environment :: Console',
                 'Intended Audience :: Developers',
                 'Intended Audience :: Education',
                 'License :: OSI Approved :: GPL-3.0 License',
                 'Operating System :: POSIX :: Linux',
                 'Programming Language :: Python',
                 'Topic :: Home Automation',
                 'Topic :: Software Development :: Embedded Systems'
    ],
    ext_modules=modules
)
