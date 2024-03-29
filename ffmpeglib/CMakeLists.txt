#设置构建本机库文件所需的 CMake的最小版本
cmake_minimum_required(VERSION 3.4.1)
# release 包去除debug信息
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
# 添加头文件路径
include_directories(
        src/main/cpp
        src/main/cpp/decoder
        src/main/cpp/glm
        src/main/cpp/render
        src/main/cpp/muxer
        src/main/cpp/utils
        src/main/cpp/include
        src/main/cpp/player
        src/main/cpp/decoder/video
        src/main/cpp/decoder/audio
        src/main/cpp/render/video
        src/main/cpp/render/audio
        src/main/cpp/render/video/native_render
        src/main/cpp/render/video/opengl_render
        src/main/cpp/opengl/drawer
        src/main/cpp/opengl/egl
        src/main/cpp/opengl/drawer/proxy
)
# 定义源码所在目录
aux_source_directory(src/main/cpp SRC)


aux_source_directory(src/main/cpp/utils SRC_UTILS)
aux_source_directory(src/main/cpp/decoder SRC_DECODER)
aux_source_directory(src/main/cpp/render SRC_RENDER)
aux_source_directory(src/main/cpp/muxer SRC_MUXER)
aux_source_directory(src/main/cpp/decoder/video SRC_DECODER_VIDEO)
aux_source_directory(src/main/cpp/decoder/audio SRC_DECODER_AUDIO)
aux_source_directory(src/main/cpp/render/video SRC_RENDER_VIDEO)
aux_source_directory(src/main/cpp/render/audio SRC_RENDER_AUDIO)
aux_source_directory(src/main/cpp/render/video/native_render SRC_RENDER_VIDEO_NATIVE)
aux_source_directory(src/main/cpp/render/video/opengl_render SRC_RENDER_VIDEO_OPENGL)
aux_source_directory(src/main/cpp/player SRC_PLAYER)
aux_source_directory(src/main/cpp/opengl/drawer SRC_OPENGL_DRAWER)
aux_source_directory(src/main/cpp/opengl/egl SRC_OPENGL_EGL)
aux_source_directory(src/main/cpp/opengl/drawer/proxy SRC_OPENGL_DRAWER_PROXY)

# 将 SRC_FFPLAYER 添加到 SRC 中
list(APPEND SRC ${SRC_PLAYER})
list(APPEND SRC ${SRC_DECODER})
list(APPEND SRC ${SRC_RENDER})
list(APPEND SRC ${SRC_MUXER})
list(APPEND SRC ${SRC_UTILS})
list(APPEND SRC ${SRC_DECODER_VIDEO})
list(APPEND SRC ${SRC_DECODER_AUDIO})
list(APPEND SRC ${SRC_RENDER_VIDEO})
list(APPEND SRC ${SRC_RENDER_AUDIO})
list(APPEND SRC ${SRC_RENDER_VIDEO_NATIVE})
list(APPEND SRC ${SRC_RENDER_VIDEO_OPENGL})
list(APPEND SRC ${SRC_OPENGL_DRAWER_PROXY})
list(APPEND SRC ${SRC_OPENGL_DRAWER})
list(APPEND SRC ${SRC_OPENGL_EGL})
list(APPEND SRC ${SRC_OPENGL_DRAWER_PROXY})

#添加外部的库(可以是动态库或静态库)
#原本 jniLibs 目录需更改名称 不然会出现如下编译错误
#More than one file was found with OS independent path 'lib/armeabi-v7a/xxxx.so'.
#If you are using jniLibs and CMake IMPORTED targets, see https://developer.android.com/studio/releases/gradle-plugin#groovy
set(distribution_DIR ${CMAKE_SOURCE_DIR}/src/main/cmakeLibs/${ANDROID_ABI})

MESSAGE(WARNING "${CMAKE_SOURCE_DIR}/src/main/cmakeLibs/${ANDROID_ABI}")

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

        -landroid
        OpenSLES
        -lEGL
        -lGLESv2
        -lGLESv3
        ${log-lib})