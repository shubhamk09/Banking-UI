/**
 * @class MessageQueue
 * @brief Thread safe message queue
 */

 #pragma once

 #include <QObject>
 #include <QVector>
 #include <QMap>
 #include <QList>
 #include <QMutex>
 #include <QWaitCondition>
 #include <QThread>

 #include "../Interfaces/IMessage.hpp"

namespace Banking
{


class MessageQueue : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Get singleton instance
     * @return Reference to MessageQueue singleton
     */
    static MessageQueue& instance();

    /**
     * @brief Enqueue message for asynchronous processing
     *
     * Message is inserted into priority queue and processed by worker thread.
     * Returns immediately (Fire-and-Forget pattern).
     *
     * @param message Message to enqueue
     */
    void enqueueMessage(QSharedPointer<IMessage> message);

    /**
     * @brief Dequeue next message from queue
     *
     * Retrieves message with highest priority. Blocks if queue empty.
     *
     * @param message Output parameter for dequeued message
     * @param maxWaitMs Maximum wait time in milliseconds
     * @return true if message available, false if timeout
     */
    bool dequeueMessage(QSharedPointer<IMessage> &message, int maxWaitMs = 100);

    /**
     * @brief Send request and wait for response (Request-Reply pattern)
     *
     * Enqueues request, blocks until response received or timeout.
     * Response matched using message ID.
     *
     * @param request Request message to send
     * @param response Output parameter for response message
     * @param timeoutMs Maximum wait time (default 5000ms)
     * @return true if response received before timeout
     */
    bool sendRequestAndWait(QSharedPointer<IMessage> request,
                           QSharedPointer<IMessage> &response,
                           int timeoutMs = 5000);

    /**
     * @brief Complete a pending response when dequeued by dispatcher
     *
     * This is invoked by MessageDispatcher when a response message is
     * dequeued, ensuring that the requester only unblocks after queue
     * ordering is preserved.
     */
    bool completePendingResponse(QSharedPointer<IMessage> response);

    /**
     * @brief Subscribe to messages of specific type
     *
     * Uses MessageType enum for dispatcher routing.
     *
     * @param messageType Message type enum
     * @param receiver Qt object to receive signal
     * @param slotName Slot name to invoke
     */
    void subscribe(MessageType messageType, QObject *receiver,
                  const char *slotName);

    // Status queries
    int getQueueSize() const;
    int getProcessedMessageCount() const;
    int getFailedMessageCount() const;

    // Configuration
    void setMaxQueueSize(int size);
    //oid setAuditLogging(bool enabled, const QString &logPath);

signals:
    void messageEnqueued(QSharedPointer<IMessage> message);
    void messageProcessed(QSharedPointer<IMessage> message);
    void messageError(QSharedPointer<IMessage> message, const QString &error);

private slots:
    void processQueuedMessages();  ///< Worker thread processes messages

private:
    /**
     * @brief Private constructor for singleton pattern.
     *
     * Registers QSharedPointer<IMessage> meta type for Qt signals/slots.
     * Called when singleton instance is first created.
     */
    MessageQueue();
    MessageQueue(const MessageQueue&) = delete;
    MessageQueue& operator=(const MessageQueue&) = delete;

    mutable QMutex m_mutex;

    // Priority queue: higher priority = lower index
    QVector<QSharedPointer<IMessage>> m_priorityQueue;

    // For request-reply pattern: messageId -> response message
    QMap<QString, QSharedPointer<IMessage>> m_pendingResponses;
    QMap<QString, QWaitCondition*> m_waitConditions;  // Pointers (not by value)

    // Subscriptions: MessageType enum -> list of (receiver, slot)
    // Using enum key instead of QString for O(1) dispatcher lookup
    QMap<quint8, QList<QPair<QObject*, const char*>>> m_subscriptions;

    // Audit logging
    // QFile m_auditLog;
    // bool m_auditEnabled;

    // Statistics
    int m_processedCount;
    int m_failedCount;
    int m_maxQueueSize;

    // Worker thread
    QThread *m_workerThread;
};

} // namespace Banking