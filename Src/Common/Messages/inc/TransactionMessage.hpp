#pragma once

#include "../../Interfaces/IMessage.hpp"

namespace Banking {

/**
 * @brief Banking transaction request/response messages
 */
class TransactionMessage : public IMessage {
public:
    /**
     * @brief Construct transaction request/response message
     * @param sourceModule Module sending this message
     * @param toAccount Recipient account number
     * @param amount Transaction amount
     * @param isRequest true for request, false for response
     */
    TransactionMessage(const QString &sourceModule,
                      const QString &toAccount,
                      double amount,
                      bool isRequest = true);

    /**
     * @brief Return message type as enum (HYBRID DESIGN)
     * @return MessageType::TRANSACTION (enum for O(1) dispatcher routing)
     */
    MessageType getMessageType() const override { return MessageType::TRANSACTION; }

    QString getSourceModule() const override { return m_sourceModule; }
    QString getTargetModule() const override { return m_targetModule; }
    QString getMessageId() const override { return m_messageId; }
    quint64 getTimestamp() const override { return m_timestamp; }

    bool isRequest() const override { return m_isRequest; }
    Priority getPriority() const override { return Priority::NORMAL; }
    bool requiresEncryption() const override { return true; }
    bool requiresAudit() const override { return true; }  // Audit all transactions
    QByteArray serialize() const override;
    bool deserialize(const QByteArray &data) override;

    QString getToAccount() const { return m_toAccount; }
    double getAmount() const { return m_amount; }
    QString getTransactionId() const { return m_txnId; }
    bool isSuccessful() const { return m_isSuccessful; }
    void setSuccessful(bool success) { m_isSuccessful = success; }

    /**
     * @brief Validate transaction message
     * @return true if amount > 0 and account is non-empty
     */
    bool validate() const override;

    /**
     * @brief Get payload summary for audit logging
     * @return Summary (e.g., "Txn: TXN001 Amount: 1000.00")
     */
    QString getPayloadSummary() const override {
        return QString("Txn: %1 Amount: %2 To: %3")
            .arg(m_txnId, QString::number(m_amount, 'f', 2), m_toAccount);
    }

private:
    QString m_sourceModule;
    QString m_targetModule = "BankingSocket";
    QString m_messageId;
    quint64 m_timestamp;
    QString m_toAccount;
    double m_amount;
    QString m_txnId;
    bool m_isSuccessful = false;
    bool m_isRequest;
};

} // namespace Banking
