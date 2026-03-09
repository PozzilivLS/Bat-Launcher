  #pragma once

#include <QtWidgets/QMainWindow>
#include <QCloseEvent>
#include <QSystemTrayIcon>
#include <QAction>
#include <memory>
#include <QParallelAnimationGroup>

#include "ui_batactivator.h"
#include "Service/serviceManager.h"
#include <RotatableWidget/rotatablewidget.h>

class QProcess;

class BatActivator : public QMainWindow {
  Q_OBJECT

 public:
  BatActivator(QWidget *parent = nullptr);
  ~BatActivator();

 protected:
  void closeEvent(QCloseEvent *event);

 private slots:
  void batBtnPressed();
  void batChoose();

  void autostartChanged(bool value);
  void autoTurnOnChanged(bool value);

  void iconActivated(QSystemTrayIcon::ActivationReason reason);

 private:
  void createTrayMenu();
  void connectMenuBar();

  void createAnimation();

  std::wstring getSaveFileName();

  void saveData();
  void loadData();

  bool turnOnBat();
  void turnOffBat();

  QStringList createExeNames(QString path);

  Ui::BatActivatorClass ui;

  QString batPath_;
  QStringList exeNames_;

  bool batState_ = false;

  bool autoTurnOn_ = false;
  bool autostart_ = false;

  QSystemTrayIcon *trayIcon_;

  std::unique_ptr<ServiceManager> serviceManager_;

  RotatableWidget *batAnimationWidget = nullptr;
  QParallelAnimationGroup *batFlyOnAnimation = nullptr;
  QParallelAnimationGroup *batFlyOffAnimation = nullptr;
};
