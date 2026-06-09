#include "SNMacBindDialog.h"

SNMacBindDialog::SNMacBindDialog(const QString& mac, QWidget* parent)
    : QDialog(parent)
{
    ui.setupUi(this);
    ui.editMac->setText(mac);
    connect(ui.btnBind, &QPushButton::clicked, this, &QDialog::accept);
}

SNMacBindDialog::~SNMacBindDialog()
{
}

QString SNMacBindDialog::sn() const
{
    return ui.editSn->text().trimmed();
}

QString SNMacBindDialog::mac() const
{
    return ui.editMac->text().trimmed();
}
