#include "optionswindow.h"

OptionsWindow::OptionsWindow(QByteArray id, QByteArray pass, QWidget *parent) : QWidget(parent)
{
    setWindowTitle(tr("选项"));

    vlayout = new QVBoxLayout(this);
    layout = new QFormLayout();
    hlayout = new QHBoxLayout();

    idEdit = new QLineEdit(this);
    idEdit->setText(id);
    passwordEdit = new QLineEdit(this);
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setText(pass);

    layout->addRow(tr("用户名"), idEdit);
    layout->addRow(tr("密码"), passwordEdit);
    layout->setSpacing(20);

    saveButton = new QPushButton("确定", this);
    cancleButton = new QPushButton("取消", this);
    hlayout->addWidget(saveButton);
    hlayout->addWidget(cancleButton);

    vlayout->addLayout(layout);
    vlayout->addLayout(hlayout);
    vlayout->setSpacing(20);

    setLayout(vlayout);

    connect(saveButton, QPushButton::clicked, this, [this](){emit saveSettings(idEdit->text().toUtf8(), passwordEdit->text().toUtf8());});
    connect(cancleButton, QPushButton::clicked, this, hide);
}
