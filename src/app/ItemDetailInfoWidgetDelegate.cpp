#include "ItemDetailInfoWidgetDelegate.h"
#include <QPainter>

ItemDetailInfoWidgetDelegate::ItemDetailInfoWidgetDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

ItemDetailInfoWidgetDelegate::~ItemDetailInfoWidgetDelegate()
{
}

void ItemDetailInfoWidgetDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                          const QModelIndex& index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    // 结果列（第1列）染色
    if (index.column() == 1) {
        QString text = index.data(Qt::DisplayRole).toString();
        if (text == "通过") {
            opt.backgroundBrush = QColor(200, 255, 200);
            opt.palette.setColor(QPalette::Text, Qt::darkGreen);
        } else if (text == "失败") {
            opt.backgroundBrush = QColor(255, 200, 200);
            opt.palette.setColor(QPalette::Text, Qt::red);
        }
    }

    QStyledItemDelegate::paint(painter, opt, index);
}
