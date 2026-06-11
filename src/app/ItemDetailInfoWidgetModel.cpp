#include "ItemDetailInfoWidgetModel.h"

ItemDetailInfoWidgetModel::ItemDetailInfoWidgetModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

ItemDetailInfoWidgetModel::~ItemDetailInfoWidgetModel()
{
}

void ItemDetailInfoWidgetModel::setResults(const QList<TestDeviceCycleItemResultVO>& results)
{
    beginResetModel();
    m_rows.clear();
    for (const auto& r : results) {
        RowData row;
        row.ruleName = r.ruleName;
        row.result = (r.result == 1) ? "通过" : (r.result == 0) ? "失败" : "未知";
        row.errorInfo = r.errorInfo;
        row.testStandard = r.testStandard;
        m_rows.append(row);
    }
    endResetModel();
}

int ItemDetailInfoWidgetModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return m_rows.size();
}

int ItemDetailInfoWidgetModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 4; // 规则名称 | 结果 | 失败原因 | 详细信息
}

QVariant ItemDetailInfoWidgetModel::data(const QModelIndex& index, int role) const
{
    if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
        return QVariant();

    int row = index.row();
    if (row < 0 || row >= m_rows.size())
        return QVariant();

    const RowData& d = m_rows.at(row);
    switch (index.column()) {
        case 0: return d.ruleName;
        case 1: return d.result;
        case 2: return d.errorInfo;
        case 3: return d.testStandard;
        default: return QVariant();
    }
}

QVariant ItemDetailInfoWidgetModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
            case 0: return "规则名称";
            case 1: return "结果";
            case 2: return "失败原因";
            case 3: return "测试标准";
            default: return QVariant();
        }
    }
    return section + 1;
}
