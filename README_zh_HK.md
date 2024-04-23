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
        <img src="https://get.microsoft.com/images/zh-tw%20dark.svg" width="200"/>
    </a>
</div>

# quardCRT

[🇺🇸 English](./README.md) | [🇨🇳 简体中文](./README_zh_CN.md) | 🇭🇰 繁體中文 | [🇯🇵 日本語](./README_ja_JP.md)

quardCRT一款多功能終端模擬/圖形桌面軟體，支援多種後端協議，無依賴跨平台使用，windows/linux/mac使用體驗完全一致，支援多標籤頁和歷史記錄管理等傳統終端軟體功能，同時支援一些獨具特色的細節功能。quardCRT的設計宗旨是創建盡可能用戶友好、功能豐富、且跨平台一致性體驗的終端軟體，相比很多專業高性能終端，quardCRT會更適合入門、輕度用戶快速的配置好所需的終端環境，但這也並不意味quardCRT不追求高性能。

| ![img2](./docs/img/windows.png) |
| :-------------------------: |
| Windows                     |
| ![img1](./docs/img/macos.png)   |
| MacOS                       |
| ![img3](./docs/img/linux.png)   |
| Linux                       |

亮/暗主題切換：

| ![img4](./docs/img/dark.png)    | ![img5](./docs/img/light.png)   |
| :-------------------------: | :-------------------------: |
| 暗主題                      | 亮主題                      |

協議選擇界面：

![img](./docs/img/img.png)

## 功能描述

### 目前支持的終端協議

- ssh
- telnet (支援帶websocket封裝)
- serial
- loaclshell
- rawsocket
- windows:NamedPipe（linux/macos:unix domain socket）

### 目前支持的圖形桌面協議

- vnc

### 基本功能

- 會話記錄管理
- 多標籤頁管理，標籤頁克隆，標籤頁拖拽排序
- 雙列分屏顯示，標籤頁自由拖拽至分列
- 終端樣式配置（配色方案，字體）
- HEX顯示
- 終端背景圖片配置
- 終端滾動行數設置
- x\y\zmodem協議支持
- 支持深色/淺色主題
- UI支持多語言（簡體中文/繁體中文/英語/日語/韓語/西班牙語/法語/俄語/德語/葡萄牙語(巴西)/捷克語/阿拉伯語）

### 特色功能 （帶視頻演示，需要前往github查看）

| 標籤頁懸浮預覽 |
| :------------: |
| <video src="https://github.com/QQxiaoming/quardCRT/assets/27486515/5ecc8560-94ed-4d6e-90c4-7c18cce0db63"></video> |
| 浮動視窗支持，可將標籤頁拖曳至浮動窗口 |
| <video src="https://github.com/QQxiaoming/quardCRT/assets/27486515/15a9401a-5302-44c4-a693-e5e3043d8ca6"></video> |
| SSH2會話一鍵開啟SFTP檔案傳輸視窗 |
| <video src="https://github.com/QQxiaoming/quardCRT/assets/27486515/3097025c-3279-4c5b-b5fe-166607211dd9"></video> |
| 本地終端工作目錄書籤 |
| <video src="https://github.com/QQxiaoming/quardCRT/assets/27486515/82a2866e-cf45-4933-8638-777f6baff682"></video> |
| 自動化發送 |
| <video src="https://github.com/QQxiaoming/quardCRT/assets/27486515/59d05a59-31fd-4133-8dbe-deb43122fe8c"></video> |
| 終端機背景圖片支援gif動畫和視頻 |
| <video src="https://github.com/QQxiaoming/quardCRT/assets/27486515/4fdb10c3-b754-4b1c-8dc3-bbe83d3b0fcf"></video> |
| 終端機關鍵詞高亮匹配 |
| <video src="https://github.com/QQxiaoming/quardCRT/assets/27486515/2d136273-8a53-4d4c-9cae-4609600bd32a"></video> |
| 選中文本翻譯功能 |
| <video src="https://github.com/QQxiaoming/quardCRT/assets/27486515/aae7a324-808f-45e5-a86d-579d9002b28b"></video> |
| 路徑匹配與一鍵直達 |
| <video src="https://github.com/QQxiaoming/quardCRT/assets/27486515/4aea9223-babd-4715-a908-56227ba04fed"></video> |
| 工作路徑直達 |
| <video src="https://github.com/QQxiaoming/quardCRT/assets/27486515/616c1c09-ffa1-428d-b897-1966ecb7517b"></video> |
| windows本地端增強（Tab鍵選擇補全指令等） |
| <video src="https://github.com/QQxiaoming/quardCRT/assets/27486515/f60eed90-5c77-48c9-9c14-71f4a3d993b9"></video> |

## 計劃中特性

- [ ] 支持操作腳本錄制/加載
- [ ] 支持終端顯示錄制
- [ ] 會話狀態查詢
- [ ] 終端樣式自定義
- [ ] 獨立會話設置終端外觀
- [ ] GitHub Copilot插件支持
- [ ] CI支持windows on arm64

## 翻譯

quardCRT支持多語言，目前支持以下語言，翻譯覆蓋率如下：

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

quardCRT的翻譯由github copilot協助翻譯，翻譯可能不是很準確，如果您發現翻譯有問題，歡迎提交issue或pull request。

## Plugin

quardCRT將從V0.4.0版本開始支持Plugin，以Qt Plug-in的形式提供，以動態庫的形式加載，想了解更多Plugin開發信息請參考Plugin開放平臺[https://github.com/QuardCRT-platform](https://github.com/QuardCRT-platform)，此平臺將提供Plugin開發的模板倉庫以及相關示例。目前Plugin功能仍處於早期開發階段，如果您有好的想法或建議，歡迎在GitHub或Gitee上提交issue或discussion。

## 編譯說明

請參考[開發筆記](./DEVELOPNOTE.md)。

## 貢獻

如果您對本項目有建議或想法，歡迎在GitHub或Gitee上提交issue和pull requests。

如果您希望改進/修復目前已知的問題，您可以查看[TODO](./TODO.md)。

目前項目建議使用版本Qt6.5.0及更高版本。

## 特別

項目目前為個人業餘時間開發，為提高開發效率，本項目較為大量的使用了GitHub Copilot協助程式碼編寫，部分程式碼的人類可讀性可能不是很好，作者也會盡量在後續版本中進行最佳化。

## 感謝

本項目程式碼引用或部份參考或依賴了以下開源項目，項目完全尊重原項目開源協議，並在此表示感謝。

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
