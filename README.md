[![Windows ci](https://img.shields.io/github/actions/workflow/status/qqxiaoming/quardCRT/windows.yml?branch=main&logo=windows)](https://github.com/QQxiaoming/quardCRT/actions/workflows/windows.yml)
[![Linux ci](https://img.shields.io/github/actions/workflow/status/qqxiaoming/quardCRT/linux.yml?branch=main&logo=linux)](https://github.com/QQxiaoming/quardCRT/actions/workflows/linux.yml)
[![Macos ci](https://img.shields.io/github/actions/workflow/status/qqxiaoming/quardCRT/macos.yml?branch=main&logo=apple)](https://github.com/QQxiaoming/quardCRT/actions/workflows/macos.yml)
[![CodeFactor](https://img.shields.io/codefactor/grade/github/qqxiaoming/quardCRT.svg?logo=codefactor)](https://www.codefactor.io/repository/github/qqxiaoming/quardCRT)
[![Documentation Status](https://readthedocs.org/projects/quardcrt/badge/?version=latest)](https://quardcrt.readthedocs.io/en/latest/?badge=latest)
[![License](https://img.shields.io/github/license/qqxiaoming/quardCRT.svg?colorB=f48041&logo=gnu)](https://github.com/QQxiaoming/quardCRT)
[![GitHub tag (latest SemVer)](https://img.shields.io/github/v/tag/QQxiaoming/quardCRT?filter=V*&logo=git)](https://github.com/QQxiaoming/quardCRT/releases)
[![GitHub All Releases](https://img.shields.io/github/downloads/QQxiaoming/quardCRT/total.svg?logo=pinboard)](https://github.com/QQxiaoming/quardCRT/releases)
[![GitHub stars](https://img.shields.io/github/stars/QQxiaoming/quardCRT.svg?logo=github)](https://github.com/QQxiaoming/quardCRT)
[![GitHub forks](https://img.shields.io/github/forks/QQxiaoming/quardCRT.svg?logo=github)](https://github.com/QQxiaoming/quardCRT)
[![Gitee stars](https://gitee.com/QQxiaoming/quardCRT/badge/star.svg?theme=dark)](https://gitee.com/QQxiaoming/quardCRT)
[![Gitee forks](https://gitee.com/QQxiaoming/quardCRT/badge/fork.svg?theme=dark)](https://gitee.com/QQxiaoming/quardCRT)

<div style="text-align: right">
    <a href="https://apps.microsoft.com/detail/quardCRT/9p6102k9qb3t?mode=direct">
        <img src="https://get.microsoft.com/images/en-us%20dark.svg" width="200"/>
    </a>
</div>

# quardCRT

🇺🇸 English | [🇨🇳 简体中文](./README_zh_CN.md) | [🇭🇰 繁體中文](./README_zh_HK.md) | [🇯🇵 日本語](./README_ja_JP.md)

quardCRT is a terminal emulation and remote desktop software that supports multiple backend protocols, can be used across platforms without dependencies, and has a completely consistent user experience on windows/linux/mac. It supports traditional terminal software functions such as multi-tab and history management, and supports some unique detailed functions. The design purpose of quardCRT is to create a terminal software that is as user-friendly, feature-rich, and cross-platform consistent as possible. Compared to many professional high-performance terminals, quardCRT will be more suitable for beginners and light users to quickly configure the required terminal environment, but this does not mean that quardCRT does not pursue high performance.

| ![img2](./docs/img/windows.png) |
| :-------------------------: |
| Windows                     |
| ![img1](./docs/img/macos.png)   |
| MacOS                       |
| ![img3](./docs/img/linux.png)   |
| Linux                       |

Light/dark theme switching:

| ![img4](./docs/img/dark.png)    | ![img5](./docs/img/light.png)   |
| :-------------------------: | :-------------------------: |
| Dark theme                  | Light theme                 |

Protocol selection interface:

![img](./docs/img/img.png)

## Feature

### Currently supported terminal protocols

- ssh
- telnet (Support websocket wrapper)
- serial
- loaclshell
- rawsocket
- windows:NamedPipe（linux/macos:unix domain socket）

### Currently supported remote desktop protocols

- vnc

### Basic functions

- Session history management
- Multi-tab management, tab cloning, tab drag-and-drop sorting
- Dual column split screen, tab drag-and-drop to split column
- Terminal style configuration (color scheme, font)
- HEX display
- Terminal background image configuration
- Terminal scroll line configuration
- Support x\y\zmodem protocol
- Support dark/light theme
- Support multiple languages (Simple Chinese/Traditional Chinese/English/Japanese/Korean/Spanish/French/Russian/German/Portuguese (Brazil)/Czech/Arabic)

### Special functions

| Tab floating preview |
| :------------------: |
| <video src="https://github.com/QQxiaoming/quardCRT/assets/27486515/5ecc8560-94ed-4d6e-90c4-7c18cce0db63"></video> |
| Floating window support, tab drag-and-drop to floating window |
| <video src="https://github.com/QQxiaoming/quardCRT/assets/27486515/15a9401a-5302-44c4-a693-e5e3043d8ca6"></video> |
| SSH2 session one-click open SFTP file transfer window |
| <video src="https://github.com/QQxiaoming/quardCRT/assets/27486515/3097025c-3279-4c5b-b5fe-166607211dd9"></video> |
| Working directory bookmark |
| <video src="https://github.com/QQxiaoming/quardCRT/assets/27486515/82a2866e-cf45-4933-8638-777f6baff682"></video> |
| Automated sending |
| <video src="https://github.com/QQxiaoming/quardCRT/assets/27486515/59d05a59-31fd-4133-8dbe-deb43122fe8c"></video> |
| Terminal background image supports gif animation and video |
| <video src="https://github.com/QQxiaoming/quardCRT/assets/27486515/4fdb10c3-b754-4b1c-8dc3-bbe83d3b0fcf"></video> |
| Terminal keyword highlight matching |
| <video src="https://github.com/QQxiaoming/quardCRT/assets/27486515/2d136273-8a53-4d4c-9cae-4609600bd32a"></video> |
| Selected text translation function |
| <video src="https://github.com/QQxiaoming/quardCRT/assets/27486515/aae7a324-808f-45e5-a86d-579d9002b28b"></video> |
| Path matching and one-click direct |
| <video src="https://github.com/QQxiaoming/quardCRT/assets/27486515/4aea9223-babd-4715-a908-56227ba04fed"></video> |
| Working path direct |
| <video src="https://github.com/QQxiaoming/quardCRT/assets/27486515/616c1c09-ffa1-428d-b897-1966ecb7517b"></video> |
| Windows local terminal enhancement (Tab key to select complete command, etc.) |
| <video src="https://github.com/QQxiaoming/quardCRT/assets/27486515/f60eed90-5c77-48c9-9c14-71f4a3d993b9"></video> |

## Planned features

- [ ] Support operation script recording/loading
- [ ] Support screen recording
- [ ] session status query
- [ ] Terminal style customization
- [ ] Independent session set terminal appearance
- [ ] GitHub Copilot plugin support
- [ ] CI support windows on arm64

## Translation

quardCRT supports multiple languages, currently supports the following languages, translation coverage is as follows:

| 🇺🇸 English   | ![en_US](./docs/img/en_US.svg) |
| :----------: | :------------------------: |
| 🇨🇳 简体中文  | ![zh_CN](./docs/img/zh_CN.svg) |
| 🇭🇰 繁體中文  | ![zh_HK](./docs/img/zh_HK.svg) |
| 🇯🇵 日本語    | ![ja_JP](./docs/img/ja_JP.svg) |
| 🇰🇷 한국어    | ![ko_KR](./docs/img/ko_KR.svg) |
| 🇪🇸 Español   | ![es_ES](./docs/img/es_ES.svg) |
| 🇫🇷 Français  | ![fr_FR](./docs/img/fr_FR.svg) |
| 🇷🇺 Русский   | ![ru_RU](./docs/img/ru_RU.svg) |
| 🇩🇪 Deutsch   | ![de_DE](./docs/img/de_DE.svg) |
| 🇧🇷 Português | ![pt_BR](./docs/img/pt_BR.svg) |
| 🇨🇿 čeština   | ![cs_CZ](./docs/img/cs_CZ.svg) |
| 🇸🇦 عربي     | ![ar_SA](./docs/img/ar_SA.svg) |

The translation of quartCRT is assisted by github copilot, and the translation may not be very accurate. If you find any problems with the translation, please submit an issue or pull request.

## Plugin

quardCRT will support plugins from version V0.4.0, plugins will be provided in the form of Qt plugins, loaded in the form of dynamic libraries, to learn more about plugin development information, please refer to the plugin open platform [https://github.com/QuardCRT-platform](https://github.com/QuardCRT-platform), This platform will provide template repositories and related examples for plugin development. At present, the plugin function is still in the early development stage. If you have good ideas or suggestions, please submit issues or discussions on GitHub or Gitee.

## Build

please refer to [DEVELOPNOTE.md](./DEVELOPNOTE.md).

## Contributing

If you have suggestions or ideas for this project, please submit issues and pull requests on GitHub or Gitee.

If you want to improve/fix the known issues, you can check [TODO](./TODO.md).

The current project is recommended to use version Qt6.5.0 and above.

## Special

The project is currently developed in my spare time. In order to improve the development efficiency, this project uses GitHub Copilot to assist in code writing. The readability of some code may not be very good, and the author will try to optimize it in subsequent versions.

## Acknowledgements

The code of this project references or partially refers to or depends on the following open source projects. The project fully respects the open source agreement of the original project and would like to express its gratitude.

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
- [qxymodem](https://github.com/QQxiaoming/qxymodem)
- [qzmodem](https://github.com/QQxiaoming/qzmodem)
- [Kermit-Protocol](https://github.com/tazlauanubianca/Kermit-Protocol)
- [QSourceHighlite](https://github.com/Waqar144/QSourceHighlite)

## Star History

<a href="https://star-history.com/#QQxiaoming/quardCRT&Date">
 <picture>
   <source media="(prefers-color-scheme: dark)" srcset="https://api.star-history.com/svg?repos=QQxiaoming/quardCRT&type=Date&theme=dark" />
   <source media="(prefers-color-scheme: light)" srcset="https://api.star-history.com/svg?repos=QQxiaoming/quardCRT&type=Date" />
   <img alt="Star History Chart" src="https://api.star-history.com/svg?repos=QQxiaoming/quardCRT&type=Date" />
 </picture>
</a>
