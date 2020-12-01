#设置构建本机库文件所需的 CMake的最小版本
cmake_minimum_required(VERSION 3.4.1)

# 添加头文件路径
include_directories(
        src/main/cpp
        src/main/cpp/include
)

# 定义源码所在目录
aux_source_directory(src/main/cpp SRC)
aux_source_directory(src/main/cpp/include INCLUDE)

# 将 SRC_FFMPEG 添加到 SRC 中
list(APPEND SRC ${SRC_FFMPEG})

#添加外部的库(可以是动态库或静态库)
set(distribution_DIR ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI})

MESSAGE(WARNING "${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}")

#添加自己写的 C/C++源文件
add_library(
        utils #so名称
        SHARED #动态库
        ${INCLUDE}
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

#依赖NDK中自带的log库
find_library(log-lib log)

#链接库
target_link_libraries(
        utils
        avutil
        ${log-lib})