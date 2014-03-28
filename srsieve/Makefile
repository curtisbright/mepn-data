# Makefile for srsieve. (C) Geoffrey Reynolds, April 2006.
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.

export MAJOR_VER=0
export MINOR_VER=6
export PATCH_VER=17

# Set DEBUG=yes to compile with debugging information and internal checks.
#
DEBUG=no

# Set ASSEMBLER=yes to use assembler code where available.
#
ASSEMBLER=yes

# Uncomment one of the following ARCH= lines:
#
#ARCH=ppc64-osx
#ARCH=ppc64-linux
#ARCH=i586
#ARCH=i686
#ARCH=athlon
#ARCH=pentium4
#ARCH=k8
#ARCH=k8-gcc430

# If ARCH not set above then these defauts will be used:
#
ifeq ($(ARCH),)
DEBUG_CFLAGS=-g
OPT_CFLAGS=-O2
CPPFLAGS=-Wall
ASSEMBLER=no
endif

ifeq ($(ARCH),ppc64-osx)
DEBUG_CFLAGS=-g -O3 -ffast-math -mdynamic-no-pic -mtune=G5 -mcpu=970 -m64
OPT_CFLAGS=-g -O3 -ffast-math -mdynamic-no-pic -mtune=G5 -mcpu=970 -m64 -fomit-frame-pointer
CPPFLAGS=-Wall -DNEED_UNDERSCORE -DUSE_INLINE_MULMOD=1
ASFLAGS=-mdynamic-no-pic -mtune=G5 -mcpu=970 -m64
LDFLAGS=-m64
endif

ifeq ($(ARCH),ppc64-linux)
DEBUG_CFLAGS=-g -O3 -ffast-math -mtune=G5 -mcpu=970 -m64 -maltivec -mabi=altivec
OPT_CFLAGS=-O3 -ffast-math -mtune=G5 -mcpu=970 -m64 -fomit-frame-pointer -maltivec -mabi=altivec
CPPFLAGS=-Wall -Xassembler -mregnames -DUSE_INLINE_MULMOD
ASFLAGS=-mregnames -mtune=G5 -mcpu=970 -m64 -maltivec -mabi=altivec
LDFLAGS=-m64
endif

ifeq ($(ARCH),i586)
DEBUG_CFLAGS=-g -march=i586
OPT_CFLAGS=-O2 -fomit-frame-pointer -ffast-math -march=i586
CPPFLAGS=-Wall
endif

ifeq ($(ARCH),i686)
DEBUG_CFLAGS=-g -march=i686
OPT_CFLAGS=-O2 -fomit-frame-pointer -ffast-math -march=i686
CPPFLAGS=-Wall -DHAVE_CMOV
endif

ifeq ($(ARCH),athlon)
DEBUG_CFLAGS=-g -march=athlon
OPT_CFLAGS=-O2 -fomit-frame-pointer -ffast-math -march=athlon
CPPFLAGS=-Wall -DHAVE_CMOV
endif

ifeq ($(ARCH),pentium4)
DEBUG_CFLAGS=-g -march=pentium4
OPT_CFLAGS=-O2 -fomit-frame-pointer -ffast-math -march=pentium4
CPPFLAGS=-Wall -DHAVE_CMOV
endif

ifeq ($(ARCH),k8)
DEBUG_CFLAGS=-g -march=k8 -m64
OPT_CFLAGS=-O2 -fomit-frame-pointer -ffast-math -march=k8 -m64
#CPPFLAGS=-Wall -DHAVE_CMOV -DUSE_FPU_MULMOD=1
CPPFLAGS=-Wall -DHAVE_CMOV
ASFLAGS=-m64
LDFLAGS=-m64
endif

# GCC 4.3.0 produduces bad code when -O2 or higher is used.
#
ifeq ($(ARCH),k8-gcc430)
DEBUG_CFLAGS=-g -march=k8 -m64
OPT_CFLAGS=-O -fomit-frame-pointer -ffast-math -march=k8 -m64
#CPPFLAGS=-Wall -DHAVE_CMOV -DUSE_FPU_MULMOD=1
CPPFLAGS=-Wall -DHAVE_CMOV
ASFLAGS=-m64
LDFLAGS=-m64
endif


ifeq ($(ASSEMBLER),yes)
CPPFLAGS+= -DUSE_ASM
ifeq ($(ARCH),ppc64-osx)
ASM_OBJS=mulmod-ppc64.o expmod-ppc64.o
endif
ifeq ($(ARCH),ppc64-linux)
ASM_OBJS=mulmod-ppc64.o expmod-ppc64.o
endif
ifeq ($(ARCH),i586)
ASM_OBJS=powmod-i386.o
endif
ifeq ($(ARCH),i686)
ASM_OBJS=powmod-i386.o
endif
ifeq ($(ARCH),athlon)
ASM_OBJS=powmod-i386.o
endif
ifeq ($(ARCH),pentium4)
ASM_OBJS=powmod-i386.o
endif
ifeq ($(ARCH),k8)
ASM_OBJS=powmod-k8.o
endif
ifeq ($(ARCH),k8-gcc430)
ASM_OBJS=powmod-k8.o
endif
endif

# Append any user-supplied CFLAGS.
ifeq ($(DEBUG),yes)
override CFLAGS:=$(DEBUG_CFLAGS) $(CFLAGS)
else
CPPFLAGS+= -DNDEBUG
override CFLAGS:=$(OPT_CFLAGS) $(CFLAGS)
LDFLAGS+= -s
endif

# ceil(x) is broken in GCC 4.1.0.
#CC=gcc -V4.1 -fno-builtin-ceil
CC=gcc
LDLIBS=-lm

TAR=tar -c
GZIP=gzip -9
ZIP=zip -q -9
MD5SUM=md5sum
SED=sed
FMT=fmt
CMP=cmp

# No changes should be needed below here.

.PHONY: all check dist clean realclean test0 test1 test2 test3 test4

export FULL_VER=$(MAJOR_VER).$(MINOR_VER).$(PATCH_VER)

PROGS=srsieve srfile

export SRSIEVE_OBJS=srsieve.o arithmetic32.o arithmetic64.o bitmap.o bsgs.o \
	clock.o events.o factors.o files.o global.o hashtable.o \
	primes.o sequences.o subseq.o util.o priority.o $(ASM_OBJS)
export SRFILE_OBJS=srfile.o arithmetic32.o arithmetic64.o files.o global.o \
	util.o $(ASM_OBJS)

SRTEST_OBJS=srtest.o arithmetic64.o $(ASM_OBJS)
SRTEST_LIBS=-lgmp

SOURCES=Makefile arithmetic.h arithmetic32.c arithmetic64.c bitmap.h bitmap.c \
	bsgs.c choose.c clock.c config.h events.c factors.c files.c filter.c \
	global.c hashtable.h hashtable.c memset_fast32.h primes.c sequences.c \
	srfile.c srsieve.h srsieve.c subseq.c util.c srtest.c priority.c \
	powmod-i386.S powmod-k8.S asm-i386-gcc.h asm-x86-64-gcc.h asm-ppc64.h \
	expmod-ppc64.S mulmod-ppc64.S README.txt.in \
	mingw32/BUILD.txt.in mingw32/Makefile \
	linux32/BUILD.in linux32/Makefile linux64/BUILD.in linux64/Makefile

DOCS=CHANGES COPYING INSTALL README TODO WARNING.txt BUG64.txt

TESTS=test1.in test1.good test2.in test2.good test3.good test4.good

TARBALL=srsieve-$(FULL_VER)-src.zip

# These mingw32 targets are only for cross-compiling from unix.
MINGW32I586=srsieve-$(FULL_VER)-mingw32-i586.zip
MINGW32I686=srsieve-$(FULL_VER)-mingw32-i686.zip
MINGW32=$(MINGW32I586) $(MINGW32I686)

LINUX32I586=srsieve-$(FULL_VER)-linux32-i586.zip
LINUX32I686=srsieve-$(FULL_VER)-linux32-i686.zip
LINUX32=$(LINUX32I586) $(LINUX32I686)
LINUX64K8=srsieve-$(FULL_VER)-linux64-k8.zip
LINUX64=$(LINUX64K8)

DISTFILES=$(TARBALL) $(MINGW32) $(LINUX32) $(LINUX64) MD5SUM.txt README.txt

all: $(PROGS)

check: test0 test1 test2 test3 test4
	@echo "All tests passed."

dist: $(DISTFILES)

$(TARBALL): $(DOCS) $(SOURCES) $(TESTS)
	$(ZIP) $@ $^

MD5SUM.txt: $(TARBALL) $(MINGW32) $(LINUX32) $(LINUX64)
	$(MD5SUM) $^ > $@

README.txt: README.txt.in
	$(SED) -e s/VERSION/$(FULL_VER)/g $< | $(FMT) > $@

srsieve: $(SRSIEVE_OBJS)

srfile: $(SRFILE_OBJS)

srtest: $(SRTEST_OBJS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^ $(SRTEST_LIBS)

$(MINGW32I586): version.h
	$(MAKE) -C mingw32 clean
	$(MAKE) -C mingw32 ARCH=i586 ASM_FLAGS="-DUSE_ASM" ASM_OBJS=powmod-i386.o $@
	cp -a mingw32/$@ $@

$(MINGW32I686): version.h
	$(MAKE) -C mingw32 clean
	$(MAKE) -C mingw32 ARCH=i686 ASM_FLAGS="-DUSE_ASM -DHAVE_CMOV" ASM_OBJS=powmod-i386.o $@
	cp -a mingw32/$@ $@

$(LINUX32I586): version.h
	$(MAKE) -C linux32 clean
	$(MAKE) -C linux32 ARCH=i586 ASM_FLAGS="-DUSE_ASM" ASM_OBJS=powmod-i386.o $@
	cp -a linux32/$@ $@

$(LINUX32I686): version.h
	$(MAKE) -C linux32 clean
	$(MAKE) -C linux32 ARCH=i686 ASM_FLAGS="-DUSE_ASM -DHAVE_CMOV" ASM_OBJS=powmod-i386.o $@
	cp -a linux32/$@ $@

$(LINUX64K8): version.h
	$(MAKE) -C linux64 clean
	$(MAKE) -C linux64 ARCH=k8 ASM_FLAGS="-DUSE_ASM" ASM_OBJS=powmod-k8.o $@
	cp -a linux64/$@ $@

test0:	srtest
	./srtest 1000000 0 123456789

test1:	srsieve test1.in test1.good
	@echo "Testing 32 bit sieve, this takes 10 seconds on a P3/450."
	rm -f srsieve.out
	./srsieve --check --quiet --nmax 100e3 --pmax 5e6 test1.in
	$(CMP) srsieve.out test1.good
	@echo "Test 1 passed."

test2:	srsieve test2.in test2.good
	@echo "Testing 64 bit sieve, this takes 15 seconds on a P3/450."
	rm -f srfactors.txt
	./srsieve --check --quiet --factors --pmax 27437e6 test2.in
	$(CMP) srfactors.txt test2.good
	@echo "Test 2 passed."

test3:	srsieve test3.good
	@echo "Testing 32/64 bit transition, this takes 47 seconds on a P3/450."
	rm -f srfactors.txt
	./srsieve -cqf -n 100e3 -N 1000e3 -p 4293e6 -P 4297e6 "164852*5^n-1"
	$(CMP) srfactors.txt test3.good
	@echo "Test 3 passed."

test4:	srsieve test4.good
	rm -f sr_35.abcd
	@echo "Testing 32 bit sieve with filtering, this takes 32 seconds on a P3/450."
	./srsieve -cqa -n 1000e3 -N 1100e3 -P 10e6 "4*35^n+1" "26*35^n+1"
	$(CMP) sr_35.abcd test4.good
	@echo "Test 4 passed."

version.h:
	echo "#define MAJOR_VER $(MAJOR_VER)" > version.h
	echo "#define MINOR_VER $(MINOR_VER)" >> version.h
	echo "#define PATCH_VER $(PATCH_VER)" >> version.h

ASM_H=asm-i386-gcc.h asm-ppc64.h asm-x86-64-gcc.h
arithmetic32.o: arithmetic32.c arithmetic.h $(ASM_H) config.h
arithmetic64.o: arithmetic64.c arithmetic.h $(ASM_H) config.h
bitmap.o: bitmap.c srsieve.h config.h bitmap.h memset_fast32.h $(ASM_H)
bsgs.o: bsgs.c filter.c srsieve.h config.h arithmetic.h $(ASM_H) hashtable.h memset_fast32.h
clock.o: clock.c srsieve.h config.h
events.o: events.c srsieve.h config.h
factors.o: factors.c srsieve.h config.h arithmetic.h $(ASM_H)
files.o: files.c srsieve.h config.h
global.o: global.c srsieve.h config.h
hashtable.o: hashtable.c srsieve.h config.h hashtable.h memset_fast32.h $(ASM_H)
primes.o: primes.c srsieve.h bitmap.h config.h memset_fast32.h $(ASM_H)
sequences.o: sequences.c srsieve.h config.h bitmap.h arithmetic.h $(ASM_H)
srfile.o: srfile.c choose.c srsieve.h config.h version.h
srsieve.o: srsieve.c srsieve.h config.h version.h
srtest.o: srtest.c srsieve.h config.h version.h arithmetic.h $(ASM_H)
subseq.o: subseq.c choose.c srsieve.h config.h bitmap.h arithmetic.h $(ASM_H)
util.o: util.c srsieve.h config.h
priority.o: priority.c srsieve.h config.h
powmod-i386.o: powmod-i386.S config.h
powmod-k8.o: powmod-k8.S config.h
expmod-ppc64.o: expmod-ppc64.S config.h
mulmod-ppc64.o: mulmod-ppc64.S config.h

clean:
	rm -f *.o srsieve.out srsieve.log srfactors.txt *.abcd srtest
	$(MAKE) -C mingw32 clean
	$(MAKE) -C linux32 clean
	$(MAKE) -C linux64 clean

realclean: clean
	rm -f $(PROGS) version.h $(TARBALL) $(MINGW32) $(LINUX32) $(LINUX64)
	rm -f README.txt MD5SUM.txt
	$(MAKE) -C mingw32 realclean
	$(MAKE) -C linux32 realclean
	$(MAKE) -C linux64 realclean
