#pragma once

#include <QObject>

namespace Banking
{
class MessageDispatcher : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param queue Reference to MessageQueue for subscribing
     */
    explicit MessageDispatcher(MessageQueue &queue);

    /**
     * @brief Register handler for specific message type
     *
     * Uses MessageType enum for fast dispatcher routing.
     *
     * @param messageType Message type enum (not string!)
     * @param handler Qt object to receive messages
     * @param slotName Slot method name (e.g., SLOT(onAuthMessage(QSharedPointer<IMessage>)))
     */
    void registerHandler(MessageType messageType,
                        QObject *handler,
                        const char *slotName);

    /**
     * @brief Register all handlers for a module
     *
     * @param module IModule instance with handler slots
     */
    void registerModuleHandler(IModule *module);

    /**
     * @brief Start dispatcher on worker thread
     */
    void start();

    /**
     * @brief Stop dispatcher
     */
    void stop();

private slots:
    /**
     * @brief Process next message from queue
     *
     * Called internally by worker thread dispatcher loop.
     * Uses enum comparison (O(1)) to route based on message type.
     */
    void onMessageAvailable(QSharedPointer<IMessage> message);

private:
    MessageQueue &m_queue;

    // Route: MessageType enum (as quint8) -> list of handler slots
    // Using enum instead of QString for O(1) dispatcher performance
    QMap<quint8, QList<QPair<QObject*, const char*>>> m_handlers;

    QThread *m_dispatcherThread;
    bool m_running;
};
}