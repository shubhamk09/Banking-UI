#pragma once

#include "../../Interfaces/IMessage.hpp"

namespace Banking {

/**
 * @brief System error notification messages
 */
class ErrorMessage : public IMessage {
public:
    /**
     * @brief Construct error notification message
     * @param sourceModule Module that detected the error
     * @param errorDesc Error description (must be non-empty)
     * @param errorCode Numeric error code (optional, default -1)
     */
    ErrorMessage(const QString &sourceModule,
                const QString &errorDesc,
                int errorCode = -1);

    /**
     * @brief Return message type as enum (HYBRID DESIGN)
     * @return MessageType::ERROR (enum for O(1) dispatcher routing)
     */
    MessageType getMessageType() const override { return MessageType::ERROR; }

    QString getSourceModule() const override { return m_sourceModule; }
    QString getTargetModule() const override { return "Broadcast"; } // Send to all modules
    QString getMessageId() const override { return m_messageId; }
    quint64 getTimestamp() const override { return m_timestamp; }

    bool isRequest() const override { return false; } // Fire-and-Forget pattern
    Priority getPriority() const override { return Priority::CRITICAL; } // Errors are critical
    bool requiresEncryption() const override { return true; }
    bool requiresAudit() const override { return true; } // Always audit errors
    QByteArray serialize() const override;
    bool deserialize(const QByteArray &data) override;

    QString getErrorDescription() const { return m_errorDesc; }
    int getErrorCode() const { return m_errorCode; }

    /**
     * @brief Validate error message (must have description)
     * @return true if error description is non-empty and code is valid
     */
    bool validate() const override { return !m_errorDesc.isEmpty() && m_errorCode > 0; }

    /**
     * @brief Get payload summary for audit logging
     * @return Summary (e.g., "ERROR [123]: Network connection failed")
     */
    QString getPayloadSummary() const override {
        return QString("ERROR [%1]: %2").arg(m_errorCode).arg(m_errorDesc);
    }

private:
    QString m_sourceModule;
    QString m_messageId;
    quint64 m_timestamp;
    QString m_errorDesc;
    int m_errorCode;
};

} // namespace Banking
