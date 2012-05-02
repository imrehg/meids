#!/bin/bash
#
# This script is responsible for uninstalling
# the meilhaus driver server
#

echo ""
echo "============================================================"
echo " This script uninstalls the Meilhaus device driver server."
echo "============================================================"
echo ""

if [ -x /usr/sbin/rc${SERVER} ] ; then
    rc${SERVER} stop
    /usr/lib/lsb/remove_initd ${SERVER}
    rm -f /usr/sbin/rc${SERVER}
    rm -f /etc/init.d/${SERVER}
    rm -f /usr/sbin/${SERVER}
fi

echo ""
echo "Uninstallation was successful."
echo ""
exit 0
