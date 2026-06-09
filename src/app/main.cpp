#include "MainWindow.h"
#include "qtlogger.h"
#include "ConfigManager.h"
#include <QtWidgets/QApplication>
#include <QFile>
#include <QCommandLineParser>
#include <QDir>
#include <windows.h>

/**
 * @brief 根据配置文件添加 DLL 搜索路径
 *        在 QApplication 创建之后、加载业务模块之前调用
 */
static void addDllSearchPaths()
{
    QString exeDir = QCoreApplication::applicationDirPath();
    auto& cfg = ConfigManager::instance();

    // 从 dll.ini 读取 DLL 路径配置（按 name_flag 环境）
    QStringList dllPaths;
    dllPaths << QDir::toNativeSeparators(exeDir + "/" + QString::fromStdString(cfg.dllPathCore()));
    dllPaths << QDir::toNativeSeparators(exeDir + "/" + QString::fromStdString(cfg.dllPathModules()));

    for (const QString& path : dllPaths) {
        if (QDir(path).exists()) {
            AddDllDirectory(path.toStdWString().c_str());
            QtLogger::WriteLog("DLL 搜索路径: " + path);
        } else {
            QtLogger::WriteLog("DLL 路径不存在: " + path, enLogType::WARNING);
        }
    }

    // 设置 DLL 搜索顺序：先 exe 目录，再 AddDllDirectory 添加的路径
    SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
}

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/Eftp/icon/e.ico"));

    // 初始化日志模块
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

    // 添加 DLL 搜索路径（从配置文件读取）
    addDllSearchPaths();

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
