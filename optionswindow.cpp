#include "optionswindow.h"

OptionsWindow::OptionsWindow(QWidget *parent) : QWidget(parent)
{
    vlayout = new QVBoxLayout(this);
    layout = new QFormLayout();
    hlayout = new QHBoxLayout();

    idEdit = new QLineEdit(this);
    passwordEdit = new QLineEdit(this);
    passwordEdit->setEchoMode(QLineEdit::Password);

    layout->addRow(tr("用户名"), idEdit);
    layout->addRow(tr("密码"), passwordEdit);

    saveButton = new QPushButton("确定", this);
    cancleButton = new QPushButton("取消", this);
    hlayout->addWidget(saveButton);
    hlayout->addWidget(cancleButton);

    vlayout->addLayout(layout);
    vlayout->addLayout(hlayout);

    setLayout(vlayout);

    connect(saveButton, QPushButton::clicked, this, [this](){emit saveSettings(idEdit->text().toUtf8(), passwordEdit->text().toUtf8());});
    connect(cancleButton, QPushButton::clicked, this, hide);
}
