#include "ErrorDialog.h"

ErrorDialog::ErrorDialog(Type type, const QString& title, const QString& detail, QWidget* parent)
    : QDialog(parent)
{
    ui.setupUi(this);

    // 根据类型设置图标和颜色
    switch (type) {
        case Error:
            ui.labelIcon->setText("✗");
            ui.labelIcon->setStyleSheet("font-size:28px; color:#E73C31;");
            setWindowTitle("错误");
            break;
        case Warning:
            ui.labelIcon->setText("⚠");
            ui.labelIcon->setStyleSheet("font-size:28px; color:#F37E00;");
            setWindowTitle("警告");
            break;
        case Info:
            ui.labelIcon->setText("✓");
            ui.labelIcon->setStyleSheet("font-size:28px; color:#01B659;");
            setWindowTitle("提示");
            break;
    }

    ui.labelTitle->setText(title);

    if (detail.isEmpty()) {
        ui.textDetail->hide();
        adjustSize();
    } else {
        ui.textDetail->setText(detail);
    }

    connect(ui.btnOk, &QPushButton::clicked, this, &QDialog::accept);
}

ErrorDialog::~ErrorDialog()
{
}

void ErrorDialog::showError(QWidget* parent, const QString& title, const QString& detail)
{
    ErrorDialog dlg(Error, title, detail, parent);
    dlg.exec();
}

void ErrorDialog::showWarning(QWidget* parent, const QString& title, const QString& detail)
{
    ErrorDialog dlg(Warning, title, detail, parent);
    dlg.exec();
}

void ErrorDialog::showInfo(QWidget* parent, const QString& title, const QString& detail)
{
    ErrorDialog dlg(Info, title, detail, parent);
    dlg.exec();
}
