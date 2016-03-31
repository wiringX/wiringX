from distutils.core import setup
from distutils.core import Extension
from distutils.command.build_ext import build_ext as _build_ext

import sys

modules = [
	Extension('wiringX.gpio', 
		sources=[
			'wiringX/wiringx.c',
			'../src/i2c-dev.c',
			'../src/wiringX.c',
			'../src/soc/soc.c',
			'../src/soc/broadcom/2836.c',
			'../src/soc/broadcom/2835.c',
			'../src/soc/allwinner/a31s.c',
			'../src/soc/allwinner/a10.c',
			'../src/soc/nxp/imx6sdlrm.c',
			'../src/soc/nxp/imx6dqrm.c',
			'../src/platform/solidrun/hummingboard_sdl.c',
			'../src/platform/solidrun/hummingboard_edge.c',
			'../src/platform/raspberrypi/raspberrypi1b+.c',
			'../src/platform/raspberrypi/raspberrypi3.c',
			'../src/platform/raspberrypi/raspberrypi2.c',
			'../src/platform/raspberrypi/raspberrypi1b1.c',
			'../src/platform/raspberrypi/raspberrypi1b2.c',
			'../src/platform/linksprite/pcduino1.c',
			'../src/platform/lemaker/bananapim2.c',
			'../src/platform/platform.c'

		], 
		include_dirs=['../src/'],
		extra_compile_args=['-Wformat=0']
	),
]

setup(
    name='wiringX',
    version='2.0',
    author='CurlyMo',
    author_email='curlymoo1@gmail.com',
    url='https://www.wiringx.org/',
    license='MPLv2',
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
