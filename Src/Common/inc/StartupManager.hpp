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
    QString getInitializerName(const QString& jsonName) const;

    std::unordered_map<QString, QString> m_moduleNameMap;
    std::vector<std::unique_ptr<IModule>> m_modules;
    ModuleFactory& m_moduleFactory;
    u_int8_t m_totalModules;
    u_int8_t m_totalLoadedModules;

    
    
};

} // namespace Banking