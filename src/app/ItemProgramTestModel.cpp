#include "ItemProgramTestModel.h"

ItemProgramTestModel::ItemProgramTestModel(int columnCount, QObject* parent)
    : QAbstractItemModel(parent)
    , m_columnCount(columnCount)
{
}

ItemProgramTestModel::~ItemProgramTestModel()
{
}

void ItemProgramTestModel::setModelCounts(int totalCount)
{
    beginResetModel();
    m_data.resize(totalCount);
    for (auto& cell : m_data) {
        cell.programName.clear();
        cell.state = TestState::UnStart;
        cell.resultInfo.clear();
    }
    endResetModel();
}

QModelIndex ItemProgramTestModel::index(int row, int column, const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    int idx = row * m_columnCount + column;
    if (idx < 0 || idx >= m_data.size())
        return QModelIndex();
    return createIndex(row, column, static_cast<quintptr>(idx));
}

QModelIndex ItemProgramTestModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex(); // 平坦模型，无父节点
}

int ItemProgramTestModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return (m_data.size() + m_columnCount - 1) / m_columnCount;
}

int ItemProgramTestModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_columnCount;
}

QVariant ItemProgramTestModel::data(const QModelIndex& index, int role) const
{
    int idx = index.row() * m_columnCount + index.column();
    if (idx < 0 || idx >= m_data.size())
        return QVariant();

    const CellData& cell = m_data.at(idx);

    switch (role) {
    case Qt::DisplayRole:
    case ProgramNameRole:
        return cell.programName;
    case ProgramStateRole:
        return static_cast<int>(cell.state);
    case ResultInfoRole:
        return cell.resultInfo;
    default:
        return QVariant();
    }
}

bool ItemProgramTestModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    int idx = index.row() * m_columnCount + index.column();
    if (idx < 0 || idx >= m_data.size())
        return false;

    CellData& cell = m_data[idx];

    switch (role) {
    case Qt::DisplayRole:
    case ProgramNameRole:
        cell.programName = value.toString();
        break;
    case ProgramStateRole:
        cell.state = static_cast<TestState>(value.toInt());
        break;
    case ResultInfoRole:
        cell.resultInfo = value.toString();
        break;
    default:
        return false;
    }

    emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
    return true;
}

Qt::ItemFlags ItemProgramTestModel::flags(const QModelIndex& index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
