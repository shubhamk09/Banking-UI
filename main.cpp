#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "Src/Common/inc/StartupManager.hpp"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("Banking-UI", "Main");
    QScopedPointer<banking::StartupManager> startupManager(new banking::StartupManager()); 
    return app.exec();
}
