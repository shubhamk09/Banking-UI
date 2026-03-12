#include <QString>
#include <iostream>
#include "inc/ModuleFactory.hpp"

namespace Banking
{
ModuleFactory &Banking::ModuleFactory::instance()
{
    static ModuleFactory factory;
    return factory;
}

void ModuleFactory::registerModule(const QString &name, CreatorFunction creator)
{
    std::cout<<"Registering module: " << name.toStdString()<<std::endl;
    m_creatorsList[name] = std::move(creator);
    std::cout<<"m_creatorsList size " << m_creatorsList.size()<<std::endl;
}
std::unique_ptr<IModule> ModuleFactory::createInstance(const QString &name) const
{
    std::cout<<"Creating the instance for: "<<name.toStdString()<<std::endl;
    auto module = m_creatorsList.find(name);
    // Not going here... why????
    
    if(module == m_creatorsList.end())
    {
        return std::unique_ptr<IModule>(nullptr);
    }
    std::cout<<"Found the module"<<std::endl;
    return module.value()();
}
} // namespace Banking