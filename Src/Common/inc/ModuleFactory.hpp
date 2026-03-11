#pragma once
#include <QScopedPointer>
#include <QHash>

#include "../Interfaces/IModule.hpp"
namespace Banking
{

class ModuleFactory
{

private:
    /* data */
public:
    ModuleFactory(/* args */) = default;
    ~ModuleFactory() = default;

    using CreatorFunction = std::function<QScopedPointer<IModule>()>;

    static ModuleFactory& instance();
    void registerModule(const QString& name, CreatorFunction creator);
    QScopedPointer<IModule> createInstance(const QString& name) const;

    private:
    QHash<QString, CreatorFunction> m_creatorsList;
};

} // namespace Banking