#设置构建本机库文件所需的 CMake的最小版本
cmake_minimum_required(VERSION 3.4.1)

# 添加头文件路径
include_directories(
        src/main/cpp
        src/main/cpp/decoder
        src/main/cpp/render
        src/main/cpp/utils
        src/main/cpp/include
        src/main/cpp/player
        src/main/cpp/decoder/video
        src/main/cpp/decoder/audio
        src/main/cpp/render/video
        src/main/cpp/render/audio
        src/main/cpp/render/video/native_render
)

# 定义源码所在目录
aux_source_directory(src/main/cpp SRC)
aux_source_directory(src/main/cpp/utils SRC_UTILS)
aux_source_directory(src/main/cpp/decoder SRC_DECODER)
aux_source_directory(src/main/cpp/render SRC_RENDER)
aux_source_directory(src/main/cpp/decoder/video SRC_DECODER_VIDEO)
aux_source_directory(src/main/cpp/decoder/audio SRC_DECODER_AUDIO)
aux_source_directory(src/main/cpp/render/video SRC_RENDER_VIDEO)
aux_source_directory(src/main/cpp/render/audio SRC_RENDER_AUDIO)
aux_source_directory(src/main/cpp/render/video/native_render SRC_RENDER_VIDEO_NATIVE)
aux_source_directory(src/main/cpp/player SRC_PLAYER)

# 将 SRC_FFPLAYER 添加到 SRC 中
list(APPEND SRC ${SRC_PLAYER})
list(APPEND SRC ${SRC_DECODER})
list(APPEND SRC ${SRC_RENDER})
list(APPEND SRC ${SRC_UTILS})
list(APPEND SRC ${SRC_DECODER_VIDEO})
list(APPEND SRC ${SRC_DECODER_AUDIO})
list(APPEND SRC ${SRC_RENDER_VIDEO})
list(APPEND SRC ${SRC_RENDER_AUDIO})
list(APPEND SRC ${SRC_RENDER_VIDEO_NATIVE})

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
        ffmpeg
        SHARED
        IMPORTED)
#指定libavutil.so库的位置
set_target_properties(
        ffmpeg
        PROPERTIES IMPORTED_LOCATION
        ${distribution_DIR}/libffmpeg.so)

#依赖NDK中自带的log库
find_library(log-lib log)

#链接库
#链接ffmpeg so，ffmpeg模块链接有些有先后顺序，如果不注意某些方法可能在使用时报错
#用 -Wl,--start-group  -Wl,--end-group 包裹起来可以不用去留意so的顺序
target_link_libraries(
        ffplayer
        ffmpeg

        android
        # 打开opensl es支持
        OpenSLES

        ${log-lib})