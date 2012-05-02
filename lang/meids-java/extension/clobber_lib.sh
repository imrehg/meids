#!/bin/bash
#
# This script is responsible for uninstalling
# the ME-iDS Java extension library
#

echo ""
echo "==========================================================="
echo " This script uninstalls the ME-iDS Java extension library. "
echo "==========================================================="
echo ""

if [ -z ${LIBRARY} ] ; then
    echo "ERROR: No library to uninstall defined!"
    exit 1
fi

if [ -z ${DESTINATION} ] ; then
	DESTINATION=/usr/lib
fi

# Only root can do the library uninstallation
if [ $(whoami) != root ] ; then
	echo "ERROR: You must be root to uninstall the library!"
	exit 1
fi

if [ ! -f ${DESTINATION}/${LIBRARY} ] ; then
	echo "ERROR: Library ${DESTINATION}/${LIBRARY} not found!"
	exit 1
fi

echo "Uninstall ${DESTINATION}/${LIBRARY%%.[0-9]*} library and related links."
if rm ${DESTINATION}/${LIBRARY%%.[0-9]*}*
then
	echo "Run ldconfig on ${DESTINATION}."
	if /sbin/ldconfig -n ${DESTINATION}
	then
		echo ""
		echo "Uninstallation was successful."
		echo ""
		exit 0
	else
		echo "ERROR: Cannot run ldconfig on ${DESTINATION}!"
		exit 1
	fi
else
	echo "ERROR: Cannot remove ${DESTINATION}/${LIBRARY%.[0-9].[0-9]} library!"
	exit 1
fi
