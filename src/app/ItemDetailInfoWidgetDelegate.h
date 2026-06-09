#pragma once

#include <QStyledItemDelegate>

/**
 * @brief 测试详情表格 Delegate — 根据结果列染色
 *        通过=绿色，失败=红色
 */
class ItemDetailInfoWidgetDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ItemDetailInfoWidgetDelegate(QObject* parent = nullptr);
    ~ItemDetailInfoWidgetDelegate();

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
};
