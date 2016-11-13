CFLAGS?=-Og
CPPFLAGS?=
LDFLAGS?=
LIBS?=

DESTDIR?=
PREFIX?=/usr/local
EXEC_PREFIX?=$(PREFIX)
INCLUDEDIR?=$(PREFIX)/include
LIBDIR?=$(EXEC_PREFIX)/lib

CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra -Wpedantic -Werror -Wno-sign-compare -Wno-unused -Wno-type-limits -fmax-errors=1
CPPFLAGS:=$(CPPFLAGS) -Iinclude
LIBK_CFLAGS:=$(CFLAGS)
LIBK_CPPFLAGS:=$(CPPFLAGS) -D__is_libk

CFLAGS:=$(CFLAGS)
CPPFLAGS:=$(CPPFLAGS)
LIBK_CFLAGS:=$(LIBK_CFLAGS)
LIBK_CPPFLAGS:=$(LIBK_CPPFLAGS)

OBJS=\
stdlib.o \
stdio.o \
string.o \
ctype.o \
errno.o \
math.o \

LIBK_OBJS:=$(OBJS:.o=.libk.o)

#BINARIES=libc.a libk.a # Not ready for libc yet.
BINARIES:=libk.a

.PHONY: all clean install install-headers install-libs
.SUFFIXES: .o .libk.o .c .S

all: $(BINARIES)

libc.a: $(OBJS)
	@$(AR) rcs $@ $(OBJS)

libk.a: $(LIBK_OBJS)
	@$(AR) rcs $@ $(LIBK_OBJS)

.c.o:
	$(CC) -MD -c $< -o $@ -std=gnu99 $(CFLAGS) $(CPPFLAGS)

.c.S:
	$(CC) -MD -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

.c.libk.o:
	$(CC) -MD -c $< -o $@ -std=gnu99 $(LIBK_CFLAGS) $(LIBK_CPPFLAGS)

.S.libk.o:
	$(CC) -MD -c $< -o $@ $(LIBK_CFLAGS) $(LIBK_CPPFLAGS)

clean:
	@rm -f $(BINARIES) *.a
	@rm -f $(OBJS) $(LIBK_OBJS) *.o */*.o */*/*.o
	@rm -f $(OBJS:.o=.d) $(LIBK_OBJS:.o=.d) *.d */*.d */*/*.d

install: install-headers install-libs

install-headers:
	@mkdir -p $(DESTDIR)$(INCLUDEDIR)
	@rsync -quraE ./include/. $(DESTDIR)$(INCLUDEDIR)/.

install-libs: $(BINARIES)
	@mkdir -p $(DESTDIR)$(LIBDIR)
	@cp $(BINARIES) $(DESTDIR)$(LIBDIR)

-include $(OBJS:.o=.d)
-include $(LIBK_OBJS:.o=.d)
