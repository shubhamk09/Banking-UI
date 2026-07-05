#include <gtest/gtest.h>
#include <QSharedPointer>
#include <thread>
#include <chrono>

#include "MessageQueue.hpp"
#include "IMessage.hpp"

using namespace Banking;

class TestMessage : public IMessage {
public:
    TestMessage(MessageType type, Priority priority, const QString &messageId = QString(), bool isRequest = false)
        : m_type(type)
        , m_priority(priority)
        , m_messageId(messageId.isEmpty() ? QString::number(m_nextId++) : messageId)
        , m_isRequest(isRequest)
    {
    }

    MessageType getMessageType() const override { return m_type; }
    QString getSourceModule() const override { return "Test"; }
    QString getTargetModule() const override { return "Test"; }
    Priority getPriority() const override { return m_priority; }
    bool isRequest() const override { return m_isRequest; }
    QString getMessageId() const override { return m_messageId; }
    bool validate() const override { return true; }
    bool requiresEncryption() const override { return false; }
    bool requiresAudit() const override { return false; }
    QByteArray serialize() const override { return QByteArray(); }
    bool deserialize(const QByteArray &data) override { Q_UNUSED(data); return true; }
    quint64 getTimestamp() const override { return 0; }
    QString getPayloadSummary() const override { return "Test message"; }

private:
    MessageType m_type;
    Priority m_priority;
    QString m_messageId;
    bool m_isRequest;
    static quint64 m_nextId;
};

quint64 TestMessage::m_nextId = 1;

class MessageQueueTests : public ::testing::Test {
protected:
    void SetUp() override
    {
        QSharedPointer<IMessage> msg;
        while (MessageQueue::instance().dequeueMessage(msg, 0)) { }
    }
};

TEST_F(MessageQueueTests, EnqueueDequeueOrder)
{
    auto &queue = MessageQueue::instance();

    auto lowMsg = QSharedPointer<IMessage>(new TestMessage(MessageType::STATUS, IMessage::Priority::LOW));
    auto highMsg = QSharedPointer<IMessage>(new TestMessage(MessageType::ERROR, IMessage::Priority::HIGH));
    auto normalMsg = QSharedPointer<IMessage>(new TestMessage(MessageType::AUTH, IMessage::Priority::NORMAL));

    queue.enqueueMessage(lowMsg);
    queue.enqueueMessage(highMsg);
    queue.enqueueMessage(normalMsg);

    EXPECT_EQ(queue.getQueueSize(), 3);

    QSharedPointer<IMessage> msg;
    EXPECT_TRUE(queue.dequeueMessage(msg, 0));
    EXPECT_EQ(msg->getPriority(), IMessage::Priority::HIGH);

    EXPECT_TRUE(queue.dequeueMessage(msg, 0));
    EXPECT_EQ(msg->getPriority(), IMessage::Priority::NORMAL);

    EXPECT_TRUE(queue.dequeueMessage(msg, 0));
    EXPECT_EQ(msg->getPriority(), IMessage::Priority::LOW);
}

TEST_F(MessageQueueTests, DequeueEmptyQueue)
{
    auto &queue = MessageQueue::instance();
    QSharedPointer<IMessage> msg;
    while (queue.dequeueMessage(msg, 0)) { }
    EXPECT_FALSE(queue.dequeueMessage(msg, 0));
}

TEST_F(MessageQueueTests, GetQueueSize)
{
    auto &queue = MessageQueue::instance();
    QSharedPointer<IMessage> msg;
    while (queue.dequeueMessage(msg, 0)) { }

    EXPECT_EQ(queue.getQueueSize(), 0);

    queue.enqueueMessage(QSharedPointer<IMessage>(new TestMessage(MessageType::STATUS, IMessage::Priority::NORMAL)));
    EXPECT_EQ(queue.getQueueSize(), 1);

    queue.dequeueMessage(msg, 0);
    EXPECT_EQ(queue.getQueueSize(), 0);
}

TEST_F(MessageQueueTests, EqualPriorityMaintainsOrder)
{
    auto &queue = MessageQueue::instance();
    QSharedPointer<IMessage> msg;
    while (queue.dequeueMessage(msg, 0)) { }

    auto first = QSharedPointer<IMessage>(new TestMessage(MessageType::STATUS, IMessage::Priority::NORMAL, QString("MSG1")));
    auto second = QSharedPointer<IMessage>(new TestMessage(MessageType::STATUS, IMessage::Priority::NORMAL, QString("MSG2")));

    queue.enqueueMessage(first);
    queue.enqueueMessage(second);

    EXPECT_TRUE(queue.dequeueMessage(msg, 0));
    EXPECT_EQ(msg->getMessageId().toStdString(), "MSG1");

    EXPECT_TRUE(queue.dequeueMessage(msg, 0));
    EXPECT_EQ(msg->getMessageId().toStdString(), "MSG2");
}

TEST_F(MessageQueueTests, SendRequestAndWaitReturnsResponse)
{
    auto &queue = MessageQueue::instance();
    QSharedPointer<IMessage> response;
    QSharedPointer<IMessage> request(new TestMessage(MessageType::AUTH, IMessage::Priority::NORMAL, QString("REQ1"), true));

    std::thread responder([&queue, request]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        auto reply = QSharedPointer<IMessage>(new TestMessage(MessageType::AUTH, IMessage::Priority::NORMAL, request->getMessageId(), false));
        queue.completePendingResponse(reply);
    });

    EXPECT_TRUE(queue.sendRequestAndWait(request, response, 1000));
    EXPECT_TRUE(response);
    EXPECT_EQ(response->getMessageId().toStdString(), "REQ1");

    responder.join();
}
