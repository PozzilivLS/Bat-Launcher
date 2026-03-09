#pragma once
#include <QString>

class ServiceManager {
 public:
  void setup();

  QString sendCommand(const QString &cmd);
 
 private:
  void startupService();
};
