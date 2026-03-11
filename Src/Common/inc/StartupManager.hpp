#pragma once

#include <QScopedPointer>
#include <QList>
#include <iostream>
#include "../Interfaces/IModule.hpp"

namespace Banking {

class ModuleFactory;
class StartupManager
{
public:
    StartupManager(/* args */);
    ~StartupManager();

    void startUp();
    void shutDown();

private:
    void loadConfig();
    void initCore();
    void initModules();
    void registerModule();
    QList<IModule*> m_modules;
    ModuleFactory& m_moduleFactory;

    // Map JSON module names to initializer class names
    std::unordered_map<QString, QString> m_moduleNameMap;
    QString getInitializerName(const QString& jsonName) const;
};

} // namespace Banking