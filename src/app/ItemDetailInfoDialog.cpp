#include "ItemDetailInfoDialog.h"

ItemDetailInfoDialog::ItemDetailInfoDialog(QWidget* parent, bool canRetest)
    : QDialog(parent)
    , m_model(new ItemDetailInfoWidgetModel(this))
    , m_delegate(new ItemDetailInfoWidgetDelegate(this))
{
    ui.setupUi(this);

    ui.tableView->setModel(m_model);
    ui.tableView->setItemDelegate(m_delegate);
    ui.tableView->horizontalHeader()->setStretchLastSection(true);
    ui.tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui.btnRetest->setEnabled(canRetest);
    connect(ui.btnRetest, &QPushButton::clicked, this, &ItemDetailInfoDialog::onRetestClicked);
}

ItemDetailInfoDialog::~ItemDetailInfoDialog()
{
}

void ItemDetailInfoDialog::setItemDetailInfo(const TestDeviceCycleItemVO& item, bool autoTestOver)
{
    m_item = item;
    ui.labelName->setText(item.itemName);

    // 测试结果
    QString statusText;
    QColor statusColor;
    if (item.result == 1) {
        statusText = "✓ 测试通过";
        statusColor = QColor("#01B659");
    } else if (item.result == 2) {
        statusText = "✗ 测试失败";
        statusColor = QColor("#E73C31");
    } else {
        statusText = "— 未测试";
        statusColor = QColor("#8D8D8D");
    }
    ui.labelStatus->setText(statusText);
    ui.labelStatus->setStyleSheet(QString("font-size:15px; font-weight:bold; color:%1;").arg(statusColor.name()));

    // 知识库提示
    ui.textTips->setText(item.tips.isEmpty() ? "无" : item.tips);

    // 规则结果表格
    m_model->setResults(item.testDeviceCycleItemResultVOList);
    ui.tableView->setColumnWidth(0, 120);
    ui.tableView->setColumnWidth(1, 60);
    ui.tableView->setColumnWidth(2, 150);

    // 重测按钮：自动测试结束后 + 支持重测 才可用
    ui.btnRetest->setEnabled(autoTestOver && item.isRepeatTest);
}

void ItemDetailInfoDialog::onRetestClicked()
{
    emit signalBtnRetestItem(QVariant::fromValue(m_item));
    accept();
}
