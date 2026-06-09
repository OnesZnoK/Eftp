#pragma once

#include <QAbstractItemModel>
#include <QVector>
#include "EftpTypes.h"

/**
 * @brief 测试项网格 Model（N 列网格布局）
 *
 * 数据存储为一维数组，按 row*columnCount + column 映射到二维网格。
 * 每个单元格存储：程序名称、运行状态、结果详情 JSON。
 */
class ItemProgramTestModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit ItemProgramTestModel(int columnCount = 8, QObject* parent = nullptr);
    ~ItemProgramTestModel();

    // 设置总测试项数量
    void setModelCounts(int totalCount);

    // QAbstractItemModel 接口
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // 自定义角色
    enum CustomRoles {
        ProgramNameRole = Qt::UserRole + 1,
        ProgramStateRole,
        ResultInfoRole
    };

private:
    struct CellData {
        QString programName;
        TestState state = TestState::UnStart;
        QString resultInfo;
    };

    int m_columnCount;
    QVector<CellData> m_data;
};
