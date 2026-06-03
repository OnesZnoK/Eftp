# Eftp 重构待办事项

## 已完成
- [x] 搭建项目骨架（CMake 多模块结构）
- [x] QtLogger — 日志工具库
- [x] JsonHpp — JSON解析库
- [x] HttpCilent — HTTP请求封装（GET/POST/UploadFile）
- [x] ConfigManager — 配置管理（app.json + wifi.json）
- [x] DeviceModel — 设备信息采集（SN/MAC/主板SN）
- [x] SerApiModel — REST API 封装（19/19个接口，全部完成）
- [x] MainWindow — 空壳调度器
- [x] 整理客户端接口文档（19个接口）
- [x] SerApiModel 结构体字段按实际接口修正
- [x] SerApiModel 精简（doGet/doPost 不加重试和日志，委托 HttpClient）
- [x] HttpClient 新增 UploadFile（multipart/form-data）

## 待实现 — 业务模块（EftpClient 内部）
- [ ] DownloadManager — 文件下载/7z解压/版本校验
- [ ] TestOrchestrator — 测试流程状态机
- [ ] NetworkMonitor — WiFi自动管理/网络检测
- [ ] DumpUploader — 蓝屏Dump检测与上传

## 待实现 — Widget 层
- [ ] StageTabBar — 流程Tab控件
- [ ] InitialPage — 初始化页面
- [ ] StageTestPage — 单阶段测试页面
- [ ] ItemDetailDialog — 测试项详情弹窗
- [ ] SNMacBindDialog — SN/MAC绑定弹窗
- [ ] ErrorViewer — 错误信息查看器
- [ ] Toast — 通知提示

## 待实现 — Delegate 层
- [ ] ItemGridModel/Delegate — 测试项网格
- [ ] DetailTableModel/Delegate — 详情表格


## 待实现 — MainWindow 调度器
- [ ] 实现信号路由（模块间通信中转）
- [ ] Tab 切换/页面堆叠管理
- [ ] 自定义标题栏/无边框窗口


