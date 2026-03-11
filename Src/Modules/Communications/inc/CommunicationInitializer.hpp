#include "../../../Common/Interfaces/IModule.hpp"

namespace Banking {

namespace Communications {

class CommunicationInitializer: public IModule
{

public:
    CommunicationInitializer(/* args */) = default;
    ~CommunicationInitializer() = default;

    const char* getModuleName() const override;
    bool init() override;
    bool start() override;
    void stop() override;

private:
    std::string m_name;
};

} // namespace Communications
} // namespace Banking
