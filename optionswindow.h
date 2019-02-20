#ifndef OPTIONSWINDOW_H
#define OPTIONSWINDOW_H

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include <QMainWindow>
#include <QWidget>
#include <QTabWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QSettings>

class OptionsWindow : public QMainWindow
{
  Q_OBJECT
public:
  explicit OptionsWindow(QByteArray id, QByteArray pass, int traffic, QWidget *parent = nullptr);

private:
  QTabWidget *tabWidget;
  QWidget *page1;
  QLineEdit *idEdit, *passwordEdit;
  QSpinBox *trafficSpin;
  QPushButton *saveButton, *cancleButton;
  QFormLayout *layout;
  QVBoxLayout *vlayout;
  QHBoxLayout *hlayout;
  QCheckBox *mobileCheck;

  QLabel *trafficLabel;

signals:
  void saveSettings(QByteArray id, QByteArray pass, int traffic);
public slots:
};

#endif // OPTIONSWINDOW_H
