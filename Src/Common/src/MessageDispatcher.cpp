/**
 * @file MessageDispatcher.cpp
 * @brief Implementation of message dispatcher and router
 */

#include "MessageDispatcher.hpp"
#include "IModule.hpp"
#include <QMetaObject>
#include <QDebug>

namespace Banking {

/**
 * @brief Constructor
 */
MessageDispatcher::MessageDispatcher(MessageQueue &queue, QObject *parent)
    : QObject(parent),
      m_queue(queue),
      m_dispatcherThread(nullptr),
      m_running(false) {

    // Connect to queue's messageAvailable signal
    connect(&m_queue, &MessageQueue::messageAvailable, this,
            &MessageDispatcher::onMessageAvailable, Qt::QueuedConnection);
}

/**
 * @brief Destructor
 */
MessageDispatcher::~MessageDispatcher() {
    stop();
}

/**
 * @brief Register handler for a specific message type
 */
void MessageDispatcher::registerHandler(MessageType messageType,
                                       QObject *handler,
                                       const char *slotName) {
    if (!handler || !slotName) {
        qWarning() << "Invalid handler or slot name";
        return;
    }

    quint8 typeKey = static_cast<quint8>(messageType);

    if (!m_handlers.contains(typeKey)) {
        m_handlers[typeKey] = QList<QPair<QObject*, const char*>>();
    }

    m_handlers[typeKey].append({handler, slotName});

    qDebug() << "Handler registered for message type:" << static_cast<int>(messageType);
}

/**
 * @brief Register all handlers for a module
 */
void MessageDispatcher::registerModuleHandler(IModule *module) {
    if (!module) {
        qWarning() << "Null module pointer";
        return;
    }

    // Modules register their handlers in their start() method
    // This is a convenience stub for future extension
    qDebug() << "Module handler registration: " << module->getModuleName();
}

/**
 * @brief Start dispatcher on worker thread
 */
void MessageDispatcher::start() {
    if (m_running) {
        qWarning() << "Dispatcher already running";
        return;
    }

    m_running = true;

    // Move dispatcher to worker thread
    m_dispatcherThread = new QThread(this);
    moveToThread(m_dispatcherThread);

    // Thread is running so queued signals can be delivered.
    connect(m_dispatcherThread, &QThread::started, this, [this]() {
        qDebug() << "MessageDispatcher started on worker thread";
    });

    // Cleanup on thread finished
    connect(m_dispatcherThread, &QThread::finished, this, [this]() {
        qDebug() << "MessageDispatcher thread finished";
        m_running = false;
    });

    m_dispatcherThread->start();
}

/**
 * @brief Stop dispatcher
 */
void MessageDispatcher::stop() {
    if (!m_running || !m_dispatcherThread) {
        return;
    }

    m_running = false;

    if (m_dispatcherThread->isRunning()) {
        m_dispatcherThread->quit();
        m_dispatcherThread->wait();
    }

    qDebug() << "MessageDispatcher stopped";
}

/**
 * @brief Handle new message from queue
 */
void MessageDispatcher::onMessageAvailable() {
    // This is called when message is enqueued.
    // The dispatcher thread is already running, so dequeue and route here.
    QSharedPointer<IMessage> message;
    while (m_queue.dequeueMessage(message)) {
        routeMessage(message);
    }
    emit m_queue.allQueueItemProcessed();
}

/**
 * @brief Route message to all registered handlers
 */
void MessageDispatcher::routeMessage(QSharedPointer<IMessage> message) {
    if (!message) {
        qWarning() << "Null message received";
        emit m_queue.messageError(message, "Null message");
        return;
    }

    if (!message->isRequest() && m_queue.completePendingResponse(message)) {
        qDebug() << "Response consumed for pending request:" << message->getMessageId();
        emit m_queue.messageProcessed(message);
        return;
    }

    // Get message type
    MessageType messageType = message->getMessageType();
    quint8 typeKey = static_cast<quint8>(messageType);

    // Find handlers for this message type
    if (!m_handlers.contains(typeKey)) {
        qDebug() << "No handlers registered for message type:" << static_cast<int>(messageType);
        emit m_queue.messageError(message, "No handlers registered");
        return;
    }

    // Invoke all registered handlers for this message type
    const auto& handlers = m_handlers[typeKey];
    bool anySuccess = false;

    for (const auto& [receiver, slotName] : handlers) {
        if (!receiver) {
            qWarning() << "Handler receiver is null";
            continue;
        }

        // Invoke slot with message as parameter
        bool invokeSuccess = QMetaObject::invokeMethod(
            receiver,
            slotName,
            Qt::QueuedConnection,
            Q_ARG(QSharedPointer<IMessage>, message)
        );

        if (!invokeSuccess) {
            qWarning() << "Failed to invoke handler slot:" << slotName;
        } else {
            qDebug() << "Message routed to handler for type:" << static_cast<int>(messageType);
            anySuccess = true;
        }
    }

    if (anySuccess) {
        emit m_queue.messageProcessed(message);
    } else {
        emit m_queue.messageError(message, "No successful handler invocation");
    }
}

} // namespace Banking
