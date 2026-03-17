/**
 * @file CommunicationInitializer.hpp
 * @brief Communication module initializer implementing the IModule interface.
 */

#include "../../../Common/Interfaces/IModule.hpp"
#include "BankingSocket.hpp"

namespace Banking {

namespace Communications {

/**
 * @class CommunicationInitializer
 * @brief Implements the Communications module for TCP socket-based banking communication.
 *
 * Manages the lifecycle of the banking socket communication layer,
 * handling initialization, startup, and shutdown of network connectivity.
 */
class CommunicationInitializer: public IModule
{

public:
    /**
     * @brief Constructor.
     */
    CommunicationInitializer(/* args */) = default;

    /**
     * @brief Destructor.
     */
    ~CommunicationInitializer() = default;

    /**
     * @brief Get the module name.
     * @return C-string containing the module name.
     */
    const char* getModuleName() const override;

    /**
     * @brief Initialize the Communications module.
     *
     * Allocates and sets up the BankingSocket instance.
     * @return True if initialization succeeds, false otherwise.
     */
    bool init() override;

    /**
     * @brief Start the Communications module.
     *
     * Establishes connection to the banking server.
     * @return True if startup and connection succeeds, false otherwise.
     */
    bool start() override;

    /**
     * @brief Stop the Communications module.
     *
     * Disconnects from the server and cleans up resources.
     */
    void stop() override;

private:
    /** @brief Module name identifier. */
    std::string m_name;

    /** @brief Socket instance for TCP communication. */
    BankingSocket* m_socket;
};

} // namespace Communications
} // namespace Banking
