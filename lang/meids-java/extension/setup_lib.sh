#!/bin/bash
#
# This script is responsible for installing
# the ME-iDS Java extension library
#

echo ""
echo "========================================================="
echo " This script installs the ME-iDS Java extension library. "
echo "========================================================="
echo ""

if [ -z ${LIBRARY} ] ; then
    echo "ERROR: No library to install defined!"
    exit 1
fi

if [ ! -f ${LIBRARY} ] ; then
    echo"ERROR: Library ${LIBRARY} not in place. Please rebuild it!"
    exit 1
fi

if [ -z ${DESTINATION} ] ; then
	DESTINATION=/usr/lib
fi

# Only root can do the library installation
if [ $(whoami) != root ] ; then
	echo "ERROR: You must be root to install the library!"
	exit 1
fi

echo "Copy ${LIBRARY} to ${DESTINATION}."
if cp ${LIBRARY} ${DESTINATION}
then
	echo "Run ldconfig on ${DESTINATION}."
	if /sbin/ldconfig -n ${DESTINATION}
	then
		echo "Create symbolic link ${LIBRARY%%.[0-9]*} to ${LIBRARY}."
		if ln -f -s ${DESTINATION}/${LIBRARY} ${DESTINATION}/${LIBRARY%%.[0-9]*}
		then
			echo ""
			echo "Installation was successful."
			echo ""
			exit 0
		else
			echo "ERROR: Cannot create symbolic link ${LIBRARY%%.[0-9]*} to ${LIBRARY}!"
			exit 1
		fi
	else
		echo "ERROR: Cannot run ldconfig on ${DESTINATION}!"
		exit 1
	fi
else
	echo "ERROR: Cannot copy ${LIBRARY} to ${DESTINATION}!"
	exit 1
fi


