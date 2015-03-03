from distutils.core import setup
from distutils.core import Extension
from distutils.command.build_ext import build_ext as _build_ext

import sys

modules = [
	Extension('wiringX.gpio', sources=['wiringX/wiringx.c', '../wiringX.c', '../hummingboard.c', '../bananapi.c', '../radxa.c', '../raspberrypi.c', '../ci20.c'], include_dirs=['../'], extra_compile_args=['-Wformat=0']),
]

setup(
    name='wiringX',
    version='0.6',
    author='CurlyMo',
    author_email='curlymoo1@gmail.com',
    url='https://www.wiringx.org/',
    license='GPLv2',
    packages=['wiringX'],
    description='Control GPIO and I2C',
    classifiers=['Environment :: Console',
                 'Intended Audience :: Developers',
                 'Intended Audience :: Education',
                 'License :: OSI Approved :: MIT License',
                 'Operating System :: POSIX :: Linux',
                 'Programming Language :: Python',
                 'Topic :: Home Automation',
                 'Topic :: Software Development :: Embedded Systems'
    ],
    ext_modules=modules
)
