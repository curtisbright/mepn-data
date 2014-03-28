# Makefile for Linux binaries. (C) Geoffrey Reynolds, July 2006.
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.

vpath %.c ..
vpath %.h ..
vpath %.S ..
vpath README ..

CFLAGS=-O2 -fomit-frame-pointer -ffast-math -march=$(ARCH) -m64
CPPFLAGS=-Wall -DNDEBUG $(ASM_FLAGS)
LDFLAGS=-s -m64
ASFLAGS=-m64
LDLIBS=-lm

CC=gcc

TAR=tar -c
GZIP=gzip -9
ZIP=zip -q -9
SED=sed
FMT=fmt

.PHONY: clean realclean

TARBALL=srsieve-$(FULL_VER)-linux64-$(ARCH).zip
PROGS=srsieve srfile
DOCS=BUILD README

$(TARBALL): $(DOCS) $(PROGS)
	$(ZIP) $@ $^

srsieve: $(SRSIEVE_OBJS) $(ASM_OBJS)
srfile: $(SRFILE_OBJS) $(ASM_OBJS)

BUILD: BUILD.in
	$(SED) -e s/VERSION/$(FULL_VER)/g \
	       -e s/FLAGS/"$(CPPFLAGS) $(CFLAGS)/g" $< | $(FMT) > $@

clean:
	rm -f *.o $(PROGS) BUILD

realclean: clean
	rm -f *.zip
