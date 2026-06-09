#pragma once

#include <QDialog>
#include "ui_ErrorDialog.h"

/**
 * @brief 统一错误/警告/信息弹窗
 *
 * 用法：
 *   ErrorDialog::showError(this, "标题", "详细信息");
 *   ErrorDialog::showWarning(this, "警告", "详细信息");
 *   ErrorDialog::showInfo(this, "提示", "详细信息");
 */
class ErrorDialog : public QDialog
{
    Q_OBJECT
public:
    enum Type { Error, Warning, Info };

    explicit ErrorDialog(Type type, const QString& title, const QString& detail, QWidget* parent = nullptr);
    ~ErrorDialog();

    static void showError(QWidget* parent, const QString& title, const QString& detail = "");
    static void showWarning(QWidget* parent, const QString& title, const QString& detail = "");
    static void showInfo(QWidget* parent, const QString& title, const QString& detail = "");

private:
    Ui::ErrorDialog ui;
};
