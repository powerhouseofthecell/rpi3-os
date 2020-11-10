#!/bin/bash
# probs don't actually execute this script, just fyi
sudo apt-get purge "qemu"
sudo apt-get build-dep qemu
tar -xJf TARFILE.tar.xz
cd TARFILE
./configure --target-list=aarch64-softmmu,aarch64-linux-user --enable-modules --enable-tcg-interpreter --enable-debug-tcg --enable-sdl --disable-vnc
make
sudo checkinstall make install
sudo apt-get install ./*.deb
qemu-system-aarch64 --version