#ifndef OPTIONSWINDOW_H
#define OPTIONSWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QTabWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QSettings>
#include <QComboBox>

class OptionsWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit OptionsWindow(QByteArray id, QByteArray pass, int traffic, QWidget *parent = nullptr);

private:
    QTabWidget *tabWidget;
    QWidget *page1;
    QComboBox *idCombo;
    QLineEdit *passwordEdit;
    QSpinBox *trafficSpin;
    QPushButton *saveButton, *cancleButton;
    QFormLayout *layout;
    QVBoxLayout *vlayout;
    QHBoxLayout *hlayout;

    QLabel *trafficLabel;

signals:
    void saveSettings(QByteArray id, QByteArray pass, int traffic);
public slots:
};

#endif // OPTIONSWINDOW_H
