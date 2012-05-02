#!/bin/bash
#
# This script is responsible for installing
# the meilhaus driver system library
#
# Version 1.3
#
# Change Log:
#
# 28-05-2008
#	* Initial release.
# 30-06-2008
#	* Adding installation of configuration files.
# 04-03-2009
#	* Adding processor detection.
# 05-03-2009
#	* Moving libraries to MEiDS sub-driectory.


echo ""
echo "=========================================================="
echo " This script installs the Meilhaus libraries (MEiDS). "
echo "=========================================================="
echo ""


INCLUDE=/usr/include/medriver
PROCESSOR_TYPE=`uname -p`

if [ -z ${LIBRARY} ] ; then
    echo "ERROR: No library to install defined!"
    exit 1
fi

if [ ! -f ${LIBRARY} ] ; then
    echo"ERROR: Library ${LIBRARY} not in place. Please rebuild it!"
    exit 1
fi


#Only root can do the library installation
if [ $(whoami) != root ] ; then
	echo "ERROR: You must be root to install the library!"
	exit 1
fi

# Copy configuration files
CONFIGDIR=/etc/medriver/
echo "Install configuration files to ${CONFIGDIR}"
if [ ! -d ${CONFIGDIR} ] ; then
	mkdir -m 0777 -p ${CONFIGDIR}
fi

install -m 0444 ../config/meconfig.xml ../config/medrvconfig.dtd ../config/meids.config ${CONFIGDIR}
echo ""

if  [ -d ${INCLUDE} ]
then
	rm -r ${INCLUDE}
fi
mkdir ${INCLUDE}

echo "Copy headers to ${INCLUDE}"
cp -f ../osi/*.h ${INCLUDE}
echo ""

if [ $PROCESSOR_TYPE == "x86_64" -a -d /usr/lib64 ] ; then
	echo "64 bit system detected."
	DESTINATION=/usr/lib64
else
	DESTINATION=/usr/lib
fi

if [ ! -d ${DESTINATION} ] ; then
	echo "ERROR: Cannot install library. '${DESTINATION}' doesn't exist!"
	exit 1
fi

if [ ! -d ${DESTINATION}/MEiDS ] ; then
	mkdir ${DESTINATION}/MEiDS
fi

echo "Copy ${LIBRARY} to ${DESTINATION}/MEiDS"
if cp -f -d ${LIBRARY}* ${DESTINATION}/MEiDS
then
	strip ${DESTINATION}/MEiDS/${LIBRARY}
	echo "Run ldconfig on ${DESTINATION}/MEiDS"
	if /sbin/ldconfig -n ${DESTINATION}/MEiDS
	then
		echo "Installation  in ${DESTINATION}/MEiDS was successful."
	else
		echo "ERROR: Cannot run ldconfig on ${DESTINATION}/MEiDS!"
		exit 1
	fi

	ln -f -s ${DESTINATION}/MEiDS/${LIBRARY} /usr/lib/${LIBRARY}
	if [ $PROCESSOR_TYPE == "x86_64" -a -d /usr/lib64 ] ; then
		ln -f -s ${DESTINATION}/MEiDS/${LIBRARY} /usr/lib/${LIBRARY}
	/sbin/ldconfig
	fi

else
	echo "ERROR: Cannot copy ${LIBRARY} to ${DESTINATION}/MEiDS!"
	exit 1
fi
echo ""

exit 0

