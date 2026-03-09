#include "batactivator.h"
#include <QtWidgets/QApplication>
#include <QProcess>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    BatActivator window;
    window.show();

    return app.exec();
}
