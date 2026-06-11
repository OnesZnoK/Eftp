#include "MainWindow.h"
#include "qtlogger.h"
#include "ConfigManager.h"
#include <QtWidgets/QApplication>
#include <QFile>
#include <QCommandLineParser>
#include <QDir>
#include <windows.h>

/**
 * @brief 在加载任何业务 DLL 之前，将子目录加入 DLL 搜索路径
 *
 * 注意：此函数不能调用任何业务 DLL 的函数（QtLogger、ConfigManager 等），
 *       因为延迟加载会在首次调用时触发，而此时路径尚未设置。
 *       因此这里直接从 dll.ini 读取路径，不依赖 ConfigManager。
 */
static void addDllSearchPaths()
{
    // 获取 exe 所在目录
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    std::wstring exeDir = exePath;
    exeDir = exeDir.substr(0, exeDir.find_last_of(L"\\/"));

    // 直接读取 dll.ini（不经过 ConfigManager，避免触发 DLL 加载）
    std::wstring iniPath = exeDir + L"\\configs\\dll.ini";
    FILE* f = nullptr;
    _wfopen_s(&f, iniPath.c_str(), L"r");
    if (!f) return;

    // 简易解析：找到第一个 [section]，读取 core= 和 modules=
    char line[512];
    bool inSection = false;
    while (fgets(line, sizeof(line), f)) {
        // 跳过注释和空行
        if (line[0] == ';' || line[0] == '#' || line[0] == '\n') continue;
        // 检测 section
        if (line[0] == '[') { inSection = true; continue; }
        if (!inSection) continue;

        // 解析 key=value
        char* eq = strchr(line, '=');
        if (!eq) continue;
        *eq = '\0';
        const char* key = line;
        const char* value = eq + 1;

        // 去掉末尾换行
        size_t vlen = strlen(value);
        while (vlen > 0 && (value[vlen-1] == '\n' || value[vlen-1] == '\r')) vlen--;

        if ((strcmp(key, "core") == 0 || strcmp(key, "modules") == 0) && vlen > 0) {
            // 拼接绝对路径
            std::wstring relPath(value, value + vlen);
            // 替换 '/' 为反斜杠
            for (auto& c : relPath) { if (c == L'/') c = L'\\'; }
            std::wstring absPath = exeDir + L"\\" + relPath;

            // 检查目录存在后添加
            DWORD attr = GetFileAttributesW(absPath.c_str());
            if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY)) {
                AddDllDirectory(absPath.c_str());
            }
        }

        // 遇到下一个 section 则停止
        // (此处 dll.ini 只有一个 section，循环自然结束)
    }
    fclose(f);

    // 设置 DLL 搜索顺序：先 exe 目录，再 AddDllDirectory 添加的路径
    SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
}

int main(int argc, char* argv[])
{
    // 启用高 DPI 缩放支持
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/Eftp/icon/e.ico"));

    // 在加载任何业务 DLL 之前，先设置 DLL 搜索路径
    addDllSearchPaths();

    // 初始化日志模块（此时 QtLogger.dll 通过延迟加载从 dll/core/ 找到）
    QtLogger::WriteLog("========== EFTP 启动 ==========");

    // 解析命令行参数
    QCommandLineParser parser;
    parser.setApplicationDescription("EFTP Client — 电子工厂测试平台");
    parser.addHelpOption();

    QCommandLineOption envOption("e",
        QCoreApplication::translate("main", "运行环境 [product/pre/test]"),
        "env", "product");
    QCommandLineOption snOption("s",
        QCoreApplication::translate("main", "设备序列号（可选）"),
        "sn");

    parser.addOption(envOption);
    parser.addOption(snOption);

    if (!parser.parse(qApp->arguments())) {
        QtLogger::WriteLog(parser.errorText(), enLogType::SERIOUS);
        return 1;
    }

    QString env = parser.value(envOption);
    QString sn = parser.value(snOption);

    // 初始化配置管理器
    QString configsDir = QCoreApplication::applicationDirPath() + "/configs";
    ConfigManager::instance().init(configsDir.toStdString());

    // 加载样式表
    QFile file(":/Eftp/NBStyleSheet.qss");
    if (file.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(file.readAll());
        a.setStyleSheet(styleSheet);
    }

    // 创建主窗口
    MainWindow w(env, sn);
    w.show();

    return a.exec();
}
