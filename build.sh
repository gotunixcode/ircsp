#!/bin/sh

if [ "$#" -eq 0 ];then
	echo "Please specify install location"
	exit 0
fi

echo "-> Running clean.sh"
sh clean.sh
#echo "-> Running library/clean.sh"
#sh library/clean.sh

#echo "Building IRCSP libraries"
#cd library
#echo "-> Running autogen.sh"
#sh autogen.sh
#echo "-> Running configure script"
#./configure --prefix=$1
#echo "-> Cleaning"
#make clean
#echo "-> Building source"
#make
#echo "-> Installing to $1"
#make install
#cd ..


echo "Building IRCSP"
echo "-> Running autogen.sh"
sh autogen.sh
echo "-> Creating version.c"
sh version.sh
echo "-> Running configure script"
./configure --prefix=$1 --enable-debug
#./configure --prefix=$1 --enable-debug --enable-ircup10
echo "-> Cleaning.."
make clean
echo "-> Building"
make
echo "-> Installing"
make install
