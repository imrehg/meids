#
# Makefile for Meilhaus MEiDS. Java interface.
#
LIB_MAGIC = 00
LIB_MAJOR = 01
LIB_MINOR = 00
LIB_MINOR_BUILD = 00

LIB_VERSION=0x${LIB_MAGIC}${LIB_MAJOR}${LIB_MINOR}${LIB_MINOR_BUILD}

NAME = meids_java
SO_NAME = lib${LIBMEUNV_NAME}.so
REAL_NAME = ${LIBMEUNV_SO_NAME}.${LIB_MAJOR}.${LIB_MINOR}.${LIB_MINOR_BUILD}
L2_NAME = ${LIBMEUNV_SO_NAME}.${LIB_MAJOR}.${LIB_MINOR}
L1_NAME = ${LIBMEUNV_SO_NAME}.${LIB_MAJOR}

CPPFLAGS = -fPIC
CPPFLAGS += -Wmissing-prototypes
CPPFLAGS += -g3 -O0
# CPPFLAGS += -O2
CPPFLAGS += -Wall

# CPPFLAGS += -v

CPPFLAGS += -DJMEIDS_VERSION_LIBRARY=${LIB_VERSION}

default: libmedriver_java

.PHONY: libmedriver_java
libmedriver_java: meids_java.o
	gcc -shared -Wl,-soname,${SO_NAME} -o ${REAL_NAME} meids_java.o -lMEiDS

meids_java.o: meids_java.c
	gcc $(CPPFLAGS) -c meids_java.c -o meids_java.o

.PHONY: install
install:
	@LIBRARY=${REAL_NAME} $(PWD)/setup_lib.sh

.PHONY: uninstall
uninstall:
	@LIBRARY=${REAL_NAME} $(PWD)/clobber_lib.sh

.PHONY: clean
clean:
	rm -f *~ core *.swp tags *.o
