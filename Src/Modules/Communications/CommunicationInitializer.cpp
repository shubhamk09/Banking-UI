
#include <iostream>
#include <string>

#include "CommunicationInitializer.hpp"
#include "../../Common/inc/ModuleNames.hpp"
namespace Banking {

namespace Communications {

const char* CommunicationInitializer::getModuleName() const
{
    return ModuleNames::Communications;
}

bool CommunicationInitializer::init()
{
    std::cout<<"Initializing Communication module"<<std::endl;
    return false;
}

bool CommunicationInitializer::start()
{
    return false;
}

void CommunicationInitializer::stop()
{
}

} // namespace Communications
} // namespace Banking