#include "ItemProgramTestDelegate.h"
#include "ItemProgramTestModel.h"
#include <QMouseEvent>

static constexpr int itemProgramHeight = 55;
static constexpr int itemWidthMargin = 15;
static constexpr int itemHeightMargin = 10;

ItemProgramTestDelegate::ItemProgramTestDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

ItemProgramTestDelegate::~ItemProgramTestDelegate()
{
}

void ItemProgramTestDelegate::setItemNumber(int number)
{
    m_itemNumber = number;
}

void ItemProgramTestDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                     const QModelIndex& index) const
{
    int idx = index.row() * 8 + index.column();
    if (!index.isValid() || idx >= m_itemNumber) {
        return;
    }

    // 根据状态设置背景色和文字颜色
    QColor backgroundColor;
    QColor textColor = Qt::black;
    TestState state = static_cast<TestState>(index.data(ItemProgramTestModel::ProgramStateRole).toInt());

    switch (state) {
        case TestState::UnStart:
            backgroundColor = QColor("#E8E8E8");
            textColor = QColor("#8D8D8D");
            break;
        case TestState::Running:
            backgroundColor = QColor("#FFECCC");
            textColor = QColor("#F37E00");
            break;
        case TestState::Success:
            backgroundColor = QColor("#E7FFF1");
            textColor = QColor("#01B659");
            break;
        case TestState::Fail:
            backgroundColor = QColor("#FFE1DE");
            textColor = QColor("#E73C31");
            break;
        default:
            backgroundColor = QColor("#E8E8E8");
            break;
    }

    // 绘制带边距的背景色
    QRect cellRect = option.rect.adjusted(itemWidthMargin, itemHeightMargin,
                                          -itemWidthMargin, -itemHeightMargin);
    painter->fillRect(cellRect, backgroundColor);

    // 绘制文字（居中）
    painter->save();
    painter->setPen(textColor);
    QString name = index.data(ItemProgramTestModel::ProgramNameRole).toString();
    painter->drawText(cellRect, Qt::AlignHCenter | Qt::AlignVCenter, name);
    painter->restore();
}

QSize ItemProgramTestDelegate::sizeHint(const QStyleOptionViewItem& option,
                                         const QModelIndex& index) const
{
    Q_UNUSED(index);
    // 宽度用 option.rect（让每列等宽），高度固定 55px
    int w = option.rect.width() > 0 ? option.rect.width() : 140;
    return QSize(w, itemProgramHeight);
}

bool ItemProgramTestDelegate::editorEvent(QEvent* event, QAbstractItemModel* model,
                                           const QStyleOptionViewItem& option, const QModelIndex& index)
{
    // 只处理鼠标释放事件
    if (event->type() != QEvent::MouseButtonRelease)
        return QStyledItemDelegate::editorEvent(event, model, option, index);

    QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
    if (mouseEvent->button() != Qt::LeftButton)
        return QStyledItemDelegate::editorEvent(event, model, option, index);

    // 检查点击是否在有效单元格内
    QRect cellRect = option.rect.adjusted(itemWidthMargin, itemHeightMargin,
                                          -itemWidthMargin, -itemHeightMargin);
    if (cellRect.contains(mouseEvent->pos())) {
        QString name = index.data(ItemProgramTestModel::ProgramNameRole).toString();
        if (!name.isEmpty()) {
            emit signalItemClicked(index);
        }
    }
    return true;
}
