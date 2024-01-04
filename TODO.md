# TODO

- [ ] mainwidgetgroup需要重构以支持无限数量的tab group
- [ ] sessionswindow需要重构将基础的共用功能抽象出基类，不同协议继承派生，提高代码可读性
- [ ] sftpwindow中需要改进性能，目前在文件夹中有大量文件时会卡顿，另外需要改进目录/文件多选功能
- [ ] sessionoptions需要增加更多选项配置以供用户调整
- [ ] globaloptions右侧的选项树需要支持多级选项
- [ ] qtermwidget在某些字体下渲染有问题，需要改进 [#I8RB90](https://gitee.com/QQxiaoming/quardCRT/issues/I8RB90)
- [x] sessionmanagerwidget的过滤功能需要完善
- [ ] plugin功能需要增加用户自定义插件的目录并提供界面安装/卸载插件
