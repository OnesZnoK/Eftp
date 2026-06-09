# EFTP

工厂产线设备自动化测试客户端，与 EFTP MES 服务器通信，管理设备测试生命周期。

## 功能概述

- 设备信息采集（BIOS SN、主板 SN、MAC 地址）
- 服务端 API 通信（19 个接口）
- 测试计划查询与执行
- 测试程序版本校验与下载
- 外部测试程序启动与结果上报
- WiFi 自动切换（按工序）
- 蓝屏 Dump 检测与上传
- 多环境支持（product/pre/test/自定义）
- 系统托盘（设置对话框）

---

## 项目结构

```
newEftp/
├── CMakeLists.txt
├── CMakeSettings.json
├── CHANGELOG.md
├── README.md
├── configs/                        # 运行时配置
│   ├── app.json                    # 基础配置（服务地址、下载参数、网络）
│   ├── wifi.json                   # WiFi 配置（默认 + 工序路由映射）
│   ├── dll.ini                     # DLL 路径配置（按环境分组）
│   ├── product/                    # product 环境覆盖
│   ├── test/                       # test 环境覆盖
│   └── pre/                        # pre 环境覆盖
│
├── third_party/                    # 第三方库
│   ├── jsonhpp/                    # nlohmann/json
│   ├── cpp-httplib/                # cpp-httplib
│   └── openssl/                    # OpenSSL 3（预编译）
│
├── src/
│   ├── core/                       # 基础设施层（SHARED DLL）
│   │   ├── QtLogger/               # 日志工具
│   │   ├── HttpClient/             # HTTP 客户端（GET/POST）
│   │   ├── ConfigManager/          # 配置管理单例
│   │   └── DownloadManager/        # 文件传输（下载/上传/Dump）
│   │
│   ├── EftpClient/                 # 业务模块集合
│   │   ├── EftpTypes.h             # 统一数据结构（INTERFACE 头文件库）
│   │   ├── EftpJsonParser/         # JSON 解析工具（STATIC）
│   │   ├── SerApiModel/            # REST API 封装（SHARED DLL）
│   │   ├── DeviceModel/            # 硬件信息采集（SHARED DLL）
│   │   ├── NetworkMonitor/         # 网络监控（SHARED DLL）
│   │   └── TestOrchestrator/       # 测试流程状态机（SHARED DLL）
│   │
│   ├── launcher/                   # 加载器（无依赖 EXE）
│   │   └── main.cpp
│   │
│   └── app/                        # 应用层（EXE）
│       ├── main.cpp
│       ├── MainWindow.h/cpp/ui
│       ├── DeviceInfoPanel.h/cpp/ui
│       ├── EftpTabWidget.h
│       ├── EDYTabBar.h/cpp
│       ├── InitialPage.h/cpp/ui
│       ├── StageTestPage.h/cpp/ui
│       ├── SettingsDialog.h/cpp/ui
│       ├── ErrorDialog.h/cpp/ui
│       ├── SNMacBindDialog.h/cpp/ui
│       ├── ItemDetailInfoDialog.h/cpp/ui
│       ├── ItemProgramTestModel.h/cpp
│       ├── ItemProgramTestDelegate.h/cpp
│       ├── ItemDetailInfoWidgetModel.h/cpp
│       ├── ItemDetailInfoWidgetDelegate.h/cpp
│       ├── resource/
│       │   ├── Eftp.qrc
│       │   ├── NBStyleSheet.qss
│       │   ├── icon/
│       │   └── image/
│       └── CMakeLists.txt
│
├── scripts/
│   └── build.bat
│
└── Output/Win32/Bin/               # 构建输出
    ├── EFTP.exe
    ├── EftpLauncher.exe
    ├── configs/
    ├── dll/core/
    ├── dll/modules/
    ├── log/
    └── platforms/
```

---

## 模块架构

```
┌─────────────────────────────────────────────────────────┐
│                    EFTP.exe (应用层)                      │
│  MainWindow + DeviceInfoPanel + EDYTabBar + Pages       │
│  SettingsDialog + ErrorDialog + SNMacBindDialog         │
└───────┬──────────┬──────────┬──────────┬───────────────┘
        │          │          │          │
   ┌────▼────┐ ┌───▼───┐ ┌───▼───┐ ┌───▼──────┐
   │EftpPages│ │Dialogs│ │Widgets│ │Delegates │  ← 编入 EXE
   └────┬────┘ └───┬───┘ └───┬───┘ └───┬──────┘
        │          │          │          │
┌───────▼──────────▼──────────▼──────────▼───────────────┐
│                 业务模块层 (SHARED DLL)                   │
│  TestOrchestrator │ DeviceModel │ SerApiModel │ Monitor │
└───────┬──────────────────────┬──────────────┬──────────┘
        │                      │              │
┌───────▼──────┐        ┌─────▼─────┐  ┌─────▼──────┐
│DownloadManager│        │EftpModels │  │EftpJsonParser│
│  (SHARED)    │        │(INTERFACE)│  │  (STATIC)   │
└───────┬──────┘        └───────────┘  └─────────────┘
        │
┌───────▼─────────────────────────────────────────────────┐
│                  基础设施层 (SHARED DLL)                   │
│  HttpClient │ ConfigManager │ QtLogger │ JsonHpp         │
└─────────────────────────────────────────────────────────┘
```

---

## 配置系统

### 分层配置

```
configs/
├── app.json              ← 基础配置
├── wifi.json             ← 基础 WiFi
├── dll.ini               ← DLL 路径配置
├── product/
│   ├── app.json          ← product 环境覆盖
│   └── wifi.json
├── test/
│   ├── app.json          ← test 环境覆盖
│   └── wifi.json
└── pre/
    ├── app.json          ← pre 环境覆盖
    └── wifi.json
```

加载顺序：`configs/app.json` → 深度合并 `configs/{env}/app.json`

### 环境检测

exe 目录下放一个空文件，文件名即为环境名：

```
EFTP.exe
product     ← 存在则环境为 product
pre         ← 存在则环境为 pre
test        ← 存在则环境为 test
```

### DLL 路径配置（dll.ini）

```ini
[product]
core=dll/core
modules=dll/modules

[test]
core=dll/core
modules=dll/modules
```

---

## 启动方式

通过 `EftpLauncher.exe` 启动：

1. 检测 exe 目录下的环境标记文件
2. 读取 `dll.ini` 获取 DLL 路径
3. 设置 DLL 搜索路径
4. 启动 `EFTP.exe`

---

## 构建

### 环境要求

- Visual Studio 2017/2019/2022（MSVC v141 工具集）
- Qt 5.12.12（msvc2017）
- CMake 3.16+

### 构建步骤

1. 用 VS 打开 `newEftp` 文件夹
2. 选择 **x86-Release** 或 **x86-Debug** 配置
3. **CMake → 全部重新生成**
4. **Ctrl+Shift+B** 构建

---

## 运行时目录结构

```
EFTP.exe
EftpLauncher.exe
product                 ← 环境标记文件
configs/
├── app.json
├── wifi.json
└── dll.ini
dll/
├── core/               ← 核心 DLL
│   ├── QtLogger.dll
│   ├── HttpClient.dll
│   ├── ConfigManager.dll
│   └── DownloadManager.dll
└── modules/            ← 业务 DLL
    ├── DeviceModel.dll
    ├── SerApiModel.dll
    ├── NetworkMonitor.dll
    └── TestOrchestrator.dll
platforms/
└── qwindows.dll
log/                    ← 自动生成
```

---

## 日志

日志文件自动生成在 `log/` 目录下，按日期命名：

```
log/2026-06-10.log
```

---

## 扩展

### 添加新环境

1. 在 `dll.ini` 添加 section
2. 创建对应的 `app.{env}.json` 覆盖配置
3. 在 exe 目录下放一个同名空文件

### 添加新业务模块

1. 在 `src/EftpClient/` 下创建目录
2. 编写 CMakeLists.txt（SHARED）
3. 在 `src/EftpClient/CMakeLists.txt` 中 `add_subdirectory`
4. 在 MainWindow 中创建实例并连接信号

### 添加托盘菜单项

1. 在 `src/EftpClient/SystemTrayManager/actions/` 下创建 `ITrayAction` 子类
2. 在 MainWindow 的 `createModules()` 中注册到 SystemTrayManager
