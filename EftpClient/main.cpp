#include "MainWindow.h"
#include "qtlogger.h"
#include <QtWidgets/QApplication>
#include <QFile>
#include <QCommandLineParser>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/ComputerTest/icon/e.ico"));

    QCommandLineParser parser;
    parser.setApplicationDescription("EFTP Client");
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

    MainWindow w;
    w.show();

    QFile file(":/NBStyleSheet.qss");
    if (file.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(file.readAll());
        a.setStyleSheet(styleSheet);
    }

    return a.exec();
}
