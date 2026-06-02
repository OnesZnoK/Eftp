#include "ComputerTest.h"
#include "qtlogger.h"
#include <QtWidgets/QApplication>
#include <QFile>
#include <QCommandLineParser>

void getDeviceBaeInfo() {}

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/ComputerTest/icon/e.ico"));

    QCommandLineParser parser;

    // 定义参数规范
    parser.setApplicationDescription("Computer Test Application");
    parser.addHelpOption();

    // 定义带默认值的环境参数（非必填）
    QCommandLineOption envOption("e", QCoreApplication::translate("main", "运行环境 [product/pre/test]"),
                                 "env",    // 参数名称（显示在帮助信息中）
                                 "product" // 默认值
    );

    // 定义非必填的序列号参数
    QCommandLineOption snOption("s", QCoreApplication::translate("main", "设备序列号（可选）"),
                                "sn" // 参数名称
    );

    parser.addOption(envOption);
    parser.addOption(snOption);
    // 改用parse()以便错误处理
    if (!parser.parse(qApp->arguments())) {
        QtLogger::WriteLog(parser.errorText(), enLogType::SERIOUS);
        return 1;
    }

    // 获取参数值
    QString env = parser.value(envOption);
    QString sn = parser.value(snOption);
    if (sn.isEmpty()) {
        sn = "";
    }
    //sn = "AAA"; // FCL
    //env = "test";
    ComputerTest w(nullptr, env, sn);
    w.show();

    /* 设置基本样式表 */
    QFile file(":/NBStyleSheet.qss");
    if (file.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(file.readAll());
        a.setStyleSheet(styleSheet);
    }

    return a.exec();
}
