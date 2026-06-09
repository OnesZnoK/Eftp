#pragma once

#include <QStyledItemDelegate>
#include <QPainter>
#include <QModelIndex>

/**
 * @brief 测试项网格 Delegate — 背景色表示状态 + 程序名称
 *
 * 每个单元格显示：
 *   - 背景色：灰色=未开始，黄色=运行中，绿色=通过，红色=失败
 *   - 程序名称（居中）
 *   - 点击时发射 signalItemClicked
 */
class ItemProgramTestDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit ItemProgramTestDelegate(QObject* parent = nullptr);
    ~ItemProgramTestDelegate();

    /** @brief 设置测试项总数（用于绘制边界判断） */
    void setItemNumber(int number);

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option,
                   const QModelIndex& index) const override;

    bool editorEvent(QEvent* event, QAbstractItemModel* model,
                     const QStyleOptionViewItem& option, const QModelIndex& index) override;

signals:
    /** @brief 用户点击测试项（StageTestPage 连接此信号） */
    void signalItemClicked(const QModelIndex& index);

private:
    int m_itemNumber = 0;  ///< 测试项总数
};
