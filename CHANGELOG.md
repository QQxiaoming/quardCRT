# Change Log

## [[Unrelease](https://github.com/QQxiaoming/quardCRT)]

- 主UI全面更新，整体风格更现代化，配套亮色/暗色主题更加美观
- 增加设置选项里打开当前设置文件功能
- 右上角增加实验室功能，SSH扫描迁移至实验室，未来将会开放插件接口，增加更多特定功能到实验室，访问[插件平台](https://github.com/QuardCRT-platform)
- SFTP窗口增加书签功能，改进SFTP操作逻辑。
- 改进优化HexView窗口
- 改进会话克隆功能，可克隆的会话类型（SSH、Telent等）直接克隆，不可克隆的会话类型（串口等）会弹出会话设置窗口
- 改进在拖拽标签时样式，现在鼠标能正确显示为抓握手势
- 修复打开会话属性存在历史页面未刷新
- 修复会话内line字符渲染错误
- 修复打开SSH会话后未变化过窗口大小的情况下，默认的终端大小不正确
- 修复切换主题时Tab Add New按钮样式未更新

## [[V0.3.1](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.3.1)] - 2023-12-05

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

- 增加SSH2协议支持（目前仅支持密码认证，用户密码存储在本地系统密钥链中，不存储在本软件配置文件中）
- 增加SSH2会话打开SFTP文件传输窗口功能
- 增加自定义设置单词字符（Word Characters）功能，可以设置哪些字符为视为单词字符，方便快速选中单词
- 增加双击会话管理器内会话进行连接功能
- 增加会话管理器关闭按钮
- 修复修改会话属性后会话管理器内会话排序重新排列问题
- 修复串口连接正常但弹出错误提示“No Error”问题
- 修复连接失败的会话无法关闭问题

## [[V0.2.6](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.2.6)] - 2023-11-15

- 增加标签卡悬浮预览功能
- windows本地终端增强，现在可以像linux一样使用Tab键选择补全命令
- 增加会话状态查询，通过会话设置/属性-状态查看会话状态，目前仅支持本地Shell会话查询进程信息
- 终端内容匹配支持路径匹配，可以右键快速打开文件/目录
- 增加高亮显示匹配内容功能
- 修复中文全角引号渲染位置错误问题
- 修复macOS错误绑定了复制粘贴快捷键占用导致无法kill终端内进程问题
- 增加更多语言支持（由GitHub Copilot提供）

## [[V0.2.5](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.2.5)] - 2023-11-09

- 增加浮动窗口模式，且支持移回主窗口
- 增加标签页自由拖拽（拖拽成浮动窗口或拖拽至分屏）
- 修复移动标签后可能导致的严重崩溃问题/标签标题显示错误问题
- 增加高级设置选择UI样式为原生风格（单平台用户可能希望使用原生风格，多平台用户可能希望使用统一风格）
- 增加设置中一键清理选择的背景图片功能
- 优化退出应用/关闭会话时的二次确认功能，现在本地Shell会话根据是否有子进程决定是否需要二次确认，其他会话类型不变
- 增加设置标签标题模式，可以选择以简要/完整/滚动，三种方式显示会话标题

## [[V0.2.4](https://github.com/QQxiaoming/quardCRT/releases/tag/V0.2.4)] - 2023-11-03

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
