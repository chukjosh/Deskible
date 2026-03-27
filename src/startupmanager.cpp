#include "startupmanager.h"
#include <QCoreApplication>
#include <QSettings>
#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QTextStream>

#ifdef Q_OS_WIN
#define REG_RUN "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"
#endif

bool StartupManager::isStartupEnabled()
{
    QString appPath = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
    QString appName = "Deskible";

#ifdef Q_OS_WIN
    QSettings settings(REG_RUN, QSettings::NativeFormat);
    return settings.value(appName).toString() == appPath;
#elif defined(Q_OS_MAC)
    QString plistPath = QDir::homePath() + "/Library/LaunchAgents/com.deskible.app.plist";
    return QFile::exists(plistPath);
#else // Linux
    QString desktopPath = QDir::homePath() + "/.config/autostart/deskible.desktop";
    return QFile::exists(desktopPath);
#endif
}

bool StartupManager::setStartupEnabled(bool enable)
{
    QString appPath = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
    QString appName = "Deskible";

#ifdef Q_OS_WIN
    QSettings settings(REG_RUN, QSettings::NativeFormat);
    if (enable)
        settings.setValue(appName, appPath);
    else
        settings.remove(appName);
    return true;
#elif defined(Q_OS_MAC)
    QString plistPath = QDir::homePath() + "/Library/LaunchAgents/com.deskible.app.plist";
    if (enable) {
        QFile file(plistPath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                << "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
                << "<plist version=\"1.0\">\n<dict>\n"
                << "\t<key>Label</key>\n\t<string>com.deskible.app</string>\n"
                << "\t<key>ProgramArguments</key>\n\t<array>\n"
                << "\t\t<string>" << appPath << "</string>\n"
                << "\t</array>\n"
                << "\t<key>RunAtLoad</key>\n\t<true/>\n"
                << "</dict>\n</plist>";
            return true;
        }
    } else {
        return QFile::remove(plistPath);
    }
#else // Linux
    QString autostartDir = QDir::homePath() + "/.config/autostart";
    QDir().mkpath(autostartDir);
    QString desktopPath = autostartDir + "/deskible.desktop";
    if (enable) {
        QFile file(desktopPath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << "[Desktop Entry]\n"
                << "Type=Application\n"
                << "Name=Deskible\n"
                << "Exec=" << appPath << "\n"
                << "Hidden=false\n"
                << "NoDisplay=false\n"
                << "X-GNOME-Autostart-enabled=true\n";
            return true;
        }
    } else {
        return QFile::remove(desktopPath);
    }
#endif
    return false;
}
