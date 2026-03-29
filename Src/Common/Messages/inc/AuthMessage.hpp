#pragma once

#include "../../Interfaces/IMessage.hpp"

namespace Banking{

class AuthMessage : public IMessage {
public:
    /**
     * @brief Construct authentication request/response message
     * @param sourceModule Module name sending this message
     * @param username Username for authentication
     * @param password Password for authentication (sensitive)
     * @param isRequest true for request, false for response
     */
    AuthMessage(const QString &sourceModule,
                const QString &username,
                const QString &password,
                bool isRequest = true);

    /**
     * @brief Return message type as enum (HYBRID DESIGN)
     * @return MessageType::AUTH (enum for O(1) dispatcher routing)
     */
    MessageType getMessageType() const override { return MessageType::AUTH; }

    QString getSourceModule() const override { return m_sourceModule; }
    QString getTargetModule() const override { return m_targetModule; }
    QString getMessageId() const override { return m_messageId; }
    quint64 getTimestamp() const override { return m_timestamp; }

    bool isRequest() const override { return m_isRequest; }
    Priority getPriority() const override { return Priority::HIGH; }
    bool requiresEncryption() const override { return true; }
    // bool requiresAudit() const override { return false; } // Don't audit passwords
    QByteArray serialize() const override;
    bool deserialize(const QByteArray &data) override;

    QString getUsername() const { return m_username; }
    QString getPassword() const { return m_password; } // Sensitive - NOT logged in audit trail
    bool isAuthenticated() const { return m_isAuthenticated; }
    void setAuthenticated(bool auth) { m_isAuthenticated = auth; }

    /**
     * @brief Validate auth message
     * @return true if username and password are non-empty
     */
    bool validate() const override;

    /**
     * @brief Get payload summary for audit logging
     * @return Summary WITHOUT password (e.g., "AUTH request from user: john")
     */
    QString getPayloadSummary() const override {
        return QString("AUTH %1 for user: %2")
            .arg(m_isRequest ? "request" : "response", m_username);
    }

private:
    QString m_sourceModule;
    QString m_targetModule = "BankingSocket";  // Default target
    QString m_messageId;
    quint64 m_timestamp;
    QString m_username;
    QString m_password;
    bool    m_isAuthenticated = false;
    bool    m_isRequest;
};

} // namespace Banking