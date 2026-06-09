#pragma once

#include <QDialog>
#include "ui_ItemDetailInfoDialog.h"
#include "EftpTypes.h"
#include "ItemDetailInfoWidgetModel.h"
#include "ItemDetailInfoWidgetDelegate.h"

/**
 * @brief 测试项详情对话框
 *
 * 显示：测试结果、规则结果表格、知识库提示、重测按钮
 */
class ItemDetailInfoDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ItemDetailInfoDialog(QWidget* parent = nullptr, bool canRetest = false);
    ~ItemDetailInfoDialog();

    /** @brief 设置测试项详情数据（名称、结果、规则列表、提示） */
    void setItemDetailInfo(const TestDeviceCycleItemVO& item, bool autoTestOver);

signals:
    /** @brief 用户点击重测按钮（携带 TestDeviceCycleItemVO） */
    void signalBtnRetestItem(QVariant program);

private slots:
    void onRetestClicked();

private:
    Ui::ItemDetailInfoDialog ui;
    ItemDetailInfoWidgetModel* m_model;
    ItemDetailInfoWidgetDelegate* m_delegate;
    TestDeviceCycleItemVO m_item;
};
