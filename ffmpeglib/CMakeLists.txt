#设置构建本机库文件所需的 CMake的最小版本
cmake_minimum_required(VERSION 3.4.1)

# 添加头文件路径
include_directories(
        src/main/cpp
        src/main/cpp/include
        src/main/cpp/ffplayer
)

# 定义源码所在目录
aux_source_directory(src/main/cpp SRC)
aux_source_directory(src/main/cpp/ffplayer SRC_FFPLAYER)

# 将 SRC_FFPLAYER 添加到 SRC 中
list(APPEND SRC ${SRC_FFPLAYER})

#添加外部的库(可以是动态库或静态库)
set(distribution_DIR ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI})

MESSAGE(WARNING "${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}")

#添加自己写的 C/C++源文件 CMAKE_SOURCE_DIR被定为到app/src/main/cpp下，所以配置的src/main/…/会定位不到，
#所以将app/src/main/cpp目录下的CMakeLists剪切到app目录下与src同级
add_library(
        ffplayer #so名称
        SHARED #动态库
        ${SRC} #源码目录
)

#libavutil.so
add_library(
        avutil
        SHARED
        IMPORTED)
#指定libavutil.so库的位置
set_target_properties(
        avutil
        PROPERTIES IMPORTED_LOCATION
        ${distribution_DIR}/libavutil.so)

#libavcodec.so
add_library(
        avcodec
        SHARED
        IMPORTED)
#指定libavutil.so库的位置
set_target_properties(
        avcodec
        PROPERTIES IMPORTED_LOCATION
        ${distribution_DIR}/libavcodec.so)

#libavdevice.so
add_library(
        avdevice
        SHARED
        IMPORTED)
#指定libavutil.so库的位置
set_target_properties(
        avdevice
        PROPERTIES IMPORTED_LOCATION
        ${distribution_DIR}/libavdevice.so)

#libavfilter.so
add_library(
        avfilter
        SHARED
        IMPORTED)
#指定libavutil.so库的位置
set_target_properties(
        avfilter
        PROPERTIES IMPORTED_LOCATION
        ${distribution_DIR}/libavfilter.so)

#libavformat.so
add_library(
        avformat
        SHARED
        IMPORTED)
#指定libavutil.so库的位置
set_target_properties(
        avformat
        PROPERTIES IMPORTED_LOCATION
        ${distribution_DIR}/libavformat.so)
#libswresample.so
add_library(
        postproc
        SHARED
        IMPORTED)
#指定libavutil.so库的位置
set_target_properties(
        postproc
        PROPERTIES IMPORTED_LOCATION
        ${distribution_DIR}/libpostproc.so)
#libswresample.so
add_library(
        swresample
        SHARED
        IMPORTED)
#指定libavutil.so库的位置
set_target_properties(
        swresample
        PROPERTIES IMPORTED_LOCATION
        ${distribution_DIR}/libswresample.so)

#libswscale.so
add_library(
        swscale
        SHARED
        IMPORTED)
#指定libavutil.so库的位置
set_target_properties(
        swscale
        PROPERTIES IMPORTED_LOCATION
        ${distribution_DIR}/libswscale.so)

#依赖NDK中自带的log库
find_library(log-lib log)

#链接库
#链接ffmpeg so，ffmpeg模块链接有些有先后顺序，如果不注意某些方法可能在使用时报错
#用 -Wl,--start-group  -Wl,--end-group 包裹起来可以不用去留意so的顺序
target_link_libraries(
        ffplayer
        -Wl,--start-group
        avcodec
        avdevice
        avfilter
        avformat
        avutil
        postproc
        swresample
        swscale
        -Wl,--end-group

        android
        OpenSLES

        ${log-lib})