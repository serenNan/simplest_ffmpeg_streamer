# 最简单的基于FFmpeg的推流器

基于雷霄骅老师的 [最简单的基于FFmpeg的推流器](https://blog.csdn.net/leixiaohua1020/article/details/39803457) 项目，针对现代 FFmpeg API 和 Linux 环境进行了修改和优化。

## 功能介绍

该项目实现了一个简单的媒体推流器，可以将本地媒体文件（如 MP4、FLV、MKV 等）推送到流媒体服务器（如 RTMP 服务器）。

主要特点：
- 支持多种媒体格式的输入文件
- 支持推送到 RTMP 流媒体服务器
- 兼容最新版本的 FFmpeg 库
- 提供友好的命令行接口

## 项目结构

```
.
├── build.sh              # 构建脚本
├── CMakeLists.txt        # CMake构建文件
├── data/                 # 示例数据
│   └── cuc_ieschool.flv  # 示例视频文件
├── README.md             # 项目说明文档
└── src/                  # 源代码目录
    ├── receiver/         # 接收器源代码
    │   └── simplest_ffmpeg_receiver.cpp
    └── streamer/         # 推流器源代码
        └── simplest_ffmpeg_streamer.cpp
```

## 环境要求

- Ubuntu 22.04 (WSL或原生)
- FFmpeg 开发库 (编译脚本会自动安装)
- GCC/G++ 编译器
- CMake 3.10+

## 编译方法

### 使用 CMake 构建

运行提供的构建脚本：
```bash
./build.sh
```

该脚本会：
- 检查并安装必要的依赖（FFmpeg开发库、CMake、pkg-config）
- 创建构建目录
- 配置并构建项目

构建完成后，可执行文件将位于 `build/` 目录中。

## 使用方法

### 推流器

基本用法：
```bash
./simplest_ffmpeg_streamer [输入文件] [输出RTMP URL]
```

例如：
```bash
./simplest_ffmpeg_streamer data/cuc_ieschool.flv rtmp://localhost/publishlive/livestream
```

如果不提供参数，程序会使用默认值：
- 输入文件：cuc_ieschool.flv
- 输出URL：rtmp://localhost/publishlive/livestream

### 接收器

基本用法：
```bash
./simplest_ffmpeg_receiver [输入RTMP URL] [输出文件]
```

例如：
```bash
./simplest_ffmpeg_receiver rtmp://localhost/publishlive/livestream output.flv
```

如果不提供参数，程序会使用默认值：
- 输入URL：rtmp://live.hkstv.hk.lxdns.com/live/hks
- 输出文件：receive.flv

## 注意事项

1. 使用前请确保已经设置好 RTMP 服务器（如 Nginx-RTMP、OBS Studio 等）。
2. 程序默认使用 FLV 封装格式进行推流，适用于 RTMP 协议。
3. 如需推送到其他协议（如 UDP），请修改源代码中相应的封装格式（mpegts）。

## 收流方法

可以使用以下工具接收和查看推送的流：
- FFplay: `ffplay rtmp://localhost/publishlive/livestream`
- VLC: 打开网络流 `rtmp://localhost/publishlive/livestream`
- OBS Studio: 添加媒体源，URL填写 `rtmp://localhost/publishlive/livestream`

## 致谢

感谢雷霄骅老师的原创项目和教程。 