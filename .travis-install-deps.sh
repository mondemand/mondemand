#!/bin/sh

set -e
cwd=`pwd`
depdir="$cwd/deps"
mkdir -p $depdir
srcdir="$depdir/source"
mkdir -p $srcdir
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$depdir/lib/pkgconfig

# lwes
LWES_VERSION=1.1.1

cd $srcdir
wget https://github.com/lwes/lwes/releases/download/${LWES_VERSION}/lwes-${LWES_VERSION}.tar.gz
tar -xzvf lwes-${LWES_VERSION}.tar.gz
cd lwes-${LWES_VERSION} \
  && ./configure --disable-hardcore --prefix=$depdir \
  && make install
