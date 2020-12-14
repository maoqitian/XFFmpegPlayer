#!/bin/bash

# ndk路径
NDK=/Users/maoqitian/Library/Android/ndk/android-ndk-r21b
# 编译工具链目录，ndk17版本以上用的是clang，以下是gcc
TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/darwin-x86_64
# 版本号
API=21
# 交叉编译树的根目录(查找相应头文件和库用)
SYSROOT="${TOOLCHAIN}/sysroot"


# 定义执行configure的shell方法
function build_android() {
    #进入对应源码目录
    cd ffmpeg-4.3.1
    echo "Compiling FFmpeg for $CPU"
    ./configure \
        --prefix=$PREFIX \
        --libdir=$LIB_DIR \
        --enable-static \
        --disable-shared \
        --enable-jni \
        --disable-doc \
        --disable-symver \
        --disable-programs \
        --target-os=android \
        --arch=$ARCH \
        --cpu=$CPU \
        --cc=$CC \
        --cxx=$CXX \
        --enable-cross-compile \
        --sysroot=$SYSROOT \
        --cross-prefix=$CROSS_PREFIX \
        --extra-cflags="-Os -fpic $OPTIMIZE_CFLAGS" \
        --extra-ldflags="$ADDI_LDFLAGS" \
        --disable-asm \
        $COMMON_FF_CFG_FLAGS
    make clean
    make -j12
    make install

    echo "The Compilation of FFmpeg for $CPU is completed"
    echo "Output library : $LIB_DIR"
    ## 回到上一级目录
    cd ..
}
## armv8-a
function v8() {
    source "config-env.sh"
    OUTPUT_FOLDER="arm64-v8a"
    ARCH=arm64
    CPU="armv8-a"
    TOOL_CPU_NAME=aarch64
    TOOL_PREFIX="$TOOLCHAIN/bin/$TOOL_CPU_NAME-linux-android"
    CROSS_PREFIX="$TOOLCHAIN/bin/$TOOL_CPU_NAME-linux-android-"

    CC="$TOOL_PREFIX$API-clang"
    CXX="$TOOL_PREFIX$API-clang++"
    SYSROOT="$NDK/toolchains/llvm/prebuilt/darwin-x86_64/sysroot"
    PREFIX="${PWD}/android/$OUTPUT_FOLDER"
    LIB_DIR="${PWD}/android/libs/$OUTPUT_FOLDER"
    OPTIMIZE_CFLAGS="-march=$CPU"
    build_android
}

## armv7-a
function v7() {
    source "config-env.sh"
    OUTPUT_FOLDER="armeabi-v7a"
    ARCH="arm"
    CPU="armv7-a"
    TOOL_CPU_NAME=armv7a
    TOOL_PREFIX="$TOOLCHAIN/bin/arm-linux-androideabi"
    CROSS_PREFIX="$TOOLCHAIN/bin/arm-linux-androideabi-"

    CC="$TOOLCHAIN/bin/armv7a-linux-androideabi$API-clang"
    CXX="$TOOLCHAIN/bin/armv7a-linux-androideabi$API-clang++"
    SYSROOT="$NDK/toolchains/llvm/prebuilt/darwin-x86_64/sysroot"
    PREFIX="${PWD}/android/$OUTPUT_FOLDER"
    LIB_DIR="${PWD}/android/libs/$OUTPUT_FOLDER"
    OPTIMIZE_CFLAGS="-march=$CPU"
    build_android
}

## x86
function x86() {
    source "config-env.sh"
    OUTPUT_FOLDER="x86"
    ARCH="x86"
    CPU="x86"
    TOOL_CPU_NAME="i686"
    TOOL_PREFIX="$TOOLCHAIN/bin/${TOOL_CPU_NAME}-linux-android"
    CROSS_PREFIX="$TOOLCHAIN/bin/${TOOL_CPU_NAME}-linux-android-"

    CC="$TOOL_PREFIX$API-clang"
    CXX="$TOOL_PREFIX$API-clang++"
    SYSROOT="$NDK/toolchains/llvm/prebuilt/darwin-x86_64/sysroot"
    PREFIX="${PWD}/android/$OUTPUT_FOLDER"
    LIB_DIR="${PWD}/android/libs/$OUTPUT_FOLDER"
    OPTIMIZE_CFLAGS="-march=i686 -mtune=intel -mssse3 -mfpmath=sse -m32"
    build_android
}

## x86_64
function x86_64() {
    source "config-env.sh"
    OUTPUT_FOLDER="x86_64"
    ARCH="x86_64"
    CPU="x86-64"
    TOOL_CPU_NAME="x86_64"
    TOOL_PREFIX="$TOOLCHAIN/bin/${TOOL_CPU_NAME}-linux-android"
    CROSS_PREFIX="$TOOLCHAIN/bin/${TOOL_CPU_NAME}-linux-android-"

    CC="$TOOL_PREFIX$API-clang"
    CXX="$TOOL_PREFIX$API-clang++"
    SYSROOT="$NDK/toolchains/llvm/prebuilt/darwin-x86_64/sysroot"
    PREFIX="${PWD}/android/$OUTPUT_FOLDER"
    LIB_DIR="${PWD}/android/libs/$OUTPUT_FOLDER"
    OPTIMIZE_CFLAGS="-march=$CPU"
    build_android
}

# 简单使用说明

if [ -z $1 ]; then
    echo "Usage:"
    echo "  $0 all"
    echo "  $0 armv8"
    echo "    all support cpu :armv7 armv8 x86 x86_64"
    exit -1
fi

# 检测构建全部

HAVE_ALL=0

for arg in $@; do

    if [ "$arg" == 'all' ]; then
        HAVE_ALL=1
    fi

done

if [ $HAVE_ALL -eq 1 ]; then

    x86
    x86_64
    v8
    v7
    exit 0

fi

# 根据输入参数检测构建哪种cpu

for arg in $@; do

    if [ "$arg" == 'x86' ]; then
        x86
    fi

    if [ "$arg" == 'x86_64' ]; then
        x86_64
    fi

    if [ "$arg" == 'armv8' ]; then
        v8
    fi

    if [ "$arg" == 'armv7' ]; then
        v7
    fi

done

# 当前NDK版本为r21b, ffmpeg版本为4.3.1
# 在原作者基础上有修改
# 1. API等级是21
# 2. 4.3.1 版本并不需要手动修改ffmpeg的configure文件,因为打出来的so文件是没有版本号的
# 3. ffmpeg 目录结构有修改, 所以有一个cd的过程

# 目录结构
# tree -L 1
# .
# ├── android # 输出目录
# ├── build_ffmpeg_android.sh # 脚本
# ├── config-env.sh # 选项
# └── ffmpeg-4.3.1 # ffmpeg 目录

# 编译选项可以使用 ./configure -h 查看, 可以根据自己的需求开启或关闭编解码支持, 会显著降低so大小
# 本选项编译出的文件文件尺寸如下:
# ll android/libs/arm64-v8a 
# total 11624
# -rwxr-xr-x  1 caijinglong  staff   3.9M  7 26 16:28 libavcodec.so
# -rwxr-xr-x  1 caijinglong  staff   106K  7 26 16:28 libavfilter.so
# -rwxr-xr-x  1 caijinglong  staff   771K  7 26 16:28 libavformat.so
# -rwxr-xr-x  1 caijinglong  staff   443K  7 26 16:28 libavutil.so
# -rwxr-xr-x  1 caijinglong  staff    82K  7 26 16:28 libswresample.so
# -rwxr-xr-x  1 caijinglong  staff   398K  7 26 16:28 libswscale.so

# 如果想要和ijkplayer那样将所有so合并为一个libffmpeg.so库,可以看下面的说明
# 说是合并,实际上so是不支持合并的, 需要编译出.a静态库而不是so动态库, 然后将多个静态库合并为一个so, 然后用TOOLCHAIN中的gcc命令进行合并
# gcc -shared -o -Wl --whole-archive avcodec.a avfilter.a other.a -Wl,--no-whole-archive
