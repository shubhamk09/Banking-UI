
#include <QScopedPointer>
#include "../Interfaces/IModule.h"

namespace banking {

class StartupManager
{
public:
    StartupManager(/* args */);
    ~StartupManager();

    void startUp();
    void shutDown();

private:
    void loadConfig();
    void initCore();
    void initModules();
    void registerModules(const std::string& moduleName);
    std::vector<QScopedPointer<IModule>> modules;
};

} // namespace banking