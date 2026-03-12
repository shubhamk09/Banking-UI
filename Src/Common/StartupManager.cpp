/**
 * 
 */
#include <iostream>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "inc/StartupManager.hpp"
#include "inc/ModuleNames.hpp"
#include "inc/ModuleFactory.hpp"
#include "../Modules/Communications/inc/CommunicationInitializer.hpp"

namespace Banking{
    constexpr int initModule = 1;
    constexpr int doNotInitModule = 0;

StartupManager::StartupManager() : m_moduleFactory(ModuleFactory::instance()),
    m_moduleNameMap{
        {"Communications", ModuleNames::Communications}
        // Add more mappings as needed
    },
    m_totalModules{0},
    m_totalLoadedModules{0}
{
    std::cout<<"Starting the Project"<<std::endl;
    registerModule();
    loadConfig();
    initCore();
    initModules();
}

StartupManager::~StartupManager()
{
    std::cout<<"Shuting down the Project"<<std::endl;
    for (auto &module : m_modules)
    {
        module->stop();
    }
    m_modules.clear();
}

void StartupManager::loadConfig()
{

    std::cout<< "Loading Config files" <<std::endl;

    QFile file(":/json/modules_list"); // Reading from resource path
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isNull() && doc.isObject()) {
            QJsonObject jsonObj = doc.object();

            QJsonArray moduleList = jsonObj["modules"].toArray();
            std::cout<<"Module list size: "<<moduleList.size()<<std::endl;
            for (const QJsonValue &moduleValue : moduleList)
            {
                if(!moduleValue.isObject())
                {
                    std::cout<< "Inavalid data in Module lists" <<std::endl;
                    return;
                }

                QJsonObject moduleObject = moduleValue.toObject();
                for (auto it = moduleObject.begin(); it != moduleObject.end(); ++it) {
                    QString moduleName = it.key();
                    int moduleValue = it.value().toInt();
                    std::cout<<"Module: "<< moduleName.toStdString() << " is " << moduleValue << std::endl;
                    if (moduleValue == initModule)
                    {
                        QString initializerName = getInitializerName(moduleName);
                        std::unique_ptr<IModule> module = m_moduleFactory.createInstance(initializerName);
                        // How should I replace .take() its depreciated??
                        // QT suggests to use std::unique_ptr and release() function.
                        // Need to analyse that.
                        m_modules.emplace_back(std::move(module));
                        m_totalModules++;
                    }
                }
            }          
        }
    }
    else
    {
        std::cout<<"qrc path failed"<<std::endl;
    }
    
}
void StartupManager::initCore()
{
    // Dont know what do do here.
    // May be initializing other things are meesage queues, loggers and all
}
void StartupManager::initModules()
{
    for (auto &module : m_modules)
    {
        if(module->init())
        {
            if(module->start())
            {
                std::cout<<module->getModuleName()<<" Started"<<std::endl;
                m_totalLoadedModules++;
            }
        }
        
    }
    
}
void StartupManager::registerModule()
{
    m_moduleFactory.registerModule(ModuleNames::Communications,
                        []() { return std::unique_ptr<IModule>(new Communications::CommunicationInitializer); });
}
QString StartupManager::getInitializerName(const QString& jsonName) const {
    auto it = m_moduleNameMap.find(jsonName);
    if (it != m_moduleNameMap.end())
        return it->second;
    return jsonName;
}
} // namespace Banking