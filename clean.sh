#!/bin/sh

make clean > /dev/null
rm -rf clean
rm -rf aclocal.m4 config.guess config.sub depcomp libtool missing 
rm -rf config.h  configure ltmain.sh config.h.in autom4te.cache
rm -rf config.log config.status install-sh 
rm -rf stamp-h1 source/*.o
rm -rf source/.deps source/ircsp
rm -rf configure.output
rm -rf compile
rm -rf m4
echo "Removing OS X cache files"
find ./ -name ".DS_Store" -type f -exec rm -f {} \;
echo "Removing Makefiles"
find ./ -name "Makefile" -type f -exec rm -f {} \;
find ./ -name "Makefile.in" -type f -exec rm -f {} \;
echo "Removing objects"
find ./ -name "*.o" -type f -exec rm -f {} \;
find ./ -name ".deps" -type d -exec rm -rf {} \;
echo "Clearning Module Files"
find ./ -name "*.lo" -type f -exec rm -f {} \;
find ./ -name "*.la" -type f -exec rm -f {} \;
