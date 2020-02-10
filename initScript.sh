#!/usr/bin/env bash

export PATH=~/bin:$PATH

export ARCH=arm64 && SUBARCH=arm64

export CROSS_COMPILE=/media/mrfox/Projects/toolchain/aarch64-linux-android-4.9/bin/aarch64-linux-androidkernel-

export CROSS_COMPILE_ARM32=/media/mrfox/Projects/toolchain/arm-linux-androideabi-4.9/bin/arm-linux-androideabi-

export USE_CCACHE=1

export KBUILD_BUILD_HOST=foxhole

export KBUILD_BUILD_USER=mrfox

make O=out ARCH=arm64 k5fpr_defconfig
make O=out -j4
