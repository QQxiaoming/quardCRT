[![Windows ci](https://img.shields.io/github/actions/workflow/status/qqxiaoming/quardCRT/windows.yml?branch=main&logo=windows)](https://github.com/QQxiaoming/quardCRT/actions/workflows/windows.yml)
[![Linux ci](https://img.shields.io/github/actions/workflow/status/qqxiaoming/quardCRT/linux.yml?branch=main&logo=linux)](https://github.com/QQxiaoming/quardCRT/actions/workflows/linux.yml)
[![Macos ci](https://img.shields.io/github/actions/workflow/status/qqxiaoming/quardCRT/macos.yml?branch=main&logo=apple)](https://github.com/QQxiaoming/quardCRT/actions/workflows/macos.yml)
[![CodeFactor](https://img.shields.io/codefactor/grade/github/qqxiaoming/quardCRT.svg?logo=codefactor)](https://www.codefactor.io/repository/github/qqxiaoming/quardCRT)
[![License](https://img.shields.io/github/license/qqxiaoming/quardCRT.svg?colorB=f48041&logo=gnu)](https://github.com/QQxiaoming/quardCRT)
[![GitHub tag (latest SemVer)](https://img.shields.io/github/tag/QQxiaoming/quardCRT.svg?logo=git)](https://github.com/QQxiaoming/quardCRT/releases)
[![GitHub All Releases](https://img.shields.io/github/downloads/QQxiaoming/quardCRT/total.svg?logo=pinboard)](https://github.com/QQxiaoming/quardCRT/releases)
[![GitHub stars](https://img.shields.io/github/stars/QQxiaoming/quardCRT.svg?logo=github)](https://github.com/QQxiaoming/quardCRT)
[![GitHub forks](https://img.shields.io/github/forks/QQxiaoming/quardCRT.svg?logo=github)](https://github.com/QQxiaoming/quardCRT)
[![Gitee stars](https://gitee.com/QQxiaoming/quardCRT/badge/star.svg?theme=dark)](https://gitee.com/QQxiaoming/quardCRT)
[![Gitee forks](https://gitee.com/QQxiaoming/quardCRT/badge/fork.svg?theme=dark)](https://gitee.com/QQxiaoming/quardCRT)

# quardCRT

[English](./README.md) | 简体中文

quardCRT一款多功能终端仿真/图形桌面软件，支持多种后端协议，无依赖跨平台使用，windows/linux/mac使用体验完全一致，支持多标签页和历史记录管理等传统终端软件功能，同时支持一些独具特色的细节功能。

| ![img2](./docs/windows.png) |
| :-------------------------: |
| Windows                     |
| ![img1](./docs/macos.png)   |
| MacOS                       |
| ![img3](./docs/linux.png)   |
| Linux                       |

亮/暗主题切换：

| ![img4](./docs/dark.png)    | ![img5](./docs/light.png)   |
| :-------------------------: | :-------------------------: |
| 暗主题                      | 亮主题                      |

协议选择界面：

![img](./docs/img.png)

## 功能描述

### 目前支持的终端协议

- ssh
- telnet (支持带websocket封装)
- serial
- loaclshell
- rawsocket
- windows:NamedPipe（linux/macos:unix domain socket）

### 目前支持的图形桌面协议

- vnc

### 基本功能

- 会话记录管理
- 多标签页管理，标签页克隆，标签页拖拽排序
- 双列分屏显示，标签页自由拖拽至分列
- 终端样式配置（配色方案，字体）
- HEX显示
- 终端背景图片配置
- 终端滚动行数设置
- 支持深色/浅色主题
- UI支持多语言（简体中文/繁体中文/英语/日语/韩语/西班牙语/法语/俄语）

### 特色功能 （带视频演示，需要前往github查看）

| 标签页悬浮预览 |
| :------------: |
| <video src="https://github.com/QQxiaoming/quardCRT/assets/27486515/5ecc8560-94ed-4d6e-90c4-7c18cce0db63"></video> |
| 浮动窗口支持，可将标签页拖拽至浮动窗口 |
| <video src="https://github.com/QQxiaoming/quardCRT/assets/27486515/15a9401a-5302-44c4-a693-e5e3043d8ca6"></video> |
| SSH2会话一键打开SFTP文件传输窗口 |
| <video src="https://github.com/QQxiaoming/quardCRT/assets/27486515/3097025c-3279-4c5b-b5fe-166607211dd9"></video> |
| 本地终端工作目录书签 |
| <video src="https://github.com/QQxiaoming/quardCRT/assets/27486515/82a2866e-cf45-4933-8638-777f6baff682"></video> |
| 自动化发送 |
| <video src="https://github.com/QQxiaoming/quardCRT/assets/27486515/59d05a59-31fd-4133-8dbe-deb43122fe8c"></video> |
| 终端背景图片支持gif动画和视频 |
| <video src="https://github.com/QQxiaoming/quardCRT/assets/27486515/4fdb10c3-b754-4b1c-8dc3-bbe83d3b0fcf"></video> |
| 终端关键词高亮匹配 |
| <video src="https://github.com/QQxiaoming/quardCRT/assets/27486515/2d136273-8a53-4d4c-9cae-4609600bd32a"></video> |
| 选中文本翻译功能 |
| <video src="https://github.com/QQxiaoming/quardCRT/assets/27486515/aae7a324-808f-45e5-a86d-579d9002b28b"></video> |
| 路径匹配与一键直达 |
| <video src="https://github.com/QQxiaoming/quardCRT/assets/27486515/4aea9223-babd-4715-a908-56227ba04fed"></video> |
| 工作路径直达 |
| <video src="https://github.com/QQxiaoming/quardCRT/assets/27486515/616c1c09-ffa1-428d-b897-1966ecb7517b"></video> |
| windows本地终端增强（Tab键选择补全命令等） |
| <video src="https://github.com/QQxiaoming/quardCRT/assets/27486515/f60eed90-5c77-48c9-9c14-71f4a3d993b9"></video> |

## 计划中特性

- [ ] xyzmodem协议支持
- [ ] 支持操作脚本录制/加载
- [ ] 支持终端显示录制
- [ ] 会话状态查询
- [ ] 终端样式自定义
- [ ] 独立会话设置终端外观
- [ ] GitHub Copilot插件支持
- [ ] CI支持Appimage包
- [ ] CI支持windows on arm64

## 翻译

| English  | ![en_US](./docs/en_US.svg) |
| :------: | :------------------------: |
| 简体中文 | ![zh_CN](./docs/zh_CN.svg) |
| 繁體中文 | ![zh_HK](./docs/zh_HK.svg) |
| 日本語   | ![ja_JP](./docs/ja_JP.svg) |
| 한국어   | ![ko_KR](./docs/ko_KR.svg) |
| Español   | ![es_ES](./docs/es_ES.svg) |
| Français   | ![fr_FR](./docs/fr_FR.svg) |
| Русский   | ![ru_RU](./docs/ru_RU.svg) |

## 插件

quardCRT将从V0.4.0版本开始支持插件，插件将以Qt Plug-in插件的形式提供，以动态库的形式加载，想了解更多插件开发信息请参考插件开放平台[https://github.com/QuardCRT-platform](https://github.com/QuardCRT-platform)，此平台将提供插件开发的模板仓库以及相关示例。目前插件功能仍处于早期开发阶段，如果您有好的想法或建议，欢迎在GitHub或Gitee上提交issue或discussion。

## 编译说明

请参考[开发笔记](./DEVELOPNOTE.md)。

## 贡献

如果您对本项目有建议或想法，欢迎在GitHub或Gitee上提交issue和pull requests。

目前项目建议使用版本Qt6.2.0及更高版本。

## 特别

项目目前为个人业余时间开发，为提高开发效率，本项目较为大量的使用了GitHub Copilot协助代码编写，部分代码的人类可读性可能不是很好，作者也会尽量在后续版本中进行优化。

## 感谢

本项目代码引用或部份参考或依赖了以下开源项目，项目完全尊重原项目开源协议，并在此表示感谢。

- [QDarkStyleSheet](https://github.com/ColinDuquesnoy/QDarkStyleSheet)
- [QFontIcon](https://github.com/dridk/QFontIcon)
- [QTelnet](https://github.com/silderan/QTelnet)
- [qtermwidget](https://github.com/lxqt/qtermwidget)
- [ptyqt](https://github.com/kafeg/ptyqt)
- [argv_split](https://github.com/bitmeal/argv_split)
- [iTerm2-Color-Schemes](https://github.com/mbadolato/iTerm2-Color-Schemes)
- [winpty](https://github.com/rprichard/winpty)
- [QtFancyTabWidget](https://github.com/SM-nzberg/QtFancyTabWidget)
- [qtftp](https://github.com/teknoraver/qtftp)
- [utf8proc](https://github.com/JuliaStrings/utf8proc)
- [fcitx-qt5](https://github.com/fcitx/fcitx-qt5)
- [libssh2](https://github.com/libssh2/libssh2)
- [QtSsh](https://github.com/condo4/QtSsh)
- [QCustomFileSystemModel](https://github.com/QQxiaoming/QCustomFileSystemModel)
- [qtkeychain](https://github.com/frankosterfeld/qtkeychain)
- [qvncclient](https://bitbucket.org/amahta/qvncclient)
- [qhexedit](https://github.com/Simsys/qhexedit2)
- [QGoodWindow](https://github.com/antonypro/QGoodWindow)
