#!/bin/bash
#
# This script is responsible for uninstalling
# the meilhaus driver system library
#

CONFIGDIR=/etc/medriver/

echo ""
echo "============================================================"
echo " This script uninstalls the Meilhaus device driver library. "
echo "============================================================"
echo ""

echo 
if [ -z ${LIBRARY} ] ; then
    echo "ERROR: No library to uninstall defined!"
    exit 1
fi

DESTINATION_32=/usr/lib/MEiDS
DESTINATION_64=/usr/lib64/MEiDS

# Only root can do the library uninstallation
if [ $(whoami) != root ] ; then
	echo "ERROR: You must be root to uninstall the library!"
	exit 1
fi

# Remove the configuration stuff
echo " "
echo "Removing ${CONFIGDIR}."
echo " "
rm -f -r ${CONFIGDIR}

if [ -d /usr/lib ] ; then
	rm -f /usr/lib/${LIBRARY}*
fi
if [ -d /usr/lib64 ] ; then
	rm -f /usr/lib64/${LIBRARY}*
fi
/sbin/ldconfig

if [ -d ${DESTINATION_32} ] ; then
	echo "Uninstall ${DESTINATION_32}/${LIBRARY} library and related links."
	if rm -f ${DESTINATION_32}/${LIBRARY}*
	then
		echo "Run ldconfig on ${DESTINATION_32}"
		if /sbin/ldconfig -n ${DESTINATION_32}
		then
			echo ""
			echo "Uninstallation from ${DESTINATION_32} was successful."
		else
			echo "ERROR: Cannot run ldconfig on ${DESTINATION_32}!"
		fi
	else
		echo "ERROR: Cannot remove ${DESTINATION_32}/${LIBRARY} library!"
		exit 1
	fi
fi

if [ -d ${DESTINATION_64} ] ; then
	echo "Uninstall ${DESTINATION_64}/${LIBRARY} library and related links."
	if rm -f ${DESTINATION_64}/${LIBRARY}*
	then
		echo "Run ldconfig on ${DESTINATION_64}"
		if /sbin/ldconfig -n ${DESTINATION_64}
		then
			echo ""
			echo "Uninstallation from ${DESTINATION_64} was successful."
			echo ""
		else
			echo "ERROR: Cannot run ldconfig on ${DESTINATION_64}!"
			exit 1
		fi
	fi
fi

exit 0
