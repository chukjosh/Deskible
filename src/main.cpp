#include <QApplication>
#include "mainwindow.h"
#include "systemtraymanager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    app.setApplicationName("Deskible");
    app.setOrganizationName("Deskible");
    app.setApplicationVersion("1.0.0");
    app.setQuitOnLastWindowClosed(false);

    MainWindow *mainWindow = new MainWindow();
    SystemTrayManager trayManager(mainWindow);

    mainWindow->show();
    trayManager.show();

    return app.exec();
}
