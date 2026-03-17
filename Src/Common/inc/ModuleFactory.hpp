/**
 * @file ModuleFactory.hpp
 * @brief Factory pattern implementation for creating module instances.
 */

#pragma once
#include <QScopedPointer>
#include <QHash>

#include "../Interfaces/IModule.hpp"

namespace Banking
{

/**
 * @class ModuleFactory
 * @brief Factory for creating and managing module instances.
 *
 * Implements the factory pattern to register and instantiate module implementations.
 * Uses the singleton pattern to ensure only one factory instance exists in the application.
 */
class ModuleFactory
{
private:
    /* data */
public:
    /**
     * @brief Constructor.
     */
    ModuleFactory(/* args */) = default;

    /**
     * @brief Destructor.
     */
    ~ModuleFactory() = default;

    /**
     * @typedef CreatorFunction
     * @brief Function pointer type for creating module instances.
     */
    using CreatorFunction = std::function<std::unique_ptr<IModule>()>;

    /**
     * @brief Get singleton instance of the factory.
     * @return Reference to the singleton ModuleFactory instance.
     */
    static ModuleFactory& instance();

    /**
     * @brief Register a module with the factory.
     * @param name The name to register the module under.
     * @param creator The creator function that instantiates the module.
     */
    void registerModule(const QString& name, CreatorFunction creator);

    /**
     * @brief Create an instance of a registered module.
     * @param name The name of the module to create.
     * @return Unique pointer to the created IModule instance, or nullptr if not found.
     */
    std::unique_ptr<IModule> createInstance(const QString& name) const;

private:
    /**
     * @brief Registry of module creator functions.
     */
    QHash<QString, CreatorFunction> m_creatorsList;
};

} // namespace Banking