/**
 * @file StartupManager.hpp
 * @brief Manages application startup and module initialization.
 */

#pragma once

#include <QScopedPointer>
#include <QList>
#include <iostream>
#include "../Interfaces/IModule.hpp"
#include "MessageDispatcher.hpp"

namespace Banking {

class ModuleFactory;

/**
 * @class StartupManager
 * @brief Orchestrates application startup and module lifecycle management.
 *
 * Responsible for loading module configuration, registering modules,
 * initializing them, and managing their startup/shutdown sequences.
 */
class StartupManager
{
public:
    /**
     * @brief Constructor.
     *
     * Initializes the startup manager and begins the startup sequence:
     * - Registers modules
     * - Loads configuration
     * - Initializes core systems
     * - Initializes and starts all modules
     */
    StartupManager(/* args */);

    /**
     * @brief Destructor.
     *
     * Shuts down all running modules and cleans up resources.
     */
    ~StartupManager();

    /**
     * @brief Start the application.
     *
     * Can be called to trigger startup sequence after construction.
     */
    void startUp();

    /**
     * @brief Shut down the application.
     *
     * Stops all modules and cleans up resources.
     */
    void shutDown();

private:
    /**
     * @brief Load module configuration from JSON resource.
     *
     * Reads module configuration from the embedded JSON resource file
     * and instantiates modules based on their enabled status.
     */
    void loadConfig();

    /**
     * @brief Initialize core application systems.
     *
     * Initializes systems like logging, message queues, and other
     * core infrastructure needed by modules.
     */
    void initCore();

    /**
     * @brief Initialize and start all loaded modules.
     *
     * Calls init() and start() on each module, tracking successful initializations.
     */
    void initModules();

    /**
     * @brief Register all available modules with the factory.
     *
     * Registers module creators with the ModuleFactory to enable
     * dynamic module instantiation.
     */
    void registerModule();

    /**
     * @brief Get the initializer class name for a module from configuration name.
     * @param jsonName The module name as specified in JSON configuration.
     * @return The corresponding initializer class name.
     */
    QString getInitializerName(const QString& jsonName) const;

    /** @brief Map of JSON module names to their initializer class names. */
    std::unordered_map<QString, QString> m_moduleNameMap;

    /** @brief List of loaded and instantiated modules. */
    std::vector<std::unique_ptr<IModule>> m_modules;

    /** @brief Reference to the module factory singleton. */
    ModuleFactory& m_moduleFactory;

    /** @brief Total number of modules in configuration. */
    u_int8_t m_totalModules;

    /** @brief Number of successfully loaded modules. */
    u_int8_t m_totalLoadedModules;

    /** @brief Message dispatcher for handling the message in messageQueue */
    QScopedPointer<Banking::MessageDispatcher> m_messageDispatcher;

};

} // namespace Banking