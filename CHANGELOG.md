# Eftp 代码修改历史

## 2026-06-09

### 架构重构
- 目录结构调整：`src/core/`、`src/EftpClient/`、`src/app/`、`third_party/`
- 第三方库统一放到 `third_party/`（JsonHpp、cpp-httplib、OpenSSL）

### 基础设施层（src/core/）
- **QtLogger** — 日志模块，从原项目迁移，移除不必要的 Qt5::Sql/Xml 依赖
- **HttpClient** — HTTP 客户端，从 HttpCilent 重命名，移除 UploadFile，统一重试逻辑（每 3 秒无限重试）
- **ConfigManager** — 配置管理单例，支持分层配置（app.json + 环境文件夹覆盖）+ INI 配置（dll.ini）
- **DownloadManager** — 合并原项目 DownloadFiles + DumpUploader，支持下载/上传/蓝屏 Dump 检测

### 数据模型层（src/EftpClient/）
- **EftpTypes.h** — 统一数据结构定义（API 层 std::string + UI 层 QString）
- **EftpJsonParser** — JSON 解析工具

### 业务模块层（src/EftpClient/）
- **SerApiModel** — 19 个 REST API 端点封装，POST+查询参数方式
- **DeviceModel** — PowerShell WMI 采集硬件信息
- **NetworkMonitor** — 定时 ping + WiFi 自动切换
- **TestOrchestrator** — 测试流程状态机 + 后台版本检查
- **SystemTrayManager** — 系统托盘管理器（ITrayAction 接口模式）

### UI 层（src/app/）
- **MainWindow** — 主窗口，信号路由中心
- **DeviceInfoPanel** — 设备信息面板（.ui 文件）
- **EDYTabBar** — 自绘 Tab 栏（状态图标 + 虚线连接）
- **EftpTabWidget** — Tab 容器（EDYTabBar + QTabWidget）
- **InitialPage** — 初始化页面（.ui 文件，支持重试）
- **StageTestPage** — 阶段测试页面（.ui 文件，8 列网格）
- **ItemProgramTestModel/Delegate** — 测试项网格 Model/Delegate（背景色状态）
- **ItemDetailInfoDialog** — 测试项详情对话框（.ui 文件）
- **SettingsDialog** — 设置对话框（.ui 文件，WiFi 配置 + 环境切换）
- **ErrorDialog** — 统一错误/警告/信息弹窗（.ui 文件）
- **SNMacBindDialog** — SN/MAC 绑定对话框（.ui 文件）
- **EDYLabel** — 双击复制标签

### 配置系统
- 分层配置：`configs/app.json` + `configs/{env}/app.json`（环境覆盖）
- 环境检测：exe 目录下文件名（product/pre/test）
- DLL 路径配置：`configs/dll.ini`（按环境分组）

### 加载器
- **EftpLauncher** — 无依赖加载器，读取 dll.ini 设置 DLL 搜索路径后启动 EFTP.exe

### 删除的模块
- DumpUploader — 合并到 DownloadManager
- VersionChecker — 合并到 TestOrchestrator

