#pragma once

#include <QDialog>
#include "ui_SNMacBindDialog.h"

/**
 * @brief SN/MAC 绑定对话框
 *        SN 由用户手动输入，MAC 自动获取（只读）
 */
class SNMacBindDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SNMacBindDialog(const QString& mac, QWidget* parent = nullptr);
    ~SNMacBindDialog();

    QString sn() const;
    QString mac() const;

private:
    Ui::SNMacBindDialog ui;
};
