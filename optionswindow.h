#ifndef OPTIONSWINDOW_H
#define OPTIONSWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QTabWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QSettings>

class OptionsWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit OptionsWindow(QByteArray id, QByteArray pass, QWidget *parent = nullptr);

private:
    QTabWidget *tabWidget;
    QWidget *page1;
    QLineEdit *idEdit, *passwordEdit;
    QPushButton *saveButton, *cancleButton;
    QFormLayout *layout;
    QVBoxLayout *vlayout;
    QHBoxLayout *hlayout;

signals:
    void saveSettings(QByteArray id, QByteArray pass);
public slots:
};

#endif // OPTIONSWINDOW_H
