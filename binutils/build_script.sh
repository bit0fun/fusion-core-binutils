#!/bin/bash

if [ !  -d "./build" ]; then
	mkdir build
fi

cd build
#../configure --target=fusion-elf --prefix=/home/bit0fun/binutils-test #--enable-maintainer-mode
../configure --target=fusion-elf --prefix=/opt/fusion-core-tools
make clean
make -j4
sudo make install
