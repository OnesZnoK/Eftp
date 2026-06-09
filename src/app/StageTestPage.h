#pragma once

#include <QWidget>
#include <QShowEvent>
#include <QResizeEvent>
#include <QTimer>
#include "ui_StageTestPage.h"
#include "EftpTypes.h"
#include "ItemProgramTestModel.h"
#include "ItemProgramTestDelegate.h"

/**
 * @brief 阶段测试页面 — QTreeView 填满空间，底部开始按钮
 *
 * 布局：
 *   ┌─ QTreeView（填满）────────────────────┐
 *   │  测试项网格 8列，55px行高，背景色状态  │
 *   ├─ 底部按钮区 (60px) ──────────────────┤
 *   │  [开始测试]              状态提示      │
 *   └───────────────────────────────────────┘
 */
class StageTestPage : public QWidget
{
    Q_OBJECT

public:
    explicit StageTestPage(QWidget* parent = nullptr);
    ~StageTestPage();

    void setStageData(const TestStageInfo& stage, bool isAutoStart);
    void updateItemResult(int itemIndex, int result);
    void setItemRunning(int itemIndex);  // 设置为运行中（黄色）
    void setAutoTestOver(bool isOver);
    ItemProgramTestModel* getModel() const { return m_model; }
    void updateTips();  // 更新底部提示（通过/失败统计）

signals:
    void startTestRequested();
    void retestRequested(int cycleItemId);

protected:
    void showEvent(QShowEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onItemClicked(const QModelIndex& index);
    void onStartBtnClicked();

private:
    void updateColumnWidths();
    Ui::StageTestPage ui;

    ItemProgramTestModel* m_model;
    ItemProgramTestDelegate* m_delegate;
    TestStageInfo m_stage;
    bool m_autoTestOver = false;

    static constexpr int COLUMN_COUNT = 8;
};
