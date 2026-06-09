#include "SettingsDialog.h"
#include "ConfigManager.h"
#include "qtlogger.h"

#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTextStream>
#include <QMessageBox>

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    ui.setupUi(this);
    setFixedSize(500, 600);

    loadCurrentConfig();

    connect(ui.btnSave, &QPushButton::clicked, this, &SettingsDialog::onSaveClicked);
    connect(ui.btnCancel, &QPushButton::clicked, this, &SettingsDialog::onCancelClicked);
    connect(ui.btnAddRoute, &QPushButton::clicked, this, &SettingsDialog::onAddRouteClicked);
    connect(ui.btnDeleteRoute, &QPushButton::clicked, this, &SettingsDialog::onDeleteRouteClicked);
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::loadCurrentConfig()
{
    auto& cfg = ConfigManager::instance();

    // 当前环境
    m_currentEnv = QString::fromStdString(cfg.detectEnvironment());
    ui.comboEnv->addItems({"product", "pre", "test"});
    ui.comboEnv->setCurrentText(m_currentEnv);

    // 服务器地址
    ui.labelServer->setText(QString::fromStdString(cfg.baseUrl()));

    // 网络配置
    ui.labelPing->setText(QString::fromStdString(cfg.pingTarget()));
    ui.labelInterval->setText(QString("%1 秒").arg(cfg.pingIntervalSec()));
    ui.labelThreshold->setText(QString("%1 ms").arg(cfg.latencyThresholdMs()));

    // 下载配置
    ui.label7za->setText(QString::fromStdString(cfg.tool7zaPath()));
    ui.labelRetry->setText(QString::number(cfg.maxRetry()));

    // WiFi 配置
    auto defaultWifi = cfg.defaultWifi();
    ui.editDefaultSsid->setText(QString::fromStdString(defaultWifi.first));
    ui.editDefaultPassword->setText(QString::fromStdString(defaultWifi.second));

    // 工序 WiFi 映射（读取 wifi.json）
    QString wifiPath = QCoreApplication::applicationDirPath() + "/configs/wifi.json";
    QFile wifiFile(wifiPath);
    if (wifiFile.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(wifiFile.readAll());
        wifiFile.close();
        QJsonObject root = doc.object();
        QJsonObject mapping = root["routeMapping"].toObject();

        ui.tableRouteWifi->setRowCount(mapping.size());
        int row = 0;
        for (auto it = mapping.begin(); it != mapping.end(); ++it, ++row) {
            QJsonObject entry = it.value().toObject();
            ui.tableRouteWifi->setItem(row, 0, new QTableWidgetItem(it.key()));
            ui.tableRouteWifi->setItem(row, 1, new QTableWidgetItem(entry["ssid"].toString()));
            ui.tableRouteWifi->setItem(row, 2, new QTableWidgetItem(entry["password"].toString()));
        }
    }
}

void SettingsDialog::onSaveClicked()
{
    saveWifiConfig();

    QString newEnv = ui.comboEnv->currentText();
    if (newEnv != m_currentEnv) {
        saveEnvironment();
        QMessageBox::information(this, "提示",
            QString("环境已切换为 %1，需要重启程序生效。").arg(newEnv));
    } else {
        QMessageBox::information(this, "提示", "WiFi 配置已保存。");
    }

    accept();
}

void SettingsDialog::onCancelClicked()
{
    reject();
}

void SettingsDialog::onAddRouteClicked()
{
    int row = ui.tableRouteWifi->rowCount();
    ui.tableRouteWifi->insertRow(row);
    ui.tableRouteWifi->setItem(row, 0, new QTableWidgetItem(""));
    ui.tableRouteWifi->setItem(row, 1, new QTableWidgetItem(""));
    ui.tableRouteWifi->setItem(row, 2, new QTableWidgetItem(""));
}

void SettingsDialog::onDeleteRouteClicked()
{
    int currentRow = ui.tableRouteWifi->currentRow();
    if (currentRow >= 0) {
        ui.tableRouteWifi->removeRow(currentRow);
    }
}

void SettingsDialog::saveWifiConfig()
{
    QJsonObject root;

    // 默认 WiFi
    QJsonObject defaultWifi;
    defaultWifi["ssid"] = ui.editDefaultSsid->text();
    defaultWifi["password"] = ui.editDefaultPassword->text();
    root["default"] = defaultWifi;

    // 工序 WiFi 映射
    QJsonObject mapping;
    for (int i = 0; i < ui.tableRouteWifi->rowCount(); i++) {
        QString routeId = ui.tableRouteWifi->item(i, 0)->text().trimmed();
        QString ssid = ui.tableRouteWifi->item(i, 1)->text().trimmed();
        QString password = ui.tableRouteWifi->item(i, 2)->text().trimmed();
        if (!routeId.isEmpty()) {
            QJsonObject entry;
            entry["ssid"] = ssid;
            entry["password"] = password;
            mapping[routeId] = entry;
        }
    }
    root["routeMapping"] = mapping;

    // 写入文件
    QString wifiPath = QCoreApplication::applicationDirPath() + "/configs/wifi.json";
    QFile file(wifiPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << QJsonDocument(root).toJson(QJsonDocument::Indented);
        file.close();
        QtLogger::WriteLog("SettingsDialog: WiFi 配置已保存");
    }
}

void SettingsDialog::saveEnvironment()
{
    QString newEnv = ui.comboEnv->currentText();
    QString exeDir = QCoreApplication::applicationDirPath();

    // 删除旧的环境标记文件
    QStringList envs = {"product", "pre", "test"};
    for (const QString& env : envs) {
        QFile::remove(exeDir + "/" + env);
    }

    // 创建新的环境标记文件
    QFile envFile(exeDir + "/" + newEnv);
    if (envFile.open(QIODevice::WriteOnly)) {
        envFile.close();
    }

    QtLogger::WriteLog("SettingsDialog: 环境已切换为 " + newEnv);
}
