#include "serviceManager.h"

#include <shlobj_core.h>

#include <QLocalSocket>

void ServiceManager::setup() {
  QString test = sendCommand("PING");

  if (test.compare("FAILED") == 0) {
    startupService();
  }
}

QString ServiceManager::sendCommand(const QString &cmd) {
  QLocalSocket socket;
  socket.connectToServer("BatLauncherServicePipe");

  if (!socket.waitForConnected(3000)) {
    qDebug() << socket.errorString();
    qDebug() << "Connection failed";
    return QString("FAILED");
  }

  socket.write(cmd.toUtf8());
  socket.flush();

  if (socket.waitForReadyRead(3000)) {
    QByteArray response = socket.readAll();
    qDebug() << "Response: " << response;

    return QString(response);
  }

  socket.disconnectFromServer();

  return QString();
}

void ServiceManager::startupService() {
  wchar_t szPath[MAX_PATH] = {};
  GetModuleFileNameW(NULL, szPath, MAX_PATH);

  std::wstring wstr = szPath;
  size_t last = wstr.find_last_of(L"\\", wstr.size());
  wstr = wstr.substr(0, last);

  qDebug() << (wstr + L"\\BatLauncherWinService.exe");

  std::wstring arg =
      L"/c sc create BatLauncherService binPath= \"" + wstr +
      L"\\BatLauncherWinService.exe\" "
      L"start= auto obj= LocalSystem & sc start BatLauncherService"; // TODO: replace

  ShellExecuteW(NULL, L"runas", L"cmd.exe", arg.c_str(), NULL, SW_NORMAL);
}