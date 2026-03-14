#include <windows.h>

#include <QProcess>
#include <QtWidgets/QApplication>

#include "batactivator.h"

void checkWindow();

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  BatActivator window;

  bool autostart = app.arguments().contains("--autostart");

  HANDLE hMutex = OpenMutexW(MUTEX_ALL_ACCESS, 0, L"BatLauncherMutex");
  if (!hMutex)
    hMutex = CreateMutexW(0, 0, L"BatLauncherMutex");
  else {
    HWND hWnd = FindWindowW(0, L"BatLauncherMutex");
    SetForegroundWindow(hWnd);
    ShowWindow(hWnd, SW_SHOW);
    return 0;
  }

  if (!autostart) window.show();

  app.exec();

  ReleaseMutex(hMutex);
  CloseHandle(hMutex);

  return 0;
}
