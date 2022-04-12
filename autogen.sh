#!/bin/sh

# This script will generate all the initial makefiles, etc.

DIE=0

(autoconf --version) < /dev/null > /dev/null 2>&1 || {
        echo
        echo "You must have autoconf installed to compile this package."
        echo "Download the appropriate package for your distribution,"
        echo "or get the source tarball at ftp://gnu.org/pub/gnu/autoconf"
        DIE=1
}

(automake --version) < /dev/null > /dev/null 2>&1 || {
        echo
        echo "You must have automake installed to compile this package."
        echo "Get it at ftp://ftp.gnu.org/pub/gnu/automake"
        DIE=1
}

if test "$DIE" -eq 1; then
        exit 1
fi

echo
echo
echo "* IRCSP "
echo "* Copyright (C) 1995 - 2019 GOTUNIX Networks."
echo
echo "Generating configuration and make files for IRCSP, Please wait..."

echo "Creating version.c..." ; /bin/sh version.sh

aclocal $ACLOCAL_FLAGS
libtoolize
echo "Running autoheader..."
autoheader > /dev/null
echo "Running automake..."
automake --add-missing --warnings=none > /dev/null
echo "Running autoconf..."
autoconf > /dev/null
mkdir m4
echo "Completed. You can now run './configure'"
