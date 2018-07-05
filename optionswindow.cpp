#include "optionswindow.h"

OptionsWindow::OptionsWindow(QByteArray id, QByteArray pass, int traffic, QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle(tr("选项"));

    tabWidget = new QTabWidget(this);
    page1 = new QWidget(this);

    vlayout = new QVBoxLayout(page1);
    layout = new QFormLayout();
    hlayout = new QHBoxLayout();

    idEdit = new QLineEdit(this);
    idEdit->setText(id);

    passwordEdit = new QLineEdit(this);
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setContextMenuPolicy(Qt::NoContextMenu);
    passwordEdit->setText(pass);
    passwordEdit->setToolTip(tr("密码仅用于登陆"));

    trafficSpin = new QSpinBox(this);
    trafficSpin->setValue(traffic);
    trafficSpin->setToolTip(tr("0为无限制"));

    trafficLabel = new QLabel(tr("流量(G)"), this);
    trafficLabel->setToolTip(tr("在流量将要用尽时进行提醒"));

    layout->addRow(tr("用户名"), idEdit);
    layout->addRow(tr("密码"), passwordEdit);
    layout->addRow(trafficLabel, trafficSpin);
    layout->setSpacing(20);

    saveButton = new QPushButton("确定", this);
    saveButton->setDefault(true);
    saveButton->setToolTip(tr("保存设置并关闭"));
    cancleButton = new QPushButton("取消", this);
    cancleButton->setShortcut(Qt::Key_Escape);
    hlayout->addWidget(saveButton);
    hlayout->addWidget(cancleButton);

    vlayout->addLayout(layout);
    vlayout->addLayout(hlayout);
    vlayout->setSpacing(20);

    page1->setLayout(vlayout);

    tabWidget->addTab(page1, tr("常规"));
    setCentralWidget(tabWidget);

    connect(saveButton, &QPushButton::clicked, this, [this]() {
        emit saveSettings(idEdit->text().toUtf8(),
                          passwordEdit->text().toUtf8(),
                          trafficSpin->value());
    });
    connect(cancleButton, &QPushButton::clicked, this, &OptionsWindow::hide);
}
