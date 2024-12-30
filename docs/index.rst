.. raw:: html

   <div style="text-align: right"><a href="../../en/latest/index.html">🇺🇸 English</a> | <a href="../../zh-cn/latest/index.html">🇨🇳 简体中文</a> | <a href="../../zh-tw/latest/index.html">🇭🇰 繁體中文</a> | <a href="../../ja/latest/index.html">🇯🇵 日本語</a></div>

quardCRT
----------------------------------

.. image:: https://img.shields.io/github/actions/workflow/status/qqxiaoming/quardCRT/windows.yml?branch=main&logo=data:image/svg+xml;base64,PHN2ZyByb2xlPSJpbWciIHZpZXdCb3g9IjAgMCAyNCAyNCIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj48dGl0bGU+V2luZG93czwvdGl0bGU+PHBhdGggZD0iTTAsMEgxMS4zNzdWMTEuMzcySDBaTTEyLjYyMywwSDI0VjExLjM3MkgxMi42MjNaTTAsMTIuNjIzSDExLjM3N1YyNEgwWm0xMi42MjMsMEgyNFYyNEgxMi42MjMiIGZpbGw9IiNmZmZmZmYiLz48L3N2Zz4=
   :target: https://github.com/QQxiaoming/quardCRT/actions/workflows/windows.yml
   :alt: Windows ci
.. image:: https://img.shields.io/github/actions/workflow/status/qqxiaoming/quardCRT/linux.yml?branch=main&logo=linux&logoColor=white
   :target: https://github.com/QQxiaoming/quardCRT/actions/workflows/linux.yml
   :alt: Linux ci
.. image:: https://img.shields.io/github/actions/workflow/status/qqxiaoming/quardCRT/macos_arm64.yml?branch=main&logo=apple
   :target: https://github.com/QQxiaoming/quardCRT/actions/workflows/macos_arm64.yml
   :alt: Macos ci
.. image:: https://img.shields.io/codefactor/grade/github/qqxiaoming/quardCRT.svg?logo=codefactor
   :target: https://www.codefactor.io/repository/github/qqxiaoming/quardCRT
   :alt: CodeFactor
.. image:: https://img.shields.io/readthedocs/quardcrt.svg?logo=readthedocs
   :target: https://quardcrt.readthedocs.io/en/latest/?badge=latest
   :alt: Documentation Status
.. image:: https://img.shields.io/github/license/qqxiaoming/quardCRT.svg?colorB=f48041&logo=gnu
   :target: https://github.com/QQxiaoming/quardCRT
   :alt: License
.. image:: https://img.shields.io/github/v/tag/QQxiaoming/quardCRT?filter=V*&logo=git
   :target: https://github.com/QQxiaoming/quardCRT/releases
   :alt: GitHub tag (latest SemVer)
.. image:: https://img.shields.io/github/downloads/QQxiaoming/quardCRT/total.svg?logo=pinboard
   :target: https://github.com/QQxiaoming/quardCRT/releases
   :alt: GitHub All Releases
.. image:: https://img.shields.io/github/stars/QQxiaoming/quardCRT.svg?logo=github
   :target: https://github.com/QQxiaoming/quardCRT
   :alt: GitHub stars
.. image:: https://img.shields.io/github/forks/QQxiaoming/quardCRT.svg?logo=github
   :target: https://github.com/QQxiaoming/quardCRT
   :alt: GitHub forks
.. image:: https://gitee.com/QQxiaoming/quardCRT/badge/star.svg?theme=dark
   :target: https://gitee.com/QQxiaoming/quardCRT
   :alt: Gitee stars
.. image:: https://gitee.com/QQxiaoming/quardCRT/badge/fork.svg?theme=dark
   :target: https://gitee.com/QQxiaoming/quardCRT
   :alt: Gitee forks

.. image:: ./img/social_preview.jpg
   :align: center
   :alt: quardCRT

quardCRT is a terminal emulation and remote desktop software that supports multiple backend protocols, can be used across platforms without dependencies, and has a completely consistent user experience on windows/linux/mac. It supports traditional terminal software functions such as multi-tab and history management, and supports some unique detailed functions. The design purpose of quardCRT is to create a terminal software that is as user-friendly, feature-rich, and cross-platform consistent as possible. Compared to many professional high-performance terminals, quardCRT will be more suitable for beginners and light users to quickly configure the required terminal environment, but this does not mean that quardCRT does not pursue high performance.

.. list-table:: 
   :widths: 33 33 33
   :header-rows: 0

   * - .. image:: ./img/windows.png
          :align: center
          :height: 160px
     - .. image:: ./img/macos.png
          :align: center
          :height: 160px
     - .. image:: ./img/linux.png
          :align: center
          :height: 160px
   * - Windows
     - MacOS
     - Linux

----------------------------------
Features
----------------------------------

- **Cross-platform**: Windows, MacOS, Linux
- **Multiple protocols**: SSH, Telnet, Serial, LocalShell, RawSocket, NamedPipe, VNC
- **Multiple sessions**: Multi-tab, multi-window, multi-monitor, floating window
- **Multiple languages**: English, Simple Chinese, Traditional Chinese, Japanese, Korean, Spanish, French, Russian, German, Portuguese (Brazil), Czech, Arabic
- **Multiple themes**: Light, Dark
- **Session history management**: Session history management, session history search
- **Session management**: Session management, session import and export
- **HEX display**: HEX display
- **File transfer**: SFTP, Xmodem, Ymodem, Zmodem, Kermit
- **Script**: Script recording, script playback
- **Terminal customization**: Terminal font, color, size, cursor, scrollback, background, etc.

----------------------------------
Special functions
----------------------------------

- Tab floating preview
- Floating window support, tab drag-and-drop to floating window
- SSH2 session one-click open SFTP file transfer window
- Working directory bookmark
- Automated sending
- Terminal background image supports gif animation and video
- Terminal keyword highlight matching
- Selected text translation function
- Path matching and one-click direct
- Working path direct
- Windows local terminal enhancement (Tab key to select complete command, etc.)
- Broadcast session
- Session label tag color
- block selection (Shift+click) and column selection (Alt+Shift+click)

----------------------------------
Plugin
----------------------------------

quardCRT supports plugins from version V0.4.0, plugins will be provided in the form of Qt plugins, loaded in the form of dynamic libraries, to learn more about plugin development information, please refer to the plugin open `platform <https://github.com/QuardCRT-platform>`_. This platform will provide template repositories and related examples for plugin development. At present, the plugin function is still in the early development stage. If you have good ideas or suggestions, please submit issues or discussions on `GitHub <https://github.com/QQxiaoming/quardCRT>`_ or `Gitee <https://gitee.com/QQxiaoming/quardCRT>`_

----------------------------------
Install from store
----------------------------------

- .. image:: https://get.microsoft.com/images/en-us%20dark.svg
   :target: https://apps.microsoft.com/detail/quardCRT/9p6102k9qb3t?mode=direct
   :alt: Microsoft Store

- .. image:: https://www.spark-app.store/assets/favicon-96x96-BB0Q9LsV.png
   :target: spk://store/development/quardcrt
   :alt: Spark Store
   Spark Store

----------------------------------
Donate
----------------------------------

If you like this project, you can donate to the author to support the development of the project.

.. list-table:: 
   :widths: 33 33 33
   :header-rows: 0

   * - .. image:: ./img/donate/paypal.jpg
          :align: center
     - .. image:: ./img/donate/alipay.jpg
          :align: center
     - .. image:: ./img/donate/wechat.jpg
          :align: center
   * - paypal
     - alipay
     - wechat

.. toctree::
   :maxdepth: 3
   :caption: Contents:

   Installation<installation.md>
   Usage<usage.md>
   Configuration<configuration.md>
   Scripts<scripts.md>
   Plugins<plugins.md>
   FAQ<faq.md>
   Contributing<contributing.md>
   Changelog<changelog.md>
   License<license.md>
   Roadmap<roadmap.md>
   Acknowledgements<acknowledgements.md>
   Privacy<privacy.md>
 