#!/usr/bin/python

from distutils.core import setup, Extension
import os
import sys


if os.name == 'nt':
	include_dirs = ['../../common']
	library_dirs = ['C:/WINDOWS/system32']
	macro = [('ME_WINDOWS', None)]
	library = ['meIDSmain']
elif os.name == 'posix':
	include_dirs = ['../../common', '/usr/lib/python/site-packages/numpy/core/include']
	library_dirs = ['.']
	macro = [('ME_POSIX', None)]
	library = ['MEiDS']
else:
	print "Error: Unknown Operating System"
	sys.exit(1)

module = Extension(
	'meDriver',
	 define_macros = macro,
	 include_dirs = include_dirs,
	 sources = ['meDriverModule.c'],
	 libraries = library,
	 library_dirs = library_dirs)

setup(
	name = 'meids-python',
	description = 'Extension module for the ME-iDS.',
	version = '0.0.4',
	author="Guenter Gebhardt & Krzysztof Gantzke",
	author_email="support@meilhaus.de",
	url="http://www.meilhaus.de",
	license="GNU LGPL",
	long_description="""Extension module which enables access
to the ME-iDS C-library providing the API described in the ME-iDS manual.""",
	ext_modules = [module])
