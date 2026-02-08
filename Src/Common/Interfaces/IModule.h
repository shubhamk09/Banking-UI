/**
 * @class IModule
 * @brief Interface for application modules.
 *
 * Provides lifecycle management for modules, including initialization,
 * startup, and shutdown. All modules should implement this interface.
 */

 // Includes
 #include <string>

class IModule
{
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~IModule() = default;

    /**
     * @brief Returns module name.
     * @return Name of the module.
     */
    virtual const std::string& getModuleName() const = 0;
    /**
     * @brief Initialize the module.
     *
     * Allocate resources, read configuration, and validate setup.
     * @return True if initialization succeeds, false otherwise.
     */
    virtual bool init() = 0;

    /**
     * @brief Start the module.
     *
     * Start threads, timers, or IPC mechanisms required by the module.
     * @return True if startup succeeds, false otherwise.
     */
    virtual bool start() = 0;

    /**
     * @brief Stop the module.
     *
     * Stop threads, timers, and free resources.
     */
    virtual void stop() = 0;
};