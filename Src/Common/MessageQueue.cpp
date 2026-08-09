#include <QMutexLocker>
#include <QMetaType>
#include <QDebug>

#include "inc/MessageQueue.hpp"

namespace Banking
{

MessageQueue::MessageQueue()
    : m_processingScheduled(false),
      m_processedCount(0),
      m_failedCount(0),
      m_maxQueueSize(0),
      m_workerThread(nullptr)
{
    // Register meta type for Qt signals/slots with smart pointers
    qRegisterMetaType<QSharedPointer<IMessage>>("QSharedPointer<Banking::IMessage>");

    connect(this, &MessageQueue::messageProcessed,
            this, &MessageQueue::onMessageProcessed);

    connect(this, &MessageQueue::messageError,
            this, &MessageQueue::onMessageError);

    connect(this, &MessageQueue::allQueueItemProcessed,
            this, &MessageQueue::onAllQueueItemProcessed);
}

MessageQueue &MessageQueue::instance()
{
    static MessageQueue instance;
    return instance;
}

void MessageQueue::enqueueMessage(QSharedPointer<IMessage> message)
{
    bool dispatchSignal = false;
    bool emitErrorSignal = false;
    QSharedPointer<IMessage> errorMessage;

    {
        QMutexLocker locker(&m_mutex);

        if (m_maxQueueSize > 0 && m_priorityQueue.size() >= m_maxQueueSize) {
            qWarning() << "MessageQueue is full. Dropping message with ID:" << message->getMessageId();
            emitErrorSignal = true;
            errorMessage = message;
        } else {
            auto insertPosition = std::upper_bound(
                m_priorityQueue.begin(),
                m_priorityQueue.end(),
                message->getPriority(),
                [](IMessage::Priority priority,
                   const QSharedPointer<IMessage> &listMessageItem)
                {
                    return priority > listMessageItem->getPriority();
                }
            );

            if (insertPosition == m_priorityQueue.end())
            {
                m_priorityQueue.append(message);
            }
            else
            {
                m_priorityQueue.insert(insertPosition, message);
            }

            if(!m_processingScheduled)
            {
                m_processingScheduled = true;
                dispatchSignal = true;
            }
        }
    }

    if (emitErrorSignal) {
        emit messageError(errorMessage, "Queue full");
        return;
    }

    if (dispatchSignal) {
        emit messageAvailable();
    }
}

bool MessageQueue::dequeueMessage(QSharedPointer<IMessage> &message, int maxWaitMs)
{
    bool status = false;
    {
        QMutexLocker locker(&m_mutex);
        if(!m_priorityQueue.isEmpty())
        {
            message = m_priorityQueue.takeFirst();
            status = true;
        }
    }
    return status;
}

bool MessageQueue::sendRequestAndWait(QSharedPointer<IMessage> request, QSharedPointer<IMessage> &response, int timeoutMs)
{
    // Step 1: Validate request
    if (!request || !request->isRequest()) {
        qWarning() << "Invalid request message";
        return false;
    }

    QString messageId = request->getMessageId();

    // Step 2: Register this request as pending
    {
        QMutexLocker locker(&m_mutex);

        if (m_waitConditions.contains(messageId)) {
            qWarning() << "Duplicate request message ID:" << messageId;
            return false;
        }

        QWaitCondition* waitCondition = new QWaitCondition();
        m_waitConditions[messageId] = waitCondition;
        m_pendingResponses[messageId] = nullptr;  // Placeholder
    }

    // Step 3: Enqueue the request (dispatcher will route it)
    enqueueMessage(request);

    // Step 4: BLOCK and wait for response
    bool responseReceived = false;
    {
        QMutexLocker locker(&m_mutex);

        while (m_waitConditions.contains(messageId) &&
               !m_pendingResponses[messageId]) {
            if (!m_waitConditions[messageId]->wait(&m_mutex, timeoutMs)) {
                break;
            }
        }

        if (m_pendingResponses.contains(messageId) && m_pendingResponses[messageId]) {
            response = m_pendingResponses[messageId];
            responseReceived = true;
        }
    }

    // Step 5: Cleanup
    {
        QMutexLocker locker(&m_mutex);

        m_pendingResponses.remove(messageId);
        if (m_waitConditions.contains(messageId)) {
            delete m_waitConditions[messageId];
            m_waitConditions.remove(messageId);
        }
    }

    if (responseReceived) {
        qDebug() << "Response received for message ID:" << messageId;
        return true;
    }

    qWarning() << "No response received for message ID:" << messageId
              << "Timeout:" << timeoutMs << "ms";
    return false;
}

bool MessageQueue::completePendingResponse(QSharedPointer<IMessage> response)
{
    if (!response || response->isRequest()) {
        return false;
    }

    QString messageId = response->getMessageId();
    QMutexLocker locker(&m_mutex);

    if (!m_waitConditions.contains(messageId)) {
        return false;
    }

    m_pendingResponses[messageId] = response;
    m_waitConditions[messageId]->wakeAll();
    return true;
}

void MessageQueue::subscribe(MessageType messageType, QObject *receiver, const char *slotName)
{
    // To Do
}

int MessageQueue::getQueueSize() const
{
    QMutexLocker locker(&m_mutex);  // ✅ Thread-safe read
    return m_priorityQueue.size();
}

int MessageQueue::getProcessedMessageCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_processedCount;
}

int MessageQueue::getFailedMessageCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_failedCount;
}

void MessageQueue::setMaxQueueSize(int size)
{  
    QMutexLocker locker(&m_mutex);
    m_maxQueueSize = qMax(0, size);
}

void MessageQueue::processQueuedMessages()
{
    QMutexLocker locker(&m_mutex);
    Q_UNUSED(locker);
}

void MessageQueue::onMessageProcessed(QSharedPointer<IMessage> message)
{
    Q_UNUSED(message);
    QMutexLocker locker(&m_mutex);
    ++m_processedCount;
}

void MessageQueue::onMessageError(QSharedPointer<IMessage> message, const QString &error)
{
    Q_UNUSED(message);
    Q_UNUSED(error);
    QMutexLocker locker(&m_mutex);
    ++m_failedCount;
}

void MessageQueue::onAllQueueItemProcessed()
{
    m_processingScheduled = false;
}

} // namespace Banking


