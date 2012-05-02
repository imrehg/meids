#!/bin/bash
#
# This script is responsible for removing old (obsolate) drivers.
# The legacy driver and MEiDS_v1.2.29 are now part of kernel tree.
# Meilhaus Electronic do no support it in kernel 2.6.27 and higher
# It is conflicting with current ME-iDS therefore has to be removed.
#
# All staging drivers.
#
# Version 2.0
#

KERNEL_VER=`uname -r`
MODDIRS="/lib/modules/${KERNEL_VER}/kernel/drivers/staging/me4000 /lib/modules/${KERNEL_VER}/kernel/drivers/staging/meilhaus"
MODNAMES="memain me0600 me0900 me1000 me1400 me1600 me4600 me6000 me8100 me8200 medummy me4000"

# Only root is allowed to remove a module
if [ $(whoami) != root ]; then
	echo "You must be root!"
	exit 1
fi

# Remove directory
for ME_DIR in ${MODDIRS}
do
	rm -f -r ${ME_DIR}
done

# Update dependency
/sbin/depmod -a

# Remove objects
for ME_MOD in ${MODNAMES}
do
	/sbin/modprobe -r ${ME_MOD} 2>>/dev/null
	/sbin/rmmod ${ME_MOD} 2>>/dev/null
done

exit 0