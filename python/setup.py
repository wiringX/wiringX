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
			'../src/soc/allwinner/a10.c',
			'../src/soc/allwinner/a31s.c',
			'../src/soc/amlogic/s805.c',
			'../src/soc/amlogic/s905.c',
			'../src/soc/broadcom/2835.c',
			'../src/soc/broadcom/2836.c',
			'../src/soc/nxp/imx6sdlrm.c',
			'../src/soc/nxp/imx6dqrm.c',
			'../src/soc/samsung/exynos5422.c',
			'../src/platform/platform.c',
			'../src/platform/hardkernel/odroidc1.c',
			'../src/platform/hardkernel/odroidc2.c',
			'../src/platform/hardkernel/odroidxu4.c',
			'../src/platform/lemaker/bananapi1.c',
			'../src/platform/lemaker/bananapim2.c',
			'../src/platform/linksprite/pcduino1.c',
			'../src/platform/raspberrypi/raspberrypi1b1.c',
			'../src/platform/raspberrypi/raspberrypi1b2.c',
			'../src/platform/raspberrypi/raspberrypi1b+.c',
			'../src/platform/raspberrypi/raspberrypi2.c',
			'../src/platform/raspberrypi/raspberrypi3.c',
			'../src/platform/raspberrypi/raspberrypizero.c',
			'../src/platform/solidrun/hummingboard_base_pro_dq.c',
			'../src/platform/solidrun/hummingboard_base_pro_sdl.c',
			'../src/platform/solidrun/hummingboard_gate_edge_dq.c',
			'../src/platform/solidrun/hummingboard_gate_edge_sdl.c'
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
