#!/bin/bash

# 检查并安装必要的库
echo "检查必要依赖..."
if ! dpkg -l | grep -qw libavformat-dev; then
    echo "安装FFmpeg开发库..."
    echo "可能需要sudo权限，如果提示权限问题，请手动运行: sudo apt-get update && sudo apt-get install -y libavformat-dev libavcodec-dev libavutil-dev libswresample-dev libswscale-dev"
    sudo apt-get update && sudo apt-get install -y libavformat-dev libavcodec-dev libavutil-dev libswresample-dev libswscale-dev
fi

if ! dpkg -l | grep -qw cmake; then
    echo "安装CMake..."
    sudo apt-get install -y cmake
fi

if ! dpkg -l | grep -qw pkg-config; then
    echo "安装pkg-config..."
    sudo apt-get install -y pkg-config
fi

# 创建构建目录
echo "创建构建目录..."
mkdir -p build
cd build

# 配置和构建
echo "配置CMake项目..."
cmake ..

echo "构建项目..."
make -j$(nproc)

echo "构建完成！"
echo ""
echo "现在可以使用以下命令运行推流器:"
echo "  ./simplest_ffmpeg_streamer [输入文件] [输出RTMP URL]"
echo "例如:"
echo "  ./simplest_ffmpeg_streamer ../data/cuc_ieschool.flv rtmp://localhost/publishlive/livestream"
echo ""
echo "现在可以使用以下命令运行收流器:"
echo "  ./simplest_ffmpeg_receiver [输入RTMP URL] [输出文件]"
echo "例如:"
echo "  ./simplest_ffmpeg_receiver rtmp://localhost/publishlive/livestream output.flv" 