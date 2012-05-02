#
# Makefile for Meilhaus device driver library
#
# Version
VER_MAGIC = 00
VER_MAJOR = 01
VER_MINOR = 03
VER_BUILD = 37

LIB_VERSION=0x$(VER_MAGIC)$(VER_MAJOR)$(VER_MINOR)$(VER_BUILD)

# Build flags
CPPFLAGS = -I./ -I../include -I../common
CPPFLAGS += -D_REENTRANT -fPIC -Wmissing-prototypes -O2 -Wall
CPPFLAGS += -DMEIDS_VERSION_LIBRARY=$(LIB_VERSION)

# Debug flags
# CPPFLAGS += -DLIBMEDEBUG_DEVELOP
# CPPFLAGS += -DLIBMEDEBUG_TEST
# CPPFLAGS += -DLIBMEDEBUG_ERROR
# CPPFLAGS += -DLIBMEDEBUG_TIMESTAMPS

# Defines of targets
UNV_NAME   := meids_unv
LOCAL_NAME := meids_local
RPC_NAME   := meids_rpc
SIMPLE_NAME   := meids_simple

# Objects
LIB_OBJS := meids_global.o meids_pthread.o meids_vrt.o meids_utility.o

ifeq ($(LIB_NAME),$(UNV_NAME))
LIB_OBJS  += meids_internal.o
LIB_OBJS  += meids_xml.o meids_xml_init.o
LIB_OBJS  += meids_xml_unv.o meids_local_calls.o meids_rpc_calls.o
LIB_OBJS  += meids_config.o meids_local_config.o meids_rpc_config.o
LIB_OBJS  += rmedriver_clnt.o rmedriver_xdr.o
LIB_OBJS  += meids_rpc_RQuery.o
endif

ifeq ($(LIB_NAME),$(LOCAL_NAME))
LIB_OBJS += meids_local_internal.o
LIB_OBJS += meids_init.o
LIB_OBJS += meids_local.o meids_local_calls.o
LIB_OBJS += meids_local_config.o meids_config.o
LIB_OBJS += meids_local_RQuery.o
endif

ifeq ($(LIB_NAME),$(RPC_NAME))
LIB_OBJS += meids_internal.o
LIB_OBJS += meids_rpc.o meids_rpc_calls.o
LIB_OBJS += meids_config.o meids_rpc_config.o
LIB_OBJS += rmedriver_clnt.o rmedriver_xdr.o
LIB_OBJS += meids_rpc_RQuery.o
endif

ifeq ($(LIB_NAME),$(SIMPLE_NAME))
LIB_OBJS  += meids_internal.o
LIB_OBJS  += meids_unv.o meids_local_calls.o meids_rpc_calls.o
LIB_OBJS  += meids_config.o meids_local_config.o meids_rpc_config.o
LIB_OBJS  += rmedriver_clnt.o rmedriver_xdr.o
LIB_OBJS  += meids_rpc_RQuery.o
endif

LIB_FLAGS := -lpthread -lrt
ifeq ($(LIB_NAME),$(UNV_NAME))
CPPFLAGS  += -I/usr/include/libxml2
LIB_FLAGS += -lxml2
endif

# Remote server
SVC = rmedriver_svc
SVC_local = rmedriver_svc_local

# Enviroment
ifdef PWD
MEiDS_SRC := $(PWD)
else
MEiDS_SRC := $(shell pwd)
endif

ifndef MEiDS_BIN_DIR
MEiDS_BIN_DIR := $(MEiDS_SRC)/lib
endif
ifeq ($(shell uname -p), x86_64)
LINUX_LIB_DIR := /usr/lib64
else
LINUX_LIB_DIR := /usr/lib
endif
MEiDS_LIB_DIR := $(LINUX_LIB_DIR)/MEiDS

MEiDS_INC_DIR := /usr/include/medriver
SRC_INC_DIR := $(MEiDS_SRC)/../osi

LINUX_NAME := medriver
WINDOWS_NAME := meIDSmain
DLL_NAME := MEiDS

SRC_CONF_DIR := $(MEiDS_SRC)/../config
MEiDS_CONF_DIR := /etc/medriver

all: local rpc simple unv
default: unv

.PHONY: unv
unv:
	@echo "Building standard MEiDS library."
	@make LIB_NAME:=$(UNV_NAME) build_meids

.PHONY: local
local:
	@echo "Building local MEiDS library."
	@make LIB_NAME:=$(LOCAL_NAME) build_meids

.PHONY: rpc
rpc:
	@echo "Building remote MEiDS library."
	@make LIB_NAME:=$(RPC_NAME) build_meids

.PHONY: simple
simple:
	@echo "Building simple MEiDS library."
	@make LIB_NAME:=$(SIMPLE_NAME) build_meids

.PHONY: build_meids
build_meids: $(LIB_OBJS)
	@gcc -fPIC -shared -Wl,-soname,lib$(LIB_NAME).so -o lib$(LIB_NAME).so.$(VER_MAJOR).$(VER_MINOR).$(VER_BUILD) $(LIB_OBJS) $(LIB_FLAGS)
	@if test ! -d $(MEiDS_BIN_DIR); then mkdir -p --mode=0777 $(MEiDS_BIN_DIR); fi
	@ln -f -s lib$(LIB_NAME).so.$(VER_MAJOR).$(VER_MINOR).$(VER_BUILD) lib$(LIB_NAME).so
	@cp -f -d lib$(LIB_NAME).so* $(MEiDS_BIN_DIR)
	@rm lib$(LIB_NAME).so*

.PHONY: svc
svc: rmedriver_main.o rmedriver_proc.o rmedriver_xdr.o
	@echo "Building MEiDS remote access server (standard)."
	@gcc $(CPPFLAGS) rmedriver_main.o rmedriver_proc.o rmedriver_xdr.o -o $(SVC) -L$(PWD) -L$(PWD)/lib -l$(UNV_NAME)

.PHONY: svc_local
svc_local: rmedriver_main.o rmedriver_proc.o rmedriver_xdr.o
	@echo "Building MEiDS remote access server (local)."
	@gcc $(CPPFLAGS) rmedriver_main.o rmedriver_proc.o rmedriver_xdr.o -o $(SVC_local) -L$(PWD) -L$(PWD)/lib -l$(LOCAL_NAME)



.PHONY: install
install: install_unv link_unv install_headers install_config

.PHONY: install_unv
install_unv:
	@echo "Installing standard MEiDS library."
	@make LIB_NAME:=$(UNV_NAME) install_lib

.PHONY: install_local
install_local:
	@echo "Installing local MEiDS library."
	@make LIB_NAME:=$(LOCAL_NAME) install_lib

.PHONY: install_rpc
install_rpc:
	@echo "Installing remote MEiDS library."
	@make LIB_NAME:=$(RPC_NAME) install_lib

.PHONY: install_simple
install_simple:
	@echo "Installing simple MEiDS library."
	@make LIB_NAME:=$(SIMPLE_NAME) install_lib

.PHONY: uninstall
uninstall: unlink uninstall_headers uninstall_config
	@echo "Uninstalling all MEiDS library."
	@-if test -d $(MEiDS_LIB_DIR); then \
		echo "UNINSTALLING LIBRARY $(MEiDS_LIB_DIR)/lib*.so"; \
		rm -f $(MEiDS_LIB_DIR)/lib*.so; \
		rm -f $(MEiDS_LIB_DIR)/lib*.so.??; \
		rm -f $(MEiDS_LIB_DIR)/lib*.so.??.??; \
		rm -f $(MEiDS_LIB_DIR)/lib*.so.??.??.??; \
	else \
		echo "UNINSTALLING LIBRARY NOT POSSIBLE"; \
	fi
	@-rm -f $(LINUX_LIB_DIR)/libmeids_*.so
	@/sbin/ldconfig -n $(LINUX_LIB_DIR)

.PHONY: unlink
unlink:
	@rm -f $(LINUX_LIB_DIR)/lib${LINUX_NAME}.so
	@rm -f $(LINUX_LIB_DIR)/lib${WINDOWS_NAME}.so
	@rm -f $(LINUX_LIB_DIR)/lib${DLL_NAME}.so
	@/sbin/ldconfig -n $(LINUX_LIB_DIR)

.PHONY: link_unv
link_unv:
	@make LIB_NAME=$(UNV_NAME) link_lib

.PHONY: link_local
link_local:
	@make LIB_NAME=$(LOCAL_NAME) link_lib

.PHONY: link_rpc
link_rpc:
	@make LIB_NAME=$(RPC_NAME) link_lib

.PHONY: link_simple
link_simple:
	@make LIB_NAME=$(SIMPLE_NAME) link_lib

.PHONY: install_lib
install_lib: test_lib
	@if test ! -d $(MEiDS_LIB_DIR); then mkdir -p --mode=0755 $(MEiDS_LIB_DIR); fi
	@cp -f -d $(MEiDS_BIN_DIR)/lib$(LIB_NAME).so* $(MEiDS_LIB_DIR)
	@strip $(MEiDS_LIB_DIR)/*
	@/sbin/ldconfig -n $(MEiDS_LIB_DIR)
	@ln -f -s $(MEiDS_LIB_DIR)/lib$(LIB_NAME).so.$(VER_MAJOR).$(VER_MINOR).$(VER_BUILD) $(LINUX_LIB_DIR)/lib$(LIB_NAME).so
	@/sbin/ldconfig -n $(LINUX_LIB_DIR)

.PHONY: link_lib
link_lib:
	@if ! test -f $(MEiDS_LIB_DIR)/lib$(LIB_NAME).so.$(VER_MAJOR).$(VER_MINOR).$(VER_BUILD); then \
		echo "ERROR: Library ${LIB_NAME}.so.$(VER_MAJOR).$(VER_MINOR).$(VER_BUILD) not in place. Please build and install it!"; \
		false; \
	fi
	@ln -f -s $(MEiDS_LIB_DIR)/lib$(LIB_NAME).so.$(VER_MAJOR).$(VER_MINOR).$(VER_BUILD) $(LINUX_LIB_DIR)/lib${LINUX_NAME}.so
	@ln -f -s $(MEiDS_LIB_DIR)/lib$(LIB_NAME).so.$(VER_MAJOR).$(VER_MINOR).$(VER_BUILD) $(LINUX_LIB_DIR)/lib${WINDOWS_NAME}.so
	@ln -f -s $(MEiDS_LIB_DIR)/lib$(LIB_NAME).so.$(VER_MAJOR).$(VER_MINOR).$(VER_BUILD) $(LINUX_LIB_DIR)/lib${DLL_NAME}.so
	@/sbin/ldconfig -n $(LINUX_LIB_DIR)

.PHONY: test_lib
test_lib:
ifndef LIB_NAME
	@echo "No library's version defined!"
	@false
endif
	@if ! test -d $(MEiDS_BIN_DIR); then \
		echo "ERROR: Destination directory not found. Please build library first!"; \
		false; \
	fi
	@if ! test -f $(MEiDS_BIN_DIR)/lib$(LIB_NAME).so; then \
		echo "ERROR: Library ${LIB_NAME} not in place. Please rebuild it!"; \
		false; \
	fi

.PHONY: install_headers
install_headers: ;
	@echo "Installing headers."
	@if test ! -d $(MEiDS_INC_DIR); then mkdir -p --mode=0755 $(MEiDS_INC_DIR); fi
	@install -m 0666 $(SRC_INC_DIR)/*.h ${MEiDS_INC_DIR}

.PHONY: uninstall_headers
uninstall_headers: ;
	@echo "Removing headers."
	@-if test -d $(MEiDS_INC_DIR); then rm -f -r $(MEiDS_INC_DIR); fi

.PHONY: install_config
install_config:
	@echo "Installing configuration files."
	@if test ! -d $(MEiDS_CONF_DIR); then mkdir -p --mode=0755 $(MEiDS_CONF_DIR); fi
	@install -m 0666 $(SRC_CONF_DIR)/meconfig.xml $(SRC_CONF_DIR)/medrvconfig.dtd $(SRC_CONF_DIR)/meids.config ${MEiDS_CONF_DIR}

.PHONY: uninstall_config
uninstall_config:
	@echo "Removing configuration files."
	@-if test -d $(MEiDS_CONF_DIR); then rm -f -r $(MEiDS_CONF_DIR); fi

.PHONY: install_svc
install_svc:
	@echo "Installing remote access server."
	@SERVER=$(SVC) $(PWD)/MEinstallSVC.sh

.PHONY: uninstall_svc
uninstall_svc:
	@echo "Uninstalling remote access server."
	@SERVER=$(SVC) $(PWD)/MEuninstallSVC.sh

.PHONY: clean
clean:
	@rm -f core *.swp *.o *.so* *.so $(SVC) $(SVC_local)

.PHONY: clear
clear:
	@rm -f core *.swp *.o *~

.PHONY: delete
delete:
	@rm -f $(MEiDS_BIN_DIR)/*.so $(MEiDS_BIN_DIR)/*.so*

.PHONY: clear_all
clear_all: clear delete
	@rm -f tags *.so* *.so

# Local (PCI and USB)
meids_local.o: meids_debug.h meids_local_internal.o meids_config.o meids_local_config.o meids_local_calls.o meids_vrt.o meids_local.c
	@gcc $(CPPFLAGS) -c meids_local.c

# External (RPC)
meids_rpc.o: meids_debug.h meids_internal.o meids_config.o meids_local_config.o meids_rpc_calls.o meids_vrt.o meids_rpc.c
	@gcc $(CPPFLAGS) -c meids_rpc.c

# General (Local and External)
meids_unv.o: meids_debug.h meids_internal.o meids_config.o meids_local_config.o meids_rpc_config.o meids_local_calls.o meids_rpc_calls.o meids_vrt.o meids_unv.c
	@gcc $(CPPFLAGS) -c meids_unv.c

# General (Local and External + XML)
meids_xml_unv.o: meids_debug.h meids_internal.o meids_config.o meids_local_config.o meids_rpc_config.o meids_local_calls.o meids_rpc_calls.o meids_vrt.o meids_xml.o meids_xml_unv.c
	@gcc $(CPPFLAGS) -c meids_xml_unv.c

# Common API interface
meids_global.o: meids_debug.h meids_internal.o meids_pthread.o meids_config.o meids_global.c
	@gcc $(CPPFLAGS) -c meids_global.c

meids_pthread.o: meids_pthread.c
	@gcc $(CPPFLAGS) -c meids_pthread.c

# Common objects
meids_internal.o: meids_internal.c
	@gcc $(CPPFLAGS) -c meids_internal.c

meids_local_internal.o: meids_internal.c
	@gcc $(CPPFLAGS) -DNO_RPC_ALLOWED -c meids_internal.c -o meids_local_internal.o

meids_config.o: meids_debug.h meids_internal.o meids_config.c
	@gcc $(CPPFLAGS) -c meids_config.c

meids_local_config.o: meids_debug.h meids_internal.o meids_config.o meids_local_calls.o meids_local_config.c
	@gcc $(CPPFLAGS) -c meids_local_config.c

meids_rpc_config.o: meids_debug.h meids_internal.o meids_config.o meids_rpc_calls.o meids_rpc_config.c
	@gcc $(CPPFLAGS) -c meids_rpc_config.c

meids_local_calls.o: meids_debug.h meids_internal.o meids_local_calls.c
	@gcc $(CPPFLAGS) -c meids_local_calls.c

meids_rpc_calls.o: meids_debug.h meids_internal.o rmedriver.h rmedriver_clnt.o meids_rpc_calls.c
	@gcc $(CPPFLAGS) -c meids_rpc_calls.c

meids_vrt.o: meids_debug.h meids_config.o meids_vrt.c
	@gcc $(CPPFLAGS) -c meids_vrt.c

meids_init.o: meids_debug.h meids_config.o meids_init.c
	@gcc $(CPPFLAGS) -c meids_init.c

# Utility functions
meids_utility.o: meids_global.o meids_utility.c
	@gcc $(CPPFLAGS) -c meids_utility.c

# Remote client
rmedriver_clnt.o: rmedriver.h rmedriver_clnt.c rmedriver_xdr.o
	@gcc $(CPPFLAGS) -c rmedriver_clnt.c

# Remote server
rmedriver_xdr.o: rmedriver_xdr.c
	@gcc $(CPPFLAGS) -c rmedriver_xdr.c


rmedriver_proc.o: rmedriver_proc.c
	@gcc $(CPPFLAGS) -c rmedriver_proc.c

rmedriver_main.o: meids_debug.h rmedriver.h rmedriver_main.c rmedriver_xdr.o
	@gcc $(CPPFLAGS) -c rmedriver_main.c

# XML
meids_xml.o: meids_debug.h meids_internal.o  meids_config.o meids_xml.c
	@gcc $(CPPFLAGS) -c meids_xml.c

meids_xml_init.o: meids_debug.h meids_config.o meids_xml.o meids_xml_init.c
	@gcc $(CPPFLAGS) -c meids_xml_init.c

# MEiDC support - Remote queries.
# Dummy for local builds
meids_local_RQuery.o: meids_local_RQuery.c
	@gcc $(CPPFLAGS) -c meids_local_RQuery.c

meids_rpc_RQuery.o: meids_debug.h rmedriver.h meids_rpc_RQuery.c
	@gcc $(CPPFLAGS) -c meids_rpc_RQuery.c

.PHONY: help
help:
	@echo "  Building MEiDS library."
	@echo "  Syntax: make target."
	@echo
	@echo "  targets:"
	@echo "    unv			- default target, build universal (full) version of library"
	@echo "    local		- build reduced version of library - only local devices, no RPC (SynapseLAN)"
	@echo "    rpc			- build reduced version of library - only RPC (SynapseLAN), no local devices"
	@echo
	@echo "    install		- install default library and link it to default name (libMEiDS.so)"
	@echo "    su_install		- install default library as the superuser"
	@echo "    uninstall		- uninstall library"
	@echo "    su_uninstall	- uninstall library as the superuser"
	@echo
	@echo "     install_unv	- install universal library"
	@echo "     link_unv		- link universal library to default name (libMEiDS.so)"
	@echo "     install_local	- install local library"
	@echo "     link_unv		- link universal library to default name (libMEiDS.so)"
	@echo "     install_rpc	- install rpc library"
	@echo "     link_rpc		- link rpc library to default name (libMEiDS.so)"
	@echo
	@echo "    svc			- build RPC server for remote station (SynapseLAN)"
	@echo "    install_svc		- install RPC server"
	@echo "    uninstall_svc	- uninstall RPC server"
	@echo "    su_install_svc	- install RPC server as the superuser"
	@echo "    su_uninstall_svc	- uninstall RPC server as the superuser"
	@echo
	@echo "    clean		- remove temporary files"
	@echo "    clear		- remove old builds, temporary and backup files"
	@echo
	@echo "Library install directory: $(MEiDS_LIB_DIR)"
	@echo

# Super-user install/uninstall routines
.PHONY: su_install
su_install:
	@su -c 'make -s install'

.PHONY: su_uninstall
su_uninstall:
	@su -c 'make -s uninstall'

.PHONY: su_install_svc
su_install_svc:
	@su -c 'make -s install_svc'

.PHONY: su_uninstall_svc
su_uninstall_svc:
	@su -c 'make -s uninstall_svc'
