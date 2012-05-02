#
# Makefile for Meilhaus linux driver system
# /home/workspace/newMEiDS/module
#

# PWD variable is sometimes not available!
ifndef PWD
PWD := $(shell pwd)
endif
ifndef LINUX_SRC
LINUX_SRC := /usr/src/linux
endif

MEiDS_SRC_DRV := $(PWD)/driver
MEiDS_FW_DIR := $(PWD)/firmware
ifndef MEiDS_BIN_DIR
MEiDS_BIN_DIR := $(PWD)/bin
endif
MEiDS_INCLUDE_DIR := $(PWD)/../include

# UTS release
ifndef KERNEL_VER
# Old location
KERNEL_VER := $(shell if [ -f $(LINUX_SRC)/include/linux/utsrelease.h ]; then \
	awk -F\" '/REL/ {print $$2}' $(LINUX_SRC)/include/linux/utsrelease.h ; \
	fi;)
ifndef KERNEL_VER
# New location
KERNEL_VER := $(shell if [ -f $(LINUX_SRC)/include/generated/utsrelease.h ]; then \
	awk -F\" '/REL/ {print $$2}' $(LINUX_SRC)/include/generated/utsrelease.h ; \
	fi;)
endif
endif

ifdef KERNEL_VER
LINUX_FW_DIR := /lib/firmware/$(KERNEL_VER)/meids
else
LINUX_FW_DIR := /lib/firmware/meids
endif

LINUX_INIT_DIR := /etc/init.d

ifdef MEiDS_EXT
MODULE_DEST := meids/$(MEiDS_EXT)
else
MODULE_DEST := meids
endif

# Global section
default: modules

.PHONY: all
all: modules dev_build

.PHONY: modules
modules: PCI_modules USB_modules MEPHISTO_modules

.PHONY: PCI_modules
PCI_modules:
	@make -s MEiDS_EXT:=PCI MEiDS_INCLUDE_DIR:=$(MEiDS_INCLUDE_DIR) MEiDS_BIN_DIR:=$(MEiDS_BIN_DIR) \
	LINUX_SRC:=${LINUX_SRC} KERNEL_VER:=${KERNEL_VER} \
	meids_modules

.PHONY: USB_modules
USB_modules:
	@make -s MEiDS_EXT:=USB MEiDS_INCLUDE_DIR:=$(MEiDS_INCLUDE_DIR) MEiDS_BIN_DIR:=$(MEiDS_BIN_DIR) \
	LINUX_SRC:=${LINUX_SRC} KERNEL_VER:=${KERNEL_VER} \
	meids_modules

.PHONY: MEPHISTO_modules
MEPHISTO_modules:
	@make -s MEiDS_EXT:=MEPHISTO MEiDS_INCLUDE_DIR:=$(MEiDS_INCLUDE_DIR) MEiDS_BIN_DIR:=$(MEiDS_BIN_DIR) \
	LINUX_SRC:=${LINUX_SRC} KERNEL_VER:=${KERNEL_VER} \
	meids_modules

.PHONY: install
install: PCI_install USB_install MEPHISTO_install

.PHONY: PCI_install
PCI_install: test_user
	@make -s MEiDS_EXT:=PCI LINUX_SRC:=${LINUX_SRC} KERNEL_VER:=${KERNEL_VER} meids_install

.PHONY: USB_install
USB_install: test_user
	@make -s MEiDS_EXT:=USB LINUX_SRC:=${LINUX_SRC} KERNEL_VER:=${KERNEL_VER} meids_install

.PHONY: MEPHISTO_install
MEPHISTO_install: test_user
	@make -s MEiDS_EXT:=MEPHISTO LINUX_SRC:=${LINUX_SRC} KERNEL_VER:=${KERNEL_VER} meids_install

.PHONY: uninstall
uninstall: PCI_uninstall USB_uninstall MEPHISTO_uninstall
	@echo "NOTE: Firmwares are not removed automaticaly."

.PHONY: PCI_uninstall
PCI_uninstall: test_user
	@make -s MEiDS_EXT:=PCI LINUX_SRC:=${LINUX_SRC} KERNEL_VER:=${KERNEL_VER} meids_uninstall

.PHONY: USB_uninstall
USB_uninstall: test_user
	@make -s MEiDS_EXT:=USB LINUX_SRC:=${LINUX_SRC} KERNEL_VER:=${KERNEL_VER} meids_uninstall

.PHONY: MEPHISTO_uninstall
MEPHISTO_uninstall: test_user
	@make -s MEiDS_EXT:=MEPHISTO LINUX_SRC:=${LINUX_SRC} KERNEL_VER:=${KERNEL_VER} meids_uninstall

# Local section (can be used manualy)
.PHONY: meids_modules
meids_modules:
	@echo "Compileing $(MEiDS_EXT) drivers."
	@make -C $(MEiDS_SRC_DRV) MEiDS_EXT:=$(MEiDS_EXT) MEiDS_SRC:=$(MEiDS_SRC_DRV) MEiDS_INCLUDE_DIR:=$(MEiDS_INCLUDE_DIR) MEiDS_BIN_DIR:=$(MEiDS_BIN_DIR) \
	LINUX_SRC:=${LINUX_SRC} KERNEL_VER:=${KERNEL_VER} \
	modules

.PHONY: meids_install
meids_install: install_firmware
	@if test -f $(MEiDS_BIN_DIR)/memain$(MEiDS_EXT).ko ||  test -f $(MEiDS_BIN_DIR)/me$(MEiDS_EXT).ko; then \
		echo "Installing $(MEiDS_EXT) drivers."; \
		make -C $(MEiDS_SRC_DRV) MEiDS_EXT:=$(MEiDS_EXT) MEiDS_SRC:=$(MEiDS_SRC_DRV) MEiDS_BIN_DIR:=$(MEiDS_BIN_DIR) LINUX_SRC:=${LINUX_SRC} MODULE_DEST_EXT:=$(MODULE_DEST) KERNEL_VER:=${KERNEL_VER} install; \
		make MEiDS_EXT:=$(MEiDS_EXT) LINUX_SRC:=${LINUX_SRC} KERNEL_VER:=${KERNEL_VER} install_scripts; \
		make MEiDS_EXT:=$(MEiDS_EXT) LINUX_SRC:=${LINUX_SRC} KERNEL_VER:=${KERNEL_VER} start_driver; \
	else \
		echo "$(MEiDS_EXT) modules not found!"; \
	fi

.PHONY: meids_uninstall
meids_uninstall:
	@echo "Uninstalling $(MEiDS_EXT) drivers."
	@make MEiDS_EXT:=$(MEiDS_EXT) LINUX_SRC:=${LINUX_SRC} KERNEL_VER:=${KERNEL_VER} stop_driver;
	@make MEiDS_EXT:=$(MEiDS_EXT) LINUX_SRC:=${LINUX_SRC} KERNEL_VER:=${KERNEL_VER} uninstall_scripts;
	@make -C $(MEiDS_SRC_DRV) MEiDS_EXT:=$(MEiDS_EXT) MEiDS_SRC:=$(MEiDS_SRC_DRV) MEiDS_BIN_DIR:=$(MEiDS_BIN_DIR) LINUX_SRC:=${LINUX_SRC} MODULE_DEST_EXT:=$(MODULE_DEST) KERNEL_VER:=${KERNEL_VER} uninstall

.PHONY: test_user
test_user:
	@if test $(shell whoami) != root; then \
		echo "root privileges required!"; \
		false; \
	fi

.PHONY: install_firmware
install_firmware:
	@if test ! -d $(LINUX_FW_DIR); then \
		mkdir -p --mode=0755 $(LINUX_FW_DIR); \
		echo "Creating directory $(LINUX_FW_DIR)."; \
	fi
	@echo "Installing firmwares in $(LINUX_FW_DIR)."
	@install -m 0444 $(MEiDS_FW_DIR)/me*.bin -t ${LINUX_FW_DIR}

.PHONY: install_scripts
install_scripts:
	@echo "Installing $(MEiDS_EXT) starting scripts."
	@cp $(PWD)/rcmedriver$(MEiDS_EXT) $(LINUX_INIT_DIR)/medriver$(MEiDS_EXT)
#Let do insserv the runlevel setup
	@if test -f /usr/lib/lsb/install_initd; then \
		echo "Adding $(MEiDS_EXT) driver scripts to runlevel entries."; \
		/usr/lib/lsb/install_initd medriver$(MEiDS_EXT); \
	else \
		echo "install_initd not found!"; \
		echo "Add starting scripts to runlevels manualy."; \
	fi

.PHONY: uninstall_scripts
uninstall_scripts:
	@-if test -f /usr/lib/lsb/remove_initd; then \
		echo "Removing $(MEiDS_EXT) driver scripts from runlevel entries."; \
		/usr/lib/lsb/remove_initd medriver$(MEiDS_EXT); \
	else \
		echo "remove_initd not found!"; \
		echo "Remove starting scripts from runlevels manualy."; \
	fi
	@echo "Uninstalling $(MEiDS_EXT) starting scripts."
	@-rm -f $(LINUX_INIT_DIR)/medriver$(MEiDS_EXT)


.PHONY: start_driver
start_driver:
	@-if test -f $(LINUX_INIT_DIR)/medriver$(MEiDS_EXT); then \
		echo "Starting $(MEiDS_EXT) driver."; \
		$(LINUX_INIT_DIR)/medriver$(MEiDS_EXT) start; \
	fi

.PHONY: stop_driver
stop_driver:
	@-if test -f $(LINUX_INIT_DIR)/medriver$(MEiDS_EXT); then \
		echo "Stoping $(MEiDS_EXT) driver."; \
		$(LINUX_INIT_DIR)/medriver$(MEiDS_EXT) stop; \
	fi



# Other options
.PHONY: clean
clean:
	@make -C $(MEiDS_SRC_DRV) MEiDS_SRC:=$(MEiDS_SRC_DRV) LINUX_SRC:=${LINUX_SRC} KERNEL_VER:=${KERNEL_VER} clean

.PHONY: clear
clear:
	@make -C $(MEiDS_SRC_DRV) MEiDS_SRC:=$(MEiDS_SRC_DRV) LINUX_SRC:=${LINUX_SRC} KERNEL_VER:=${KERNEL_VER} clear
	@rm -f *~

.PHONY: delete
delete:
	@make -C $(MEiDS_SRC_DRV) MEiDS_BIN_DIR:=$(MEiDS_BIN_DIR) LINUX_SRC:=${LINUX_SRC} KERNEL_VER:=${KERNEL_VER} delete

.PHONY: help
help:
	@echo "  Building MEiDS modules."
	@echo "  Syntax: make [options] target."
	@echo "  Source directory: $(MEiDS_SRC_DRV)"
	@echo "  Firmware directory: $(MEiDS_FW_DIR)"
	@echo "  Destination directory: $(MEiDS_BIN_DIR)"
	@echo
	@echo "  options:"
	@echo "    LINUX_SRC=kernel source directory (default: /usr/src/linux)"
	@echo
	@echo "  targets:"
	@echo "    modules		- default target, build PCI, USB and MephistoScope modules"
	@echo "    PCI_modules		- build PCI modules"
	@echo "    USB_modules		- build USB modules"
	@echo "    MEPHISTO_modules	- build MephistoScope module"
	@echo
	@echo "    install		- install PCI and USB modules"
	@echo "    su_install		- install as root"
	@echo "    PCI_install		- install PCI modules"
	@echo "    USB_install		- install USB modules"
	@echo "    MEPHISTO_install	- install MephistoScope module"
	@echo
	@echo "    uninstall		- uninstall PCI and USB modules"
	@echo "    su_install		- uninstall as root"
	@echo "    PCI_uninstall	- uninstall PCI modules"
	@echo "    USB_uninstall	- uninstall USB modules"
	@echo "    MEPHISTO_uninstall	- uninstall MephistoScope module"
	@echo
	@echo "    clean		- remove temporary files"
	@echo "    clear		- remove temporary and backup files"
	@echo "    delete		- delete modules"
	@echo
	@echo "For more help see READ.ME file"
	@echo

# Super-user install/uninstall routines
.PHONY: su_install
su_install:
	@su -c 'make -s install'

.PHONY: su_uninstall
su_uninstall:
	@su -c 'make -s uninstall'
