#!/bin/bash

rm -rf build
mkdir build
cd build
../configure --target=fusion-elf --prefix=/home/bit0fun/binutils-test
make -j4
