#!/bin/sh

#
# configure.ac ---> aclocal.m4
#
aclocal --force

#
# configure.ac + aclocal.m4 ---> configure
#
autoconf --force

#
# configure.ac + Makefile.am ---> Makefile.in
#
automake --add-missing --force-missing --foreign

#
# Makefile.in ---> Makefile
#
# The "arm-none-linux-gnueabi" must be a valid entry
# of the file "/usr/share/automake-1.11/config.sub".
#
# You can find keywords "Decode aliases for certain cpu-company
# combinations" from the file "/usr/share/automake-1.11/config.sub",
# and insert "arm-none-linux-gnueabi" into the file.
#
./configure                   \
--build=i686-pc-linux-gnu     \
--host=arm-none-linux-gnueabi \
CPPFLAGS="-DNDEBUG            \
          -D_GNU_SOURCE       \
          -D_LIBC_REENTRANT   \
          -D_REENTRANT"       \
CFLAGS="  -O2 -Wall"          \
CXXFLAGS="-O2 -Wall"          \
LDFLAGS="" $@
