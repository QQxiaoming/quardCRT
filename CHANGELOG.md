# Change Log

## [[Unrelease](https://github.com/QQxiaoming/quardCRT)]

- 增加windows上NamedPipe协议支持（linux/macos上对应unix domain socket）
- 修复windows上无法启动打印机服务问题
- 修复关闭标签页部份内存未释放问题
- 增加设置终端字体功能
- 增加设置终端滚动行数设置
- 增加设置光标形状和闪烁功能
- 终端背景支持Gif动画格式文件（需要在高级设置中启用动画支持）
- 终端背景支持mp4/avi/mkv/mov视频格式文件（需要在高级设置中启用动画支持）
- 更新全局设置界面，分类显示设置项
- 增加会话设置界面
- 增加分离启动新窗口功能
- 增加软件自身调试信息日志系统，用户默认上不打开的，必须手写config文件才会启动

## [[V0.2.3](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.2.3)] - 2023-10-26

- 修复错误ci环境，与V0.2.2无实质改动

## [[V0.2.2](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.2.2)] - 2023-10-26

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

- linux打包fcit插件以支持更多中文输入法
- linux修复中文字符渲染光标错误
- 修复HEX视图格式渲染错误和数据获取失败问题
- 持久化存储会话信息

## [[V0.2.0](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.2.0)] - 2023-10-18

- 增加使用alt+n/alt+J切换简约/标准UI
- 增加启动tfftp服务器实用功能
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

- 增加终端右侧滚动条
- 修复window/mac上鼠标中键不能执行复制并粘贴
- 增加双终端会话分屏显示
- 增加简约UI界面选项

## [[V0.1.4](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.1.4)] - 2023-10-08

- 增加侧边栏会话管理器
- 增加状态栏提示信息
- 增加命令输入栏
- 完善快捷键功能
- 修复会话克隆存在工作路径不一致问题
- 增加终端URL内容动态解析
- 修复可能的崩溃问题

## [[V0.1.3](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.1.3)] - 2023-09-29

- 再修复windows存在无法启动问题（V0.1.0引入）（这个版本彻底修复了V0.1.0问题）

## [[V0.1.2](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.1.2)] - 2023-09-29

- 再修复windows存在无法启动问题（V0.1.0引入）

## [[V0.1.1](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.1.1)] - 2023-09-29

- 修复windows存在无法启动问题（V0.1.0引入）

## [[V0.1.0](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.1.0)] - 2023-09-29

- 标签卡名称可双击切换长/短名称
- 增加存储log功能
- 增加快速打开本地Shell快捷键和克隆会话快捷键
- 增加windows下感知当前实时工作路径（像linux和mac一样的体验）
- 增加HEX查看器

## [[V0.0.2](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.0.2)] - 2023-09-27

- 主界面增加菜单栏、工具栏，完善更多右键菜单
- 增加更多终端界面配色风格
- 增加动态切换语言功能
- 增加动态切换主题功能

## [[V0.0.1](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.0.1)] - 2023-09-26

- 实现主界面终端功能
- 增加标签卡分页管理
- 支持telnet协议
- 支持串口协议
- 支持RAW协议
- 支持本地Shell
- 界面支持中文/英文/日文
