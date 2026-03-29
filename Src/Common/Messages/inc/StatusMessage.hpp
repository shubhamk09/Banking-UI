#pragma once

#include "../../Interfaces/IMessage.hpp"

namespace Banking {

/**
 * @brief Connection status notification messages
 */
class StatusMessage : public IMessage {
public:
    /**
     * @brief Connection status enumeration
     */
    enum class ConnectionStatus : quint8 {
        DISCONNECTED = 0,   ///< Socket not connected
        CONNECTING = 1,     ///< Connection in progress
        CONNECTED = 2,      ///< Socket successfully connected
        RECONNECTING = 3    ///< Attempting to reconnect
    };

    /**
     * @brief Construct status notification message
     * @param sourceModule Module sending status update (typically BankingSocket)
     * @param status Connection status
     */
    StatusMessage(const QString &sourceModule, ConnectionStatus status);

    /**
     * @brief Return message type as enum (HYBRID DESIGN)
     * @return MessageType::STATUS (enum for O(1) dispatcher routing)
     */
    MessageType getMessageType() const override { return MessageType::STATUS; }

    QString getSourceModule() const override { return m_sourceModule; }
    QString getTargetModule() const override { return "Broadcast"; } // UI updates for all
    QString getMessageId() const override { return m_messageId; }
    quint64 getTimestamp() const override { return m_timestamp; }

    bool isRequest() const override { return false; } // Fire-and-Forget pattern
    Priority getPriority() const override { return Priority::NORMAL; }
    bool requiresEncryption() const override { return false; }
    bool requiresAudit() const override { return false; } // Status updates not audited
    QByteArray serialize() const override;
    bool deserialize(const QByteArray &data) override;

    ConnectionStatus getStatus() const { return m_status; }

    /**
     * @brief Validate status message
     * @return Always true (status is always valid)
     */
    bool validate() const override { return true; }

    /**
     * @brief Get payload summary for logging
     * @return Summary (e.g., "STATUS: CONNECTED")
     */
    QString getPayloadSummary() const override {
        QString statusStr;
        switch (m_status) {
            case ConnectionStatus::DISCONNECTED: statusStr = "DISCONNECTED"; break;
            case ConnectionStatus::CONNECTING: statusStr = "CONNECTING"; break;
            case ConnectionStatus::CONNECTED: statusStr = "CONNECTED"; break;
            case ConnectionStatus::RECONNECTING: statusStr = "RECONNECTING"; break;
        }
        return QString("STATUS: %1").arg(statusStr);
    }

private:
    QString m_sourceModule;
    QString m_messageId;
    quint64 m_timestamp;
    ConnectionStatus m_status;
};

} // namespace Banking
