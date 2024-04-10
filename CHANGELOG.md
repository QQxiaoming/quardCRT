# Change Log

## [[Unrelease](https://github.com/QQxiaoming/quardCRT)]

en-US:

- Modify the terminal to select the text after the text is emphasized with a transparency of 50%, rather than the original 100%
- Fix the issue of abnormal width of rendering symbols '×' '÷' and '‖'
- Fix the issue of program crash in a small probability situation
- Fix cursor positioning issues
- Remove the dependency on core5compat in Qt (Refer to the pr upstream of qtermwidget, but with modifications).
- Prohibit to look up the historical information with the middle mouse scrolling (Refer to the pr upstream of qtermwidget, but with modifications).
- Fix rendering issue with abnormal width of some symbols
- Add ANSI OSC52 sequence support
- Fix issue that disconnecting the session will directly close the tab instead of disconnecting the session

zh-CN:

- 修改终端选中后文本强调色透明度50%，而非原本的100%
- 修复渲染符号'×' '÷' '‖'宽度异常问题
- 修复存在一小概率情况下程序崩溃的问题
- 修复光标定位问题 [#I8RB90](https://gitee.com/QQxiaoming/quardCRT/issues/I8RB90)
- 删除对Qt中core5compat的依赖（参考qtermwidget上游pr，但有修改）
- 禁止使用中键滚动查看历史信息（参考qtermwidget上游pr，但有修改）
- 修复渲染某些符号宽度异常问题
- 增加 ANSI OSC52 sequence 支持
- 修复断开会话会直接关闭标签而不是断开会话的问题

## [[V0.4.4](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.4.4)] - 2024-03-26

en-US:

- Add ascii send/receive, Kermit send/receive, xyzmodem send/receive functions
- Fix the problem that the status indicated by the start tftp menu item may be incorrect
- Fix the problem that the program crashes when the terminal displays Unicode characters beyond the first plane
- Fix the problem that the terminal area rendering does not refresh after drag the window to the screen edge to trigger the window size adjustment on linux

zh-CN:

- 增加ascii发送/接收、Kermit发送/接收、xyzmodem发送/接收功能
- 修复启动tftp菜单项指示的状态可能出现错误问题
- 修复终端显示超过第一平面的Unicode字符时程序崩溃的问题
- 修复linux上拖拽窗口到屏幕边缘触发窗口大小调整后终端界面渲染不刷新的问题

## [[V0.4.3](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.4.3)] - 2024-02-26

en-US:

- Add a tooltip to the terminal link, and modify the mouse shape to the corresponding shape after pressing ctrl
- Modify the default LC_CTYPE configuration to UTF-8 on macos, and do not make default configuration on other platforms. You can modify whether to make configuration through the setting file
- Fix the problem that the pre-built version on macos missed part of the packaged translation files
- Fix the problem that the terminal area rendering is not refreshed in time when the window is maximized on linux
- The plugin system completes multi-language support
- Partial UI details beautification
- Add a new help document
- Add German/Portuguese (Brazil)/Czech/Arabic support

zh-CN:

- 终端链接增加tooltip，以及按下ctrl后鼠标形状修改对应形状
- 修改macos上默认LC_CTYPE配置为UTF-8，其他平台不做默认配置，可通过setting文件修改是否做配置
- 修复macos上预编译的版本遗漏打包部份翻译文件问题
- 修复linux上最大化窗口终端界面渲染未及时刷新问题
- 插件系统完成多语言支持
- 部分UI细节美化
- 新增一个新的帮助文档界面
- 增加德语/葡萄牙语(巴西)/捷克语/阿拉伯语支持

## [[V0.4.2](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.4.2)] - 2024-01-28

en-US:

- Add ESC to exit full screen mode in full screen mode
- The plugin platform supports dynamic enable/disable plugin functions
- Optimize and solve the problem of fixed CPU load after software startup
- Optimize the problem of incorrect cursor position when searching up and down the history command in the local terminal under windows
- Fix the possible error ack processing problem of tftpsever
- Fix the crash problem under full screen on macos
- Fix the title button does not switch to the macos style problem in the native UI mode on macos
- Fix the problem that the title button full screen on macos through the native UI style cannot display the exit full screen option in the context menu on the main window
- Pre-built versions add pre-built plugin packaging for the [plugin ecosystem platform](https://github.com/QuardCRT-platform)

zh-CN:

- 全屏模式下增加ESC退出全屏功能
- 插件平台支持动态启用/禁用插件功能
- 优化解决软件启动后固定占用CPU负载问题
- 优化windows下本地终端上下搜索历史命令时光标位置错误问题
- 修复tftpsever可能的错误ack处理问题
- 修复macos下全屏导致程序崩溃问题
- 修复macos native UI模式下标题按钮没有切换为macos风格问题
- 修复macos通过native UI样式标题按钮全屏后，无法显示界面上上下文菜单中退出全屏选项问题
- 预构建版本增加对于[插件生态平台](https://github.com/QuardCRT-platform)的预构建插件的打包

## [[V0.4.1](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.4.1)] - 2024-01-13

en-US:

- Add the function of opening a session in a new window in the session manager
- Add the setting label tab plus button mode optional for new session/clone session/local shell session three ways
- Add filtering function in session manager
- Fix the window flashing problem caused by the preview window when dragging the label

zh-CN:

- 增加会话管理器中在新窗口打开会话功能
- 增加设置标签页加号按钮位模式可选为 新建会话/克隆会话/本地Shell会话 三种方式
- 增加会话管理器中过滤功能
- 修复在拖拽标签时预览窗口存在导致的窗口闪烁问题

## [[V0.4.0](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.4.0)] - 2023-12-20

en-US:

- The main UI is fully updated, the overall style is more modern, and the matching light/dark theme is more beautiful
- Add the function of opening the current setting file in the setting option
- Add the experimental function in the upper right corner, and the SSH scan is migrated to the laboratory, and the plugin interface is opened to add more specific functions to the laboratory, visit [plugin platform](https://github.com/QuardCRT-platform)
- The SFTP window adds a bookmark function and improves the SFTP operation logic.
- Improve and optimize the HexView window
- Improve the session cloning function, the session type that can be cloned (SSH, Telent, etc.) is directly cloned, and the session type that cannot be cloned (serial port, etc.) will pop up the session setting window
- Improve the style when dragging the label, now the mouse can be correctly displayed as a grip gesture
- Fix the problem that the history page is not refreshed when opening the session properties
- Fix the rendering error of the line character in the session
- Fix the default terminal size is incorrect when opening the SSH session without changing the window size
- Fix the Tab Add New button style is not updated when switching the theme
- Fix the problem that the terminal color scheme cannot be persistently saved after modification [#I8PLTP](https://gitee.com/QQxiaoming/quardCRT/issues/I8PLTP)

zh-CN:

- 主UI全面更新，整体风格更现代化，配套亮色/暗色主题更加美观
- 增加设置选项里打开当前设置文件功能
- 右上角增加实验室功能，SSH扫描迁移至实验室，开放插件接口以增加更多特定功能到实验室，访问[插件平台](https://github.com/QuardCRT-platform)
- SFTP窗口增加书签功能，改进SFTP操作逻辑。
- 改进优化HexView窗口
- 改进会话克隆功能，可克隆的会话类型（SSH、Telent等）直接克隆，不可克隆的会话类型（串口等）会弹出会话设置窗口
- 改进在拖拽标签时样式，现在鼠标能正确显示为抓握手势
- 修复打开会话属性存在历史页面未刷新
- 修复会话内line字符渲染错误
- 修复打开SSH会话后未变化过窗口大小的情况下，默认的终端大小不正确
- 修复切换主题时Tab Add New按钮样式未更新
- 修复终端配色方案修改后无法持久保存 [#I8PLTP](https://gitee.com/QQxiaoming/quardCRT/issues/I8PLTP)

## [[V0.3.1](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.3.1)] - 2023-12-05

en-US:

- Add password input box support display/hide function
- Add common baud rate prompt
- Add right-click menu request translation function
- Add practical tools, scan local network SSH service
- SFTP file transfer window adds transfer progress display
- Add vnc protocol support
- Fix the problem that the port information of the serial port session cannot be correctly connected after being stored
- Fix the problem that the keychain authorization fails to use the wrong information
- Fix the problem that the maximum value of the port number input box is incorrect, resulting in the inability to correctly save the session information

zh-CN:

- 增加密码输入框支持显示/隐藏功能
- 增加常用波特率提示
- 增加右键菜单请求翻译功能
- 增加实用工具，扫描本地网络SSH服务
- SFTP文件传输窗口增加传输进度显示
- 增加vnc协议支持
- 修复二次编辑存储串口会话port信息后无法正确连接问题
- 修复钥匙串授权失败使用错误信息的问题
- 修复端口号输入框最大值错误导致的无法正确保存会话信息问题

## [[V0.3.0](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.3.0)] - 2023-11-28

en-US:

- Add SSH2 protocol support (currently only supports password authentication, user passwords are stored in the local system keychain, not stored in the software configuration file)
- Add SSH2 session open SFTP file transfer window function
- Add custom setting word characters function, you can set which characters are considered as word characters, which is convenient for quickly selecting words
- Add double-click session management session to connect function
- Add session manager close button
- Fix the problem that the session manager session sorting is rearranged after modifying the session properties
- Fix the problem that the serial port connection is normal but an error prompt "No Error" pops up
- Fix the problem that the session that failed to connect cannot be closed

zh-CN:

- 增加SSH2协议支持（目前仅支持密码认证，用户密码存储在本地系统密钥链中，不存储在本软件配置文件中）
- 增加SSH2会话打开SFTP文件传输窗口功能
- 增加自定义设置单词字符（Word Characters）功能，可以设置哪些字符为视为单词字符，方便快速选中单词
- 增加双击会话管理器内会话进行连接功能
- 增加会话管理器关闭按钮
- 修复修改会话属性后会话管理器内会话排序重新排列问题
- 修复串口连接正常但弹出错误提示“No Error”问题
- 修复连接失败的会话无法关闭问题

## [[V0.2.6](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.2.6)] - 2023-11-15

en-US:

- Add label tab hover preview function
- Windows local terminal enhancement, now you can use the Tab key to complete the command like linux
- Add session status query, view session status through session settings/properties-status, currently only support local Shell session query process information
- Terminal content matching supports path matching, you can right-click to quickly open files/directories
- Add highlight matching content function
- Fix the problem of rendering position error of Chinese full-width quotation marks
- Fix the problem that macOS incorrectly binds the copy and paste shortcut keys, causing the terminal to be unable to kill the process
- Add more language (Spanish/French/Korean/Russian/Traditional Chinese) support (provided by GitHub Copilot)

zh-CN:

- 增加标签卡悬浮预览功能
- windows本地终端增强，现在可以像linux一样使用Tab键选择补全命令
- 增加会话状态查询，通过会话设置/属性-状态查看会话状态，目前仅支持本地Shell会话查询进程信息
- 终端内容匹配支持路径匹配，可以右键快速打开文件/目录
- 增加高亮显示匹配内容功能
- 修复中文全角引号渲染位置错误问题
- 修复macOS错误绑定了复制粘贴快捷键占用导致无法kill终端内进程问题
- 增加更多语言(西班牙语/法语/韩语/俄语/繁体中文)支持（由GitHub Copilot提供）

## [[V0.2.5](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.2.5)] - 2023-11-09

en-US:

- Add floating window mode, and support moving back to the main window
- Add tab page free drag (drag to floating window or drag to split screen)
- Fix the serious crash problem caused by moving the label/label title display error problem
- Add advanced settings to select the UI style as the native style (single platform users may want to use the native style, multi-platform users may want to use the unified style)
- Add one-click cleaning of selected background images in settings
- Optimize the secondary confirmation function when exiting the application/closing the session, now the local Shell session will decide whether to require secondary confirmation based on whether there are child processes, and other session types remain unchanged
- Add setting label title mode, you can choose to display the session title in brief/complete/scrolling, three ways

zh-CN:

- 增加浮动窗口模式，且支持移回主窗口
- 增加标签页自由拖拽（拖拽成浮动窗口或拖拽至分屏）
- 修复移动标签后可能导致的严重崩溃问题/标签标题显示错误问题
- 增加高级设置选择UI样式为原生风格（单平台用户可能希望使用原生风格，多平台用户可能希望使用统一风格）
- 增加设置中一键清理选择的背景图片功能
- 优化退出应用/关闭会话时的二次确认功能，现在本地Shell会话根据是否有子进程决定是否需要二次确认，其他会话类型不变
- 增加设置标签标题模式，可以选择以简要/完整/滚动，三种方式显示会话标题

## [[V0.2.4](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.2.4)] - 2023-11-03

en-US:

- Add windows NamedPipe protocol support (unix domain socket corresponding to linux/macos)
- Fix the problem that the printer service cannot be started on windows
- Fix the problem that part of the memory is not released when closing the tab
- Add secondary confirmation function when exiting the application/closing the session
- Add session link status display function (the tab displays in the form of an icon)
- Add terminal font setting function
- Add terminal scroll line number setting
- Add setting cursor shape and blink function
- Terminal background supports Gif animation format files (need to enable animation support in advanced settings)
- Terminal background supports mp4/avi/mkv/mov video format files (need to enable animation support in advanced settings)
- Update the global settings window, categorize the settings
- Add session settings window, now you can modify and edit session properties
- Add separate start new window function
- Add software's own debugging information log system, which is not turned on by default, and must be manually written to the config file to start

zh-CN:

- 增加windows上NamedPipe协议支持（linux/macos上对应unix domain socket）
- 修复windows上无法启动打印机服务问题
- 修复关闭标签页部份内存未释放问题
- 增加退出应用/关闭会话时需要二次确认功能
- 增加会话链接状态显示功能（标签页以图标形式显示）
- 增加设置终端字体功能
- 增加设置终端滚动行数设置
- 增加设置光标形状和闪烁功能
- 终端背景支持Gif动画格式文件（需要在高级设置中启用动画支持）
- 终端背景支持mp4/avi/mkv/mov视频格式文件（需要在高级设置中启用动画支持）
- 更新全局设置界面，分类显示设置项
- 增加会话设置界面，现在可以修改编辑会话属性
- 增加分离启动新窗口功能
- 增加软件自身调试信息日志系统，用户默认上不打开的，必须手写config文件才会启动

## [[V0.2.3](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.2.3)] - 2023-10-26

en-US:

- Fix the error ci environment, no substantial changes from V0.2.2

zh-CN:

- 修复错误ci环境，与V0.2.2无实质改动

## [[V0.2.2](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.2.2)] - 2023-10-26

en-US:

- Fix the session information storage error that caused the right session manager to fail to connect correctly [#I8AJN1](https://gitee.com/QQxiaoming/quardCRT/issues/I8AJN1)
- Quick connection supports selecting only open session and only save session
- Improve bookmark management function
- Implement save settings and real-time save settings button function
- Add ALT+'-' and ALT+'=' global shortcut keys to switch the current tab
- Add ALT+'{num}' global shortcut key to switch to the specified tab
- Add ALT+LEFT, ALT+RIGHT global shortcut keys mapped to home and end keys (considering that macbook does not have home and end keys)
- Add high-definition screenshot current terminal function
- Add export current terminal session content (pdf/txt) function
- Add printer print current terminal session function
- Add lock session function
- Add configuration to start a new local terminal with the default working path
- Terminal background image supports tile mode
- Persistently store the main window layout information
- Optimize the context menu on the main window
- Optimize the open file dialog UI
- Modify the rendering bold font on windows to cause the cursor to be abnormal, temporarily not supporting bold rendering to ensure that the cursor position is correct
- Fix the problem that the working path set after executing the command on windows is incorrect
- Add cmd+delete as delete key function on macos (because the delete key is backspace on macos)

zh-CN:

- 修复会话信息保存错误导致右侧会话管理器内无法正确连接会话问题 [#I8AJN1](https://gitee.com/QQxiaoming/quardCRT/issues/I8AJN1)
- 快速连接支持选择仅打开会话和仅保存会话
- 改进书签管理功能
- 实现保存设置和实时保存设置按钮功能
- 增加ALT+'-'和ALT+'='全局快捷键切换当前标签
- 增加ALT+'{num}'全局快捷键切换到指定标签
- 增加ALT+LEFT、ALT+RIGHT全局快捷键映射到home和end按键（考虑macbook没有home和end按键）
- 增加高清截图当前终端功能
- 增加导出当前终端会话内容（pdf/txt）功能
- 增加打印机打印当前终端会话功能
- 增加锁定会话功能
- 增加配置启动新的本地终端默认的工作路径
- 终端背景图片支持平铺模式
- 持久化存储主界面布局信息
- 优化界面上下文菜单
- 优化打开文件对话框UI
- 修改windows上渲染粗体字体导致光标异常，暂时不支持粗体渲染保证光标位置正确
- 修复windows上执行命令后设置的工作路径不正确问题
- 增加macos上cmd+delete为delete按键功能（因为通常macos上delete按键为backspace）

## [[V0.2.1](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.2.1)] - 2023-10-19

en-US:

- linux package fcit plugin to support more Chinese input methods
- Fix the rendering cursor error of Chinese characters on linux
- Fix HEX view format rendering error and data acquisition failure problem
- Persistently store session information

zh-CN:

- linux打包fcit插件以支持更多中文输入法
- linux修复中文字符渲染光标错误
- 修复HEX视图格式渲染错误和数据获取失败问题
- 持久化存储会话信息

## [[V0.2.0](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.2.0)] - 2023-10-18

en-US:

- Add the function of switching simple/standard UI with alt+n/alt+J
- Add the practical function of starting tftp server
- Optimize hex view display window
- The window installation program adds the system-integrated right-click menu opened with quardCRT
- The tab bar right-click menu adds multiple practical functions
- Fix the position error of wide characters such as Chinese on window
- Add terminal background image configuration function
- Add directory bookmark function
- Add persistent storage of user settings function
- Fix some UI details error
- Fix some small probability of possible crash problems

zh-CN:

- 增加使用alt+n/alt+J切换简约/标准UI
- 增加启动tftp服务器实用功能
- 优化hex view显示界面
- window安装程序增加使用quardCRT打开的系统集成的右键菜单
- 标签栏右键菜单增加个多个实用功能
- 修复window上中文等其他宽字符光标位置错误
- 增加配置终端背景图片功能
- 增加目录书签功能
- 增加持久存储用户设置功能
- 修复了一些UI细节错误
- 修复了一些小概率的可能崩溃问题

## [[V0.1.5](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.1.5)] - 2023-10-09

en-US:

- Add terminal right scroll bar
- Fix the problem that the middle mouse button cannot execute copy and paste on window/mac
- Add dual terminal session split display
- Add simple UI options

zh-CN:

- 增加终端右侧滚动条
- 修复window/mac上鼠标中键不能执行复制并粘贴
- 增加双终端会话分屏显示
- 增加简约UI界面选项

## [[V0.1.4](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.1.4)] - 2023-10-08

en-US:

- Add sidebar session manager
- Add status bar prompt information
- Add command input bar
- Improve shortcut key function
- Fix the problem that the session clone has inconsistent working paths
- Add dynamic parsing of terminal URL content
- Fix possible crash problems

zh-CN:

- 增加侧边栏会话管理器
- 增加状态栏提示信息
- 增加命令输入栏
- 完善快捷键功能
- 修复会话克隆存在工作路径不一致问题
- 增加终端URL内容动态解析
- 修复可能的崩溃问题

## [[V0.1.3](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.1.3)] - 2023-09-29

en-US:

- Re-fix the problem that windows cannot start (introduced in V0.1.0) (this version completely fixes the problem introduced in V0.1.0)

zh-CN:

- 再修复windows存在无法启动问题（V0.1.0引入）（这个版本彻底修复了V0.1.0问题）

## [[V0.1.2](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.1.2)] - 2023-09-29

en-US:

- Re-fix the problem that windows cannot start (introduced in V0.1.0)

zh-CN:

- 再修复windows存在无法启动问题（V0.1.0引入）

## [[V0.1.1](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.1.1)] - 2023-09-29

en-US:

- Fix the problem that windows cannot start (introduced in V0.1.0)

zh-CN:

- 修复windows存在无法启动问题（V0.1.0引入）

## [[V0.1.0](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.1.0)] - 2023-09-29

en-US:

- Add the function of switching long/short names by double-clicking the label tab name
- Add storage log function
- Add quick open local Shell shortcut key and clone session shortcut key
- Add windows to perceive the current real-time working path (the same experience as linux and mac)
- Add HEX viewer

zh-CN:

- 标签卡名称可双击切换长/短名称
- 增加存储log功能
- 增加快速打开本地Shell快捷键和克隆会话快捷键
- 增加windows下感知当前实时工作路径（像linux和mac一样的体验）
- 增加HEX查看器

## [[V0.0.2](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.0.2)] - 2023-09-27

en-US:

- Add menu bar and toolbar to the main window, and improve more right-click menus
- Add more terminal theme styles
- Add dynamic switching language function
- Add dynamic switching theme function

zh-CN:

- 主界面增加菜单栏、工具栏，完善更多右键菜单
- 增加更多终端界面配色风格
- 增加动态切换语言功能
- 增加动态切换主题功能

## [[V0.0.1](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.0.1)] - 2023-09-26

en-US:

- Realize the main window terminal function
- Add label tab paging management
- Support telnet protocol
- Support serial port protocol
- Support RAW protocol
- Support local Shell
- UI supports simplified Chinese/English/Japanese

zh-CN:

- 实现主界面终端功能
- 增加标签卡分页管理
- 支持telnet协议
- 支持串口协议
- 支持RAW协议
- 支持本地Shell
- 界面支持简体中文/英文/日文
