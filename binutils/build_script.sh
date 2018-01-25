#!/bin/bash

if [ !  -d "./build" ]; then
	mkdir build
fi

cd build
../configure --target=fusion-elf --prefix=/home/bit0fun/binutils-test #--enable-maintainer-mode
make clean
make -j4
make install
