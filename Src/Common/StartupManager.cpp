/**
 * 
 */
#include <iostream>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "inc/StartupManager.hpp"

namespace banking{

StartupManager::StartupManager(){
    std::cout<<"Starting the Project"<<std::endl;
    loadConfig();
}

StartupManager::~StartupManager(){
    std::cout<<"Shuting down the Project"<<std::endl;
}

void StartupManager::loadConfig()
{

    std::cout<< "Loading Config files" <<std::endl;
    QFile file(":/json/modules_list"); // Your resource path
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isNull() && doc.isObject()) {
            QJsonObject obj = doc.object();
            for (auto it = obj.constBegin(); it != obj.constEnd(); ++it)
            {
                if (it.key() == "modules")
                {
                    QJsonArray modulesList = it.value().toArray();
                    std::cout<<modulesList.size()<<std::endl;
                    // Need to write logic to call for initialization of the modules. 
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
}
void StartupManager::initModules()
{
}
void StartupManager::registerModules(const std::string &moduleName)
{
}
} // namespace banking