#ifndef SYSTEMTRAYMANAGER_H
#define SYSTEMTRAYMANAGER_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>

class MainWindow;

class SystemTrayManager : public QObject
{
    Q_OBJECT

public:
    explicit SystemTrayManager(MainWindow *window, QObject *parent = nullptr);
    void show();

private slots:
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void updateTooltip(const QString &text, const QString &reference);

private:
    QIcon createTrayIcon();
    
    MainWindow *m_window;
    QSystemTrayIcon *m_trayIcon;
    QMenu *m_trayMenu;
};

#endif // SYSTEMTRAYMANAGER_H
