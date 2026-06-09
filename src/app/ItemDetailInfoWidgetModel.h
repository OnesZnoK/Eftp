#pragma once

#include <QAbstractTableModel>
#include <QVector>
#include "EftpTypes.h"

/**
 * @brief 测试详情表格 Model — 显示测试规则结果列表
 *
 * 列：规则名称 | 结果 | 失败原因 | 详细信息
 */
class ItemDetailInfoWidgetModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit ItemDetailInfoWidgetModel(QObject* parent = nullptr);
    ~ItemDetailInfoWidgetModel();

    void setResults(const QList<TestDeviceCycleItemResultVO>& results);

    // QAbstractTableModel 接口
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    struct RowData {
        QString ruleName;
        QString result;
        QString errorInfo;
        QString testStandard;
    };

    QVector<RowData> m_rows;
};
