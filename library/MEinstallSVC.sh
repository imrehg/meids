#!/bin/bash
#
# This script is responsible for installing a meilhaus server
#

if [ -z ${SERVER} ] ; then
    echo "No server to install defined."
    exit 1
fi

if [ ! -x ${SERVER} ] ; then
    echo "Server ${SERVER} not in place. Please rebuild it."
    exit 1
fi

# Only root can do the server installation
if [ $(whoami) != root ] ; then
	echo "You must be root to install the server!"
	exit 1
fi

# Install the server
echo "Copy server ${SERVER} to /usr/sbin/"
cp ${SERVER} /usr/sbin/

# Install the init script
echo "Copy init script rc.${SERVER} to /etc/init.d/"
cp rc.${SERVER} /etc/init.d/${SERVER}

# Let do install_initd the runlevel setup
echo "Setup runlevel entries."
/usr/lib/lsb/install_initd ${SERVER}

# Create symbolic link
echo "Create symbolic link /usr/sbin/rc${SERVER} to /etc/init.d/${SERVER}"
ln -s -f /etc/init.d/${SERVER} /usr/sbin/rc${SERVER}
