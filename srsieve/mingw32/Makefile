# Makefile for cross-compiling to mingw32. (C) Geoffrey Reynolds, June 2006.
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.

vpath %.c ..
vpath %.h ..
vpath %.S ..
vpath README ..

CFLAGS=-O2 -fomit-frame-pointer -ffast-math -march=$(ARCH)
CPPFLAGS=-Wall -DNDEBUG $(ASM_FLAGS)
LDFLAGS=-s
LDLIBS=-lm

CC=i586-mingw32msvc-gcc
AS=i586-mingw32msvc-as

ZIP=zip -q -9
SED=sed
FMT=fmt

.PHONY: clean realclean

ZIPFILE=srsieve-$(FULL_VER)-mingw32-$(ARCH).zip
EXE=srsieve.exe srfile.exe
TXT=BUILD.txt README.txt

%.exe %.txt: %
	cp $< $@

srsieve: $(SRSIEVE_OBJS) $(ASM_OBJS)
srfile: $(SRFILE_OBJS) $(ASM_OBJS)

BUILD.txt: BUILD.txt.in
	$(SED) -e s/VERSION/$(FULL_VER)/g \
	       -e s/FLAGS/"$(CPPFLAGS) $(CFLAGS)/g" $< | $(FMT) > $@

$(ZIPFILE): $(TXT) $(EXE)
	$(ZIP) -l $@ $(TXT)
	$(ZIP) $@ $(EXE)

clean:
	rm -f *.o $(TXT) srsieve srfile $(EXE)

realclean: clean
	rm -f *.zip
