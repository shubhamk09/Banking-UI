
#include <iostream>
#include <string>

#include "CommunicationInitializer.hpp"
#include "../../Common/inc/ModuleNames.hpp"
namespace Banking {

namespace Communications {

constexpr const char* SERVERADDRESS {"127.0.0.1"};
constexpr const int SERVERPORT      {5020};

const char* CommunicationInitializer::getModuleName() const
{
    return ModuleNames::Communications;
}

bool CommunicationInitializer::init()
{
    bool returnStatus{false};
    std::cout<<"Initializing Communication module"<<std::endl;
    m_socket = new BankingSocket();
    if(m_socket!=nullptr)
        returnStatus = true;
    return returnStatus;
}

bool CommunicationInitializer::start()
{
    bool returnStatus{false};
    if(m_socket->connectToServer(SERVERADDRESS, SERVERPORT))
        returnStatus = true;
    return returnStatus;
}

void CommunicationInitializer::stop()
{
    std::cout<<"Stoping Communication module"<<std::endl;
    m_socket->disconnectFromServer();
    delete m_socket;
}

} // namespace Communications
} // namespace Banking