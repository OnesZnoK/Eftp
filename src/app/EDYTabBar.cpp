#include "EDYTabBar.h"
#include <QStyleOptionTab>
#include <QStylePainter>
#include <QFontMetrics>

static const QString picRunning   = ":/Eftp/image/running.png";
static const QString picPass      = ":/Eftp/image/testPass.png";
static const QString picFail      = ":/Eftp/image/testFail.png";
static const QString picUnstart   = ":/Eftp/image/unStartTest.png";

static constexpr int iconW = 34;
static constexpr int iconH = 34;
static constexpr int leftMargin = 78;
static constexpr int topMargin = 26;
static constexpr int textTopMargin = 74;
static constexpr int textH = 20;
static constexpr int lineTopMargin = 43;

EDYTabBar::EDYTabBar(QWidget* parent)
    : QTabBar(parent)
{
    setFixedHeight(114);
}

EDYTabBar::~EDYTabBar()
{
}

void EDYTabBar::setTabStatus(int index, TestState status)
{
    m_tabStatus[index] = status;
    update();
}

void EDYTabBar::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QStylePainter painter(this);
    QStyleOptionTab opt;

    for (int i = 0; i < count(); i++) {
        initStyleOption(&opt, i);
        painter.drawControl(QStyle::CE_TabBarTabShape, opt);

        QRect rect = tabRect(i);
        QString text = tabText(i);

        // 根据状态选择图标
        QIcon icon;
        switch (m_tabStatus.value(i, TestState::UnStart)) {
            case TestState::Running: icon = QIcon(picRunning); break;
            case TestState::Success: icon = QIcon(picPass);    break;
            case TestState::Fail:    icon = QIcon(picFail);    break;
            default:                 icon = QIcon(picUnstart); break;
        }

        QPixmap pixmap = icon.pixmap(iconW, iconH);
        QRect iconRect;
        QRect textRect;

        // 虚线连接
        QPen pen(Qt::gray);
        pen.setStyle(Qt::DashDotLine);
        painter.setPen(pen);

        QFontMetrics fm = painter.fontMetrics();
        int textWidth = fm.width(text);

        if (i == 0 && count() != 1) {
            iconRect = QRect(rect.left() + leftMargin, rect.top() + topMargin, iconW, iconH);
            textRect = QRect(rect.left() + leftMargin + (iconW - textWidth) / 2, rect.top() + textTopMargin, textWidth, textH);
            painter.drawLine(QLine(rect.left() + leftMargin + iconW, lineTopMargin, rect.right(), lineTopMargin));
        } else if (i == count() - 1 && count() != 1) {
            iconRect = QRect(rect.right() - leftMargin - iconW, rect.top() + topMargin, iconW, iconH);
            textRect = QRect(rect.right() - leftMargin - iconW + (iconW - textWidth) / 2, rect.top() + textTopMargin, textWidth, textH);
            painter.drawLine(QLine(rect.left(), lineTopMargin, rect.right() - leftMargin - iconW, lineTopMargin));
        } else {
            iconRect = QRect(rect.left() + (rect.width() - iconW) / 2, rect.top() + topMargin, iconW, iconH);
            textRect = QRect(rect.left() + (rect.width() - textWidth) / 2, rect.top() + textTopMargin, textWidth, textH);
            painter.drawLine(QLine(rect.left(), lineTopMargin, rect.left() + (rect.width() - iconW) / 2, lineTopMargin));
            painter.drawLine(QLine(rect.right() - (rect.width() - iconW) / 2, lineTopMargin, rect.right(), lineTopMargin));
        }

        painter.drawPixmap(iconRect, pixmap);

        QColor textColor = (currentIndex() == i) ? Qt::blue : Qt::black;
        if (!text.isEmpty()) {
            painter.setPen(QPen(textColor, 2));
            painter.drawText(textRect, Qt::AlignVCenter, text);
        }
    }
}

QSize EDYTabBar::sizeHint() const
{
    QSize size = QTabBar::sizeHint();
    size.setHeight(114);
    return size;
}

QSize EDYTabBar::tabSizeHint(int index) const
{
    QSize size = QTabBar::tabSizeHint(index);
    size.setHeight(114);
    size.setWidth(qMax(size.width(), 160)); // 最小宽度 160px
    return size;
}

void EDYTabBar::mousePressEvent(QMouseEvent* event)
{
    int idx = tabAt(event->pos());
    if (idx >= 0 && m_tabStatus.value(idx, TestState::UnStart) == TestState::UnStart) {
        return; // 未开始的 Tab 不可点击
    }
    QTabBar::mousePressEvent(event);
}
