#ifndef STARTUPMANAGER_H
#define STARTUPMANAGER_H

#include <QString>

class StartupManager
{
public:
    static bool isStartupEnabled();
    static bool setStartupEnabled(bool enable);

private:
    StartupManager() = delete; // Static only
};

#endif // STARTUPMANAGER_H
