#include "StageTestPage.h"
#include "ItemDetailInfoDialog.h"

StageTestPage::StageTestPage(QWidget* parent)
    : QWidget(parent)
    , m_model(new ItemProgramTestModel(COLUMN_COUNT, this))
    , m_delegate(new ItemProgramTestDelegate(this))
{
    ui.setupUi(this);

    ui.treeView->setRootIsDecorated(false);
    ui.treeView->header()->hide();
    ui.treeView->setModel(m_model);
    ui.treeView->setItemDelegate(m_delegate);

    connect(m_delegate, &ItemProgramTestDelegate::signalItemClicked,
            this, &StageTestPage::onItemClicked);
    connect(ui.btnStart, &QPushButton::clicked, this, &StageTestPage::onStartBtnClicked);
}

StageTestPage::~StageTestPage()
{
}

void StageTestPage::setStageData(const TestStageInfo& stage, bool isAutoStart)
{
    m_stage = stage;

    int totalItems = (int)stage.items.size();
    m_model->setModelCounts(totalItems);
    m_delegate->setItemNumber(totalItems);

    // 填充测试项名称和状态
    int idx = 0;
    for (const auto& item : stage.items) {
        int row = idx / COLUMN_COUNT;
        int col = idx % COLUMN_COUNT;
        QModelIndex index = m_model->index(row, col);
        m_model->setData(index, QString::fromUtf8(item.itemName.c_str()), ItemProgramTestModel::ProgramNameRole);
        m_model->setData(index, item.testResult, ItemProgramTestModel::ProgramStateRole);
        idx++;
    }

    // 设置列宽
    for (int i = 0; i < COLUMN_COUNT; i++) {
        ui.treeView->setColumnWidth(i, (ui.treeView->width() - 60) / COLUMN_COUNT);
    }

    // 自动/手动模式
    ui.btnStart->setVisible(!isAutoStart);
    ui.labelTips->setText(QString("阶段: %1 | %2 个测试项")
        .arg(QString::fromUtf8(stage.stageName.c_str()))
        .arg(totalItems));
}

void StageTestPage::updateItemResult(int itemIndex, int result)
{
    int row = itemIndex / COLUMN_COUNT;
    int col = itemIndex % COLUMN_COUNT;
    QModelIndex index = m_model->index(row, col);

    // 服务器 result 映射到 TestState：
    // result=0（未知/未上报）→ Fail（红色）
    // result=1 → Success（绿色）
    // result=2 → Fail（红色）
    TestState displayState;
    switch (result) {
        case 1:  displayState = TestState::Success; break;
        case 2:  displayState = TestState::Fail;    break;
        default: displayState = TestState::Fail;    break;
    }

    m_model->setData(index, static_cast<int>(displayState), ItemProgramTestModel::ProgramStateRole);

    // 更新底部提示（通过/失败统计）
    updateTips();
}

void StageTestPage::setItemRunning(int itemIndex)
{
    int row = itemIndex / COLUMN_COUNT;
    int col = itemIndex % COLUMN_COUNT;
    QModelIndex index = m_model->index(row, col);
    m_model->setData(index, static_cast<int>(TestState::Running), ItemProgramTestModel::ProgramStateRole);
}

void StageTestPage::setAutoTestOver(bool isOver)
{
    m_autoTestOver = isOver;
}

void StageTestPage::updateTips()
{
    int total = (int)m_stage.items.size();
    int passCount = 0;
    int failCount = 0;
    int runCount = 0;

    for (int i = 0; i < total; i++) {
        int row = i / COLUMN_COUNT;
        int col = i % COLUMN_COUNT;
        QModelIndex idx = m_model->index(row, col);
        int state = idx.data(ItemProgramTestModel::ProgramStateRole).toInt();
        if (state == static_cast<int>(TestState::Success)) passCount++;
        else if (state == static_cast<int>(TestState::Fail)) failCount++;
        else if (state == static_cast<int>(TestState::Running)) runCount++;
    }

    QString stageName = QString::fromUtf8(m_stage.stageName.c_str());
    QString tips = QString("%1 | 共%2项  通过:%3  失败:%4  进行中:%5")
        .arg(stageName).arg(total).arg(passCount).arg(failCount).arg(runCount);

    if (failCount > 0) {
        ui.labelTips->setStyleSheet("font-size:13px; color:#E73C31; font-weight:bold;");
    } else if (passCount == total) {
        ui.labelTips->setStyleSheet("font-size:13px; color:#01B659; font-weight:bold;");
    } else {
        ui.labelTips->setStyleSheet("font-size:13px; color:gray;");
    }

    ui.labelTips->setText(tips);
}

void StageTestPage::onItemClicked(const QModelIndex& index)
{
    QString name = index.data(ItemProgramTestModel::ProgramNameRole).toString();
    if (name.isEmpty())
        return;

    // 构造测试项数据
    int itemIndex = index.row() * COLUMN_COUNT + index.column();
    if (itemIndex < 0 || itemIndex >= (int)m_stage.items.size())
        return;

    const auto& item = m_stage.items[itemIndex];

    // 构造 TestDeviceCycleItemVO 用于对话框
    TestDeviceCycleItemVO vo;
    vo.itemName = QString::fromUtf8(item.itemName.c_str());
    vo.deviceCycleItemId = item.cycleItemId;
    vo.result = item.testResult;
    vo.isRepeatTest = 1; // 默认支持重测
    vo.tips = QString::fromUtf8(item.detail.c_str());

    // 显示详情对话框
    ItemDetailInfoDialog dialog(this, m_autoTestOver);
    dialog.setItemDetailInfo(vo, m_autoTestOver);

    connect(&dialog, &ItemDetailInfoDialog::signalBtnRetestItem, [this](QVariant program) {
        TestDeviceCycleItemVO vo = program.value<TestDeviceCycleItemVO>();
        emit retestRequested(vo.deviceCycleItemId);
    });

    dialog.exec();
}

void StageTestPage::onStartBtnClicked()
{
    emit startTestRequested();
}

void StageTestPage::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    // 列宽在窗口显示后才能正确计算
    QTimer::singleShot(0, this, [this]() {
        updateColumnWidths();
    });
}

void StageTestPage::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateColumnWidths();
}

void StageTestPage::updateColumnWidths()
{
    int w = ui.treeView->viewport()->width();
    if (w <= 0) return;
    int colW = w / COLUMN_COUNT;
    for (int i = 0; i < COLUMN_COUNT; i++) {
        ui.treeView->setColumnWidth(i, colW);
    }
}
