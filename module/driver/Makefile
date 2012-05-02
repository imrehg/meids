#
# Makefile for Meilhaus linux driver system PCI modules
# /home/workspace/newMEiDS/module/driver
#

#
# Set this to the root of your kernel source tree.  This must match
# the version of the kernel for which you are building the meilhaus driver
# as the modules version information and the header files must match.
#

DRV_VERSION := 0x00010357


ifndef LINUX_SRC
LINUX_SRC := /usr/src/linux
#LINUX_SRC := /home/ggebhardt/kernels/linux-2.6.21.3
endif

ifndef MODULE_DEST_EXT
MODULE_DEST_EXT := meids
endif
ifdef KERNEL_VER
MODULE_DEST := /lib/modules/$(KERNEL_VER)/extra/$(MODULE_DEST_EXT)
else
MODULE_DEST := /lib/modules/extra/$(MODULE_DEST_EXT)
endif

ifndef LINUX_UDEV_DIR
LINUX_UDEV_DIR := /etc/udev
endif

ifndef KERNEL_VER
KERNEL_VER := default
endif

EXTRA_CFLAGS := -DME_$(MEiDS_EXT)
EXTRA_CFLAGS += -DKERNEL_RELEASE='"${KERNEL_VER}/"'
#EXTRA_CFLAGS += -DME_SYNAPSE

LOCAL_CFLAGS := -Wall

ifndef MEiDS_SRC
# PWD variable is sometimes not available!
ifndef PWD
PWD := $(shell pwd)
endif
MEiDS_SRC := $(PWD)
endif

ifndef MEiDS_BIN_DIR
MEiDS_BIN_DIR := $(MEiDS_SRC)/../bin
endif

ifndef MEiDS_INCLUDE_DIR
MEiDS_INCLUDE_DIR := $(MEiDS_SRC)/../../include
endif

ifndef MEiDS_UDEV_DIR
MEiDS_UDEV_DIR := $(MEiDS_SRC)/../udev
endif


MEHARDWARE := mehardware_access.o

ifeq ($(MEiDS_EXT),USB)
MEHARDWARE += NET2282_access.o
endif

#Version
EXTRA_CFLAGS += -DME_VERSION_DRIVER=${DRV_VERSION}

#Test build
# EXTRA_CFLAGS += -DMEDEBUG_INFO
# EXTRA_CFLAGS += -DMEDEBUG_DEBUG
# EXTRA_CFLAGS += -DMEDEBUG_ERROR
# EXTRA_CFLAGS += -DMEDEBUG_TIMESTAMPS


#
# Let the kernel build system do the job of generating the driver modules
#

#drivers
ifeq ($(MEiDS_EXT),MEPHISTO)
obj-m := meMEPHISTO.o
meMEPHISTO-objs := mephisto.o
meMEPHISTO-objs += memain_common.o
meMEPHISTO-objs += medevice.o medlist.o medlock.o mesubdevice.o meslist.o meslock.o
meMEPHISTO-objs += me_spin_lock.o
meMEPHISTO-objs += mephisto_access.o
meMEPHISTO-objs += mephisto_device.o
meMEPHISTO-objs += mephisto_dio.o
meMEPHISTO-objs += mephisto_ai.o meseg_buf.o

else #MEPHISTO

obj-m := memain$(MEiDS_EXT).o
obj-m += me0600$(MEiDS_EXT).o
obj-m += me0700$(MEiDS_EXT).o
obj-m += me0900$(MEiDS_EXT).o
obj-m += me1000$(MEiDS_EXT).o
obj-m += me1400$(MEiDS_EXT).o
obj-m += me1600$(MEiDS_EXT).o
obj-m += me4600$(MEiDS_EXT).o
obj-m += me4700$(MEiDS_EXT).o
obj-m += me6000$(MEiDS_EXT).o
obj-m += me8100$(MEiDS_EXT).o
obj-m += me8200$(MEiDS_EXT).o

memain$(MEiDS_EXT)-objs := $(MEHARDWARE) memain_common.o memain_$(MEiDS_EXT).o

me0600$(MEiDS_EXT)-objs := $(MEHARDWARE) medevice.o medlist.o medlock.o me0600_device.o
me0600$(MEiDS_EXT)-objs += mesubdevice.o meslist.o meslock.o me_spin_lock.o me0600_relay.o me0600_di.o me0600_dio.o me0600_ext_irq.o

me0700$(MEiDS_EXT)-objs := $(MEHARDWARE) medevice.o medlist.o medlock.o mefirmware.o meeeprom.o me0700_device.o
me0700$(MEiDS_EXT)-objs += mesubdevice.o meslist.o meslock.o me_spin_lock.o me0700_bus.o me4600_dio.o me4600_ai.o me0700_ai.o me4600_ao.o me8254.o me4600_ext_irq.o meseg_buf.o

me0900$(MEiDS_EXT)-objs := $(MEHARDWARE) medevice.o medlist.o medlock.o me0900_device.o
me0900$(MEiDS_EXT)-objs += mesubdevice.o meslist.o meslock.o me_spin_lock.o me0900_do.o me0900_di.o

me1000$(MEiDS_EXT)-objs := $(MEHARDWARE) medevice.o medlist.o medlock.o me1000_device.o
me1000$(MEiDS_EXT)-objs += mesubdevice.o meslist.o meslock.o me_spin_lock.o me1000_dio.o

me1400$(MEiDS_EXT)-objs := $(MEHARDWARE) medevice.o medlist.o medlock.o me1400_device.o
me1400$(MEiDS_EXT)-objs += mesubdevice.o meslist.o meslock.o me_spin_lock.o me8255.o me8254.o me1400AB_ext_irq.o me1400CD_ext_irq.o

me1600$(MEiDS_EXT)-objs := $(MEHARDWARE) medevice.o medlist.o medlock.o me1600_device.o
me1600$(MEiDS_EXT)-objs += mesubdevice.o meslist.o meslock.o me_spin_lock.o me1600_ao.o

me4600$(MEiDS_EXT)-objs := $(MEHARDWARE) medevice.o medlist.o medlock.o mefirmware.o meeeprom.o me4600_device.o
me4600$(MEiDS_EXT)-objs += mesubdevice.o meslist.o meslock.o me_spin_lock.o me4600_dio.o me8254.o me4600_ai.o me4600_ao.o me4600_ext_irq.o meseg_buf.o

me4700$(MEiDS_EXT)-objs := $(MEHARDWARE) medevice.o medlist.o medlock.o mefirmware.o meeeprom.o me4700_device.o
me4700$(MEiDS_EXT)-objs += mesubdevice.o meslist.o meslock.o me_spin_lock.o me4600_dio.o me4600_ai.o me4600_ao.o me8254.o me4700_fi.o me4700_fo.o me4600_ext_irq.o meseg_buf.o

me6000$(MEiDS_EXT)-objs := $(MEHARDWARE) medevice.o medlist.o medlock.o mefirmware.o me6000_device.o
me6000$(MEiDS_EXT)-objs += mesubdevice.o meslist.o meslock.o me_spin_lock.o me6000_dio.o me6000_ao.o

me8100$(MEiDS_EXT)-objs := $(MEHARDWARE) medevice.o medlist.o medlock.o me8100_device.o
me8100$(MEiDS_EXT)-objs += mesubdevice.o meslist.o meslock.o me_spin_lock.o me8100_di.o me8100_do.o me8254.o

me8200$(MEiDS_EXT)-objs := $(MEHARDWARE) medevice.o medlist.o medlock.o me8200_device.o
me8200$(MEiDS_EXT)-objs += mesubdevice.o meslist.o meslock.o me_spin_lock.o me8200_di.o me8200_do.o me8200_dio.o

endif #MEPHISTO


LOCAL_CFLAGS += -I$(MEiDS_INCLUDE_DIR) -I$(MEiDS_SRC)

KBUILD_CPPFLAGS += $(LOCAL_CFLAGS)
CPPFLAGS += $(LOCAL_CFLAGS)

default: help

.PHONY: modules
modules: test_ext delete clean
	@rm -f *$(MEiDS_EXT).ko *.o .me* *.mod.c
	@make -C ${LINUX_SRC} M=$(MEiDS_SRC) modules
	@if test ! -d $(MEiDS_BIN_DIR); then mkdir -p --mode=0777 $(MEiDS_BIN_DIR); fi
	@install -m 0444 $(MEiDS_SRC)/me*$(MEiDS_EXT)*.ko -t $(MEiDS_BIN_DIR)

.PHONY: clean
clean:
	@make -C ${LINUX_SRC} M=$(MEiDS_SRC) clean
	@rm -f *.ko *.o .me* *.mod.c

.PHONY: clear
clear: clean
	@rm -f *~

.PHONY: delete
delete:
	@rm -f $(MEiDS_BIN_DIR)/me*$(MEiDS_EXT)*.ko

.PHONY: install
install: test_ext
	@if test ! -d $(MODULE_DEST); then mkdir -p --mode=0755 $(MODULE_DEST); fi
	@install -m 0444 $(MEiDS_BIN_DIR)/me*$(MEiDS_EXT)*.ko -t $(MODULE_DEST)
	@/sbin/depmod -a
	@install -m 0444 $(MEiDS_UDEV_DIR)/*-meids.rules -t $(LINUX_UDEV_DIR)

.PHONY: uninstall
uninstall: test_ext
	@rm -f $(MODULE_DEST)/me*$(MEiDS_EXT)*.ko
	@/sbin/depmod -a
	@rm -f $(LINUX_UDEV_DIR)/*-meids.rules

.PHONY: test_ext
test_ext:
ifndef MEiDS_EXT
	@echo "No modules' extension defined!"
	@false
endif

.PHONY: help
help:
	@echo "  MEiDS drivers."
	@echo "  Syntax: make [options] target."
	@echo
	@echo "  options:"
	@echo "    LINUX_SRC=kernel source directory (default: /usr/src/linux)"
	@echo "    MEiDS_BIN_DIR=destination directory"
	@echo "    MEiDS_EXT=modules' extention (PCI or USB)"
	@echo
	@echo "  targets:"
	@echo "    modules		- build modules"
	@echo "    install		- install modules"
	@echo "    uninstall		- uninstall modules"
	@echo
	@echo "    clean		- remove temporary files"
	@echo "    clear		- remove temporary and backup files"
	@echo "    delete		- delete modules from destination directory"
	@echo
