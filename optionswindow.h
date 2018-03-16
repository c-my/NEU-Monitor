#ifndef OPTIONSWINDOW_H
#define OPTIONSWINDOW_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QSettings>

class OptionsWindow : public QWidget
{
    Q_OBJECT
public:
    explicit OptionsWindow(QByteArray id, QByteArray pass, QWidget *parent = nullptr);

private:
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
