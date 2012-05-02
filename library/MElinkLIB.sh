#!/bin/bash
#
# This script is responsible for installing
# the meilhaus driver system library
#
# Version 1.1
#
# Change Log:
#
# 04-03-2009
#	* Initial release.
# 05-03-2009
#	* Add links into /usr/lib also for 64 bits systems (some application like LabView need it).


LINUX_NAME=medriver
WINDOWS_NAME=meIDSmain
DLL_NAME=MEiDS

PROCESSOR_TYPE=`uname -p`

if [ -z ${LIBRARY} ] ; then
    echo "ERROR: No library to link defined!"
    exit 1
fi

#Only root can do the library installation
if [ $(whoami) != root ] ; then
	echo "ERROR: You must be root to make library linking!"
	exit 1
fi

if [ $PROCESSOR_TYPE == "x86_64" -a -d /usr/lib64 ] ; then
	DESTINATION=/usr/lib64/MEiDS
else
	DESTINATION=/usr/lib/MEiDS
fi

if [ ! -f ${DESTINATION}/${LIBRARY} ] ; then
    echo"ERROR: Library ${LIBRARY} not in place. Please rebuild it!"
    exit 1
fi

echo "Creating aliases in /usr/lib."
echo " "
ln -f -s ${DESTINATION}/${LIBRARY} /usr/lib/lib${LINUX_NAME}.so
ln -f -s ${DESTINATION}/${LIBRARY} /usr/lib/lib${WINDOWS_NAME}.so
ln -f -s ${DESTINATION}/${LIBRARY} /usr/lib/lib${DLL_NAME}.so
# ln -f -s ${DESTINATION}/${LIBRARY} /usr/lib/${LIBRARY}

if [ $PROCESSOR_TYPE == "x86_64" -a -d /usr/lib64 ] ; then
    echo "Creating aliases in /usr/lib64."
    echo " "
    ln -f -s ${DESTINATION}/${LIBRARY} /usr/lib64/lib${LINUX_NAME}.so
    ln -f -s ${DESTINATION}/${LIBRARY} /usr/lib64/lib${WINDOWS_NAME}.so
    ln -f -s ${DESTINATION}/${LIBRARY} /usr/lib64/lib${DLL_NAME}.so
#     ln -f -s ${DESTINATION}/${LIBRARY} /usr/lib64/${LIBRARY}
fi
/sbin/ldconfig
exit 0

