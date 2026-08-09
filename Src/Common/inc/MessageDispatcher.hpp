#pragma once

#include <QObject>
#include <QMap>
#include <QThread>
#include <QList>
#include <QPair>
#include "MessageQueue.hpp"
#include "IMessage.hpp"

namespace Banking {

class IModule;

/**
 * @class MessageDispatcher
 * @brief Routes messages from queue to appropriate module handlers
 *
 * Processes queued messages and dispatches them to registered handlers
 * based on message type. Handles both fire-and-forget and request-reply
 * patterns with priority ordering maintained.
 */
class MessageDispatcher : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param queue Reference to MessageQueue for subscribing
     * @param parent Optional parent QObject
     */
    explicit MessageDispatcher(MessageQueue &queue, QObject *parent = nullptr);

    /**
     * @brief Destructor
     */
    ~MessageDispatcher();

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
     * @brief Process all the message from queue
     *
     * Called internally by worker thread dispatcher loop.
     * Uses enum comparison (O(1)) to route based on message type.
     */
    void onMessageAvailable();

private:
    /**
     * @brief Route message to all registered handlers for its type
     */
    void routeMessage(QSharedPointer<IMessage> message);

    /** @brief Reference to MessageQueue singleton */
    MessageQueue &m_queue;

    /** @brief Route: MessageType enum (as quint8) -> list of handler slots */
    QMap<quint8, QList<QPair<QObject*, const char*>>> m_handlers;

    /** @brief Worker thread for message processing */
    QThread *m_dispatcherThread;

    /** @brief Flag indicating if dispatcher is running */
    bool m_running;
};

} // namespace Banking
