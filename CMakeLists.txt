cmake_minimum_required(VERSION 3.10)
project(simplest_ffmpeg_tools VERSION 1.0.0 LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 FFmpeg 相关库
find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED libavformat libavcodec libavutil libswresample libswscale)

# 添加头文件路径
include_directories(${FFMPEG_INCLUDE_DIRS})

# 定义推流器可执行文件
add_executable(simplest_ffmpeg_streamer src/streamer/simplest_ffmpeg_streamer.cpp)
target_link_libraries(simplest_ffmpeg_streamer ${FFMPEG_LIBRARIES})

# 定义接收器可执行文件
add_executable(simplest_ffmpeg_receiver src/receiver/simplest_ffmpeg_receiver.cpp)
target_link_libraries(simplest_ffmpeg_receiver ${FFMPEG_LIBRARIES})

# 安装目标
install(TARGETS simplest_ffmpeg_streamer simplest_ffmpeg_receiver 
        DESTINATION bin)

# 复制示例文件
install(FILES data/cuc_ieschool.flv 
        DESTINATION share/simplest_ffmpeg_tools
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                  GROUP_READ GROUP_EXECUTE
                  WORLD_READ WORLD_EXECUTE) 