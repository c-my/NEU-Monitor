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
    idEdit->setPlaceholderText(tr("学(工)号"));

    passwordEdit = new QLineEdit(this);
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setContextMenuPolicy(Qt::NoContextMenu);
    passwordEdit->setText(pass);
    passwordEdit->setToolTip(tr("密码仅用于登陆"));
    passwordEdit->setPlaceholderText(tr("请输入密码"));

//    mobileCheck = new QCheckBox(tr("作为移动端登陆"), this);
//    mobileCheck->setChecked(isMobile);
//    mobileCheck->setToolTip(tr("实现PC多终端在线"));

    trafficBox = new QComboBox(this);
    trafficBox->setEditable(true);
    trafficBox->setCurrentText(QString::number(traffic));
    trafficBox->setValidator(new QIntValidator(0, INT_MAX));
    trafficBox->setToolTip(tr("0为无限制"));
    trafficBox->addItem(tr("5"));
    trafficBox->addItem(tr("25"));
    trafficBox->addItem(tr("60"));


    trafficLabel = new QLabel(tr("流量(G)"), this);
    trafficLabel->setToolTip(tr("在流量将要用尽时进行提醒"));

    layout->addRow(tr("用户名"), idEdit);
    layout->addRow(tr("密码"), passwordEdit);
    layout->addRow(trafficLabel, trafficBox);
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
                          trafficBox->currentText().toInt());
    });
    connect(cancleButton, &QPushButton::clicked, this, &OptionsWindow::hide);
}
