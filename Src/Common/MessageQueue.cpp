#include <QMutexLocker>
#include <QMetaType>

#include "inc/MessageQueue.hpp"

namespace Banking
{

MessageQueue::MessageQueue()
{
    // Register meta type for Qt signals/slots with smart pointers
    qRegisterMetaType<QSharedPointer<IMessage>>("QSharedPointer<Banking::IMessage>");
}

MessageQueue &MessageQueue::instance()
{
    static MessageQueue instance;
    return instance;
}

void MessageQueue::enqueueMessage(QSharedPointer<IMessage> message)
{
    IMessage::Priority messagePriority = message->getPriority();

    {
        QMutexLocker locker(&m_mutex);
        QVector<QSharedPointer<IMessage>>::iterator insertPosition;
        for ( insertPosition = m_priorityQueue.begin(); insertPosition != m_priorityQueue.end(); ++insertPosition) {
            IMessage::Priority currentItrPriority = (*insertPosition)->getPriority();
            if (currentItrPriority < messagePriority)
            {
                break;
            }
            
        } 
        if (insertPosition == m_priorityQueue.end())
        {
            m_priorityQueue.append(message);
        }
        else
        {
            m_priorityQueue.insert(insertPosition, message);
        }
        emit messageEnqueued(message);
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
    // To Do
    return false;
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
    //Need to see if we need to use mutex here
    m_maxQueueSize = size;
}

} // namespace Banking


