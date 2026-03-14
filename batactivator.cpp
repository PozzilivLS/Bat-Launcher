#include "batactivator.h"

#include <shlobj_core.h>
#include <tlhelp32.h>
#include <windows.h>

#include <QFileDialog>
#include <QInputDialog>
#include <QLocalSocket>
#include <QMessageBox>
#include <QProcess>
#include <QPropertyAnimation>
#include <QRegularExpression>
#include <QStyle>
#include <sstream>

#include "BatParser/batparser.h"

BatActivator::BatActivator(QWidget *parent) : QMainWindow(parent) {
  ui.setupUi(this);

  setWindowFlags(this->windowFlags() | Qt::MSWindowsFixedSizeDialogHint);

  connectMenuBar();
  connect(ui.toggler, &QToolButton::clicked, this,
          &BatActivator::batBtnPressed);

  createTrayMenu();

  serviceManager_ = std::make_unique<ServiceManager>(ServiceManager());

  serviceManager_->setup();

  createAnimation();

  loadData();

  if (autostart_) {
    ui.autostartToggler->setChecked(true);
  }

  turnOffBat();

  if (autoTurnOn_) {
    ui.autoTurnOn->setChecked(true);

    turnOnBat();
  }
}

BatActivator::~BatActivator() {
  delete trayIcon_;
  delete batAnimationWidget;
  delete batFlyOnAnimation;
  delete batFlyOffAnimation;
}

void BatActivator::closeEvent(QCloseEvent *event) {
  if (!event->spontaneous()) {
    return;
  }

  if (isVisible()) {
    event->ignore();
    hide();
  }
}

void BatActivator::hideEvent(QHideEvent *event) {
  hide();

  if (!hideMessageShowed_) {
    trayIcon_->showMessage(
        "Bat Launcher",
        "The application is minimized to the system tray. To open the "
        "application window, click the application icon in the tray",
        QIcon(":/BatActivator/Icons/bat.ico"), 2000);
    hideMessageShowed_ = true;
  }
}

void BatActivator::batBtnPressed() {
  if (batState_) {
    turnOffBat();
  } else {
    turnOnBat();
  }
}

void BatActivator::batChoose() {
  QString path = QFileDialog::getOpenFileName(this, tr("Choose .bat"), "",
                                              tr("Bat (*.bat)"));

  if (!path.isEmpty()) {
    QStringList exeNames = createExeNames(path);

    QMessageBox msgBox;
    msgBox.setWindowTitle("Launched files");
    msgBox.setText(
        "After turn off, all processes with these names will be "
        "terminated:\n- " +
        exeNames.join("\n- "));
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Ok);
    int ret = msgBox.exec();

    if (ret == QMessageBox::Ok) {
      turnOffBat();

      batPath_ = path;
      exeNames_ = std::move(exeNames);

      saveData();
    }
  }
}

void BatActivator::autostartChanged(bool value) {
  if (value == autostart_) return;
  autostart_ = value;

  saveData();

  HKEY hKey;
  wchar_t szPath[MAX_PATH];
  GetModuleFileName(NULL, szPath, MAX_PATH);

  std::wstring path = std::wstring(szPath) + L" --autostart";

  RegCreateKeyEx(
      HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
      NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hKey, NULL);

  if (hKey) {
    if (value) {
      RegSetValueEx(hKey, L"BatLauncher", NULL, REG_SZ, (LPBYTE)path.data(),
                    (wcslen(path.data()) + 1) * sizeof(wchar_t));
    } else {
      RegDeleteValue(hKey, L"BatLauncher");
    }
    RegCloseKey(hKey);
  }
}

void BatActivator::autoTurnOnChanged(bool value) {
  if (value == autoTurnOn_) return;
  autoTurnOn_ = value;
  saveData();
}

void BatActivator::iconActivated(QSystemTrayIcon::ActivationReason reason) {
  switch (reason) {
    case QSystemTrayIcon::Trigger:
      if (!this->isVisible()) {
        this->show();
      }
      break;
    default:
      break;
  }
}

void BatActivator::exit() {
  turnOffBat();
  QApplication::quit();
}

void BatActivator::saveData() {
  QFile dataFile(QString::fromStdWString(getSaveFileName()));
  if (!dataFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
    return;
  }

  QTextStream out(&dataFile);
  out << batPath_ << "\n"
      << exeNames_.join('/') << "\n"
      << autoTurnOn_ << "\n"
      << autostart_;
}

void BatActivator::loadData() {
  QFile dataFile(QString::fromStdWString(getSaveFileName()));
  if (!dataFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return;
  }

  QTextStream in(&dataFile);
  batPath_ = in.readLine();
  exeNames_ = in.readLine().split('/');
  autoTurnOn_ = QVariant(in.readLine()).toBool();
  autostart_ = QVariant(in.readLine()).toBool();
}

bool BatActivator::turnOnBat() {
  if (batPath_.isEmpty()) {
    return false;
  }

  QString res = serviceManager_->sendCommand("RUN " + batPath_);

  if (res.compare("OK") != 0) {
    return false;
  }

  ui.info->setStyleSheet("color: green;");
  ui.info->setText("On");
  batFlyOnAnimation->start();

  batState_ = true;

  return true;
}

void BatActivator::turnOffBat() {
  if (exeNames_.isEmpty() && !batState_) {
    return;
  }

  ui.info->setStyleSheet("color: red;");
  ui.info->setText("Off");
  if (batState_) {
    batFlyOffAnimation->start();
  }

  serviceManager_->sendCommand("TERM " + exeNames_.join('/'));

  batState_ = false;
}

QStringList BatActivator::createExeNames(QString path) {
  BatParser p;
  return p.parse(path);
}

void BatActivator::createTrayMenu() {
  trayIcon_ = new QSystemTrayIcon(this);
  trayIcon_->setIcon(QIcon(":/BatActivator/Icons/bat.ico"));
  trayIcon_->setToolTip("Bat Launcher");

  QMenu *menu = new QMenu(this);
  QAction *viewWindow = new QAction(tr("Show"), this);
  QAction *quitAction = new QAction(tr("Exit"), this);

  connect(viewWindow, SIGNAL(triggered()), this, SLOT(show()));
  connect(quitAction, SIGNAL(triggered()), this, SLOT(exit()));

  menu->addAction(viewWindow);
  menu->addAction(quitAction);

  trayIcon_->setContextMenu(menu);
  trayIcon_->show();

  connect(trayIcon_, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this,
          SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
}

void BatActivator::connectMenuBar() {
  connect(ui.chooseBat, &QAction::triggered, this, &BatActivator::batChoose);
  connect(ui.autostartToggler, &QAction::toggled, this,
          &BatActivator::autostartChanged);
  connect(ui.autoTurnOn, &QAction::toggled, this,
          &BatActivator::autoTurnOnChanged);
}

void BatActivator::createAnimation() {
  batAnimationWidget = new RotatableWidget(this);
  batAnimationWidget->resize(300, 300);
  batAnimationWidget->move(1000, 1000);
  batAnimationWidget->setAttribute(Qt::WA_TransparentForMouseEvents);
  batAnimationWidget->show();

  QPropertyAnimation *onPosAnim =
      new QPropertyAnimation(batAnimationWidget, "pos", this);
  onPosAnim->setDuration(500);
  onPosAnim->setKeyValueAt(0, QPoint(-150, 340));
  onPosAnim->setKeyValueAt(0.2, QPoint(-100, 250));
  onPosAnim->setKeyValueAt(1, QPoint(-10, 65));
  onPosAnim->setEasingCurve(QEasingCurve::OutQuad);

  QPropertyAnimation *onRotAnim =
      new QPropertyAnimation(batAnimationWidget, "rotation", this);
  onRotAnim->setDuration(500);
  onRotAnim->setKeyValueAt(0, 20);
  onRotAnim->setKeyValueAt(0.2, 20);
  onRotAnim->setKeyValueAt(1, 45);
  onRotAnim->setEasingCurve(QEasingCurve::OutQuad);

  batFlyOnAnimation = new QParallelAnimationGroup;
  batFlyOnAnimation->addAnimation(onPosAnim);
  batFlyOnAnimation->addAnimation(onRotAnim);

  QPropertyAnimation *offPosAnim =
      new QPropertyAnimation(batAnimationWidget, "pos", this);
  offPosAnim->setDuration(500);
  offPosAnim->setKeyValueAt(0, QPoint(-10, 65));
  // offPosAnim->setKeyValueAt(0.2, QPoint(100, -200));
  offPosAnim->setKeyValueAt(1, QPoint(250, -50));
  offPosAnim->setEasingCurve(QEasingCurve::OutQuad);

  QPropertyAnimation *offRotAnim =
      new QPropertyAnimation(batAnimationWidget, "rotation", this);
  offRotAnim->setDuration(500);
  offRotAnim->setKeyValueAt(0, 45);
  offRotAnim->setKeyValueAt(1, 80);
  offRotAnim->setEasingCurve(QEasingCurve::OutQuad);

  batFlyOffAnimation = new QParallelAnimationGroup;
  batFlyOffAnimation->addAnimation(offPosAnim);
  batFlyOffAnimation->addAnimation(offRotAnim);
}

std::wstring BatActivator::getSaveFileName() {
  wchar_t *appdataFilePath = 0;
  SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &appdataFilePath);

  std::wstringstream ss;
  ss << appdataFilePath << L"\\BatLauncher";
  CoTaskMemFree(static_cast<void *>(appdataFilePath));

  std::wstring wstr = ss.str();

  std::filesystem::create_directory(wstr);

  ss << "\\data.txt";
  wstr = ss.str();

  return wstr;
}
