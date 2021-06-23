#!/bin/bash
#
# This script is used to install i386-elf-tools for cross compile
# our os.
# Run this file in build directory and sudo
#
# Reference:
# https://steemit.com/esteem/@geyu/10-cross-compiler-i386-elf-gcc 
#

gmp=gmp-4.3.2
mpfr=mpfr-2.4.2
mpc=mpc-0.8.1

# build gmp

if [ ! -f gmp.mk ]; then
  curl -O ftp://gcc.gnu.org/pub/gcc/infrastructure/$gmp.tar.bz2
  tar xf $gmp.tar.bz2
  pushd $gmp 
  ./configure
  make
  make install
  popd
  touch gmp.mk
fi


# build mpfr

if [ ! -f mpfr.mk ]; then
  curl -O ftp://gcc.gnu.org/pub/gcc/infrastructure/$mpfr.tar.bz2
  tar xf $mpfr.tar.bz2
  pushd $mpfr 
  ./configure
  make
  make install
  popd
  touch mpfr.mk
fi

# build mpc

if [ ! -f mpc.mk ]; then
  curl -O ftp://gcc.gnu.org/pub/gcc/infrastructure/$mpc.tar.bz
  tar xf $mpc.tar.bz
  pushd $mpc
  ./configure
  make
  make install
  popd
  touch mpc.mk
fi

PREFIX=$(pwd)/i386-elf-gcc
TARGET=i386-elf

# build binutils
binutils=binutils-2.24

if [ ! -f binutils.mk ]; then
  # If the link 404's, look for a more recent version
  curl -O http://ftp.gnu.org/gnu/binutils/$binutils.tar.gz 
  tar xf $binutils.tar.gz

  mkdir binutils-build
  pushd binutils-build && \
  ../binutils-2.24/configure \
    --target=$TARGET \
    --enable-interwork \
    --enable-multilib \
    --disable-nls \
    --disable-werror \
    --prefix=$PREFIX 2>&1 | tee configure.log
  make all install 2>&1 | tee make.log
  popd
  touch binutils.mk
fi

# build gcc-4.9.1

if [ ! -f gcc.mk ]; then
  curl -O https://ftp.gnu.org/gnu/gcc/gcc-4.9.1/gcc-4.9.1.tar.bz2
  tar xf gcc-4.9.1.tar.bz2

  mkdir gcc-build
  pushd gcc-build
  ../gcc-4.9.1/configure \
    --target=$TARGET \
    --prefix="$PREFIX" \
    --disable-nls \
    --disable-libssp \
    --enable-languages=c \
    --without-headers
  make all-gcc 
  make all-target-libgcc 
  make install-gcc 
  make install-target-libgcc
  popd
  touch gcc.mk
fi

# build bochs with --enable-gdb-stub

if [ ! -f bochs.mk ]; then
  curl -O https://bochs.sourceforge.io/svn-snapshot/bochs-20210618.tar.gz
  tar -xvf bochs-20210618.tar.gz
  mv bochs-20210618 bochs
  pushd bochs
  ./configure --enable-gdb-stub
  make
  popd
  touch bochs.mk
fi
