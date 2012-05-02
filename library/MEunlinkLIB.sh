#!/bin/bash
#
# This script is responsible for installing
# the meilhaus driver system library
#
# Version 1.0
#
# Change Log:
#
# 04-03-2009
#	* Initial release.

LINUX_NAME=medriver
WINDOWS_NAME=meIDSmain
DLL_NAME=MEiDS

PROCESSOR_TYPE=`uname -p`

#Only root can do the library installation
if [ $(whoami) != root ] ; then
	echo "ERROR: You must be root to make library unlinking!"
	exit 1
fi

if [ -d /usr/lib ] ; then
	if [ -z ${LIBRARY} ] ; then
		rm -f /usr/lib/lib${LINUX_NAME}.so
		rm -f /usr/lib/lib${WINDOWS_NAME}.so
		rm -f /usr/lib/lib${DLL_NAME}.so
	else
		rm -f /usr/lib/${LIBRARY}
	fi
fi

if [ -d /usr/lib64 ] ; then
	if [ -z ${LIBRARY} ] ; then
		rm -f /usr/lib64/lib${LINUX_NAME}.so
		rm -f /usr/lib64/lib${WINDOWS_NAME}.so
		rm -f /usr/lib64/lib${DLL_NAME}.so
	else
		rm -f /usr/lib64/${LIBRARY}
	fi
fi

/sbin/ldconfig
exit 0

