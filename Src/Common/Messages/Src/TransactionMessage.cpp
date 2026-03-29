/**
 * @file TransactionMessage.cpp
 * @brief Implementation of banking transaction messages
 */

#include "../inc/TransactionMessage.hpp"
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>
#include <QDateTime>

namespace Banking {

/**
 * @brief Construct transaction message
 */
TransactionMessage::TransactionMessage(const QString &sourceModule,
                                       const QString &toAccount,
                                       double amount,
                                       bool isRequest)
    : m_sourceModule(sourceModule),
      m_toAccount(toAccount),
      m_amount(amount),
      m_isRequest(isRequest) {

    // Generate unique message ID
    m_messageId = QUuid::createUuid().toString();

    // Generate transaction ID
    m_txnId = QString("TXN%1").arg(QDateTime::currentMSecsSinceEpoch());

    // Set current timestamp in milliseconds
    m_timestamp = QDateTime::currentMSecsSinceEpoch();
}

/**
 * @brief Validate transaction message
 *
 * Requirements:
 * - Amount must be > 0
 * - Amount must be <= 1,000,000 (max transfer limit)
 * - Recipient account must not be empty
 * - Account number must be 10 numeric digits
 *
 * @return true if valid, false otherwise
 */
bool TransactionMessage::validate() const {
    // Amount validation
    if (m_amount <= 0) {
        return false;
    }
    if (m_amount > 1000000) {  // Max transfer limit
        return false;
    }

    // Account validation
    if (m_toAccount.isEmpty()) {
        return false;
    }

    // Account must be 10 digit number
    if (m_toAccount.length() != 10) {
        return false;
    }

    // Account must be numeric
    bool isNumeric = false;
    m_toAccount.toULongLong(&isNumeric, 10);
    if (!isNumeric) {
        return false;
    }

    return true;
}

/**
 * @brief Serialize transaction message to JSON
 *
 * @return QByteArray containing JSON serialization
 */
QByteArray TransactionMessage::serialize() const {
    QJsonObject json;

    // Message metadata
    json["type"] = "TRANSACTION";
    json["sourceModule"] = m_sourceModule;
    json["targetModule"] = m_targetModule;
    json["messageId"] = m_messageId;
    json["timestamp"] = static_cast<qint64>(m_timestamp);
    json["isRequest"] = m_isRequest;

    // Message-specific data
    json["toAccount"] = m_toAccount;
    json["amount"] = m_amount;
    json["transactionId"] = m_txnId;
    json["isSuccessful"] = m_isSuccessful;

    // Convert to JSON document and serialize
    QJsonDocument doc(json);
    return doc.toJson(QJsonDocument::Compact);
}

/**
 * @brief Deserialize transaction message from JSON
 *
 * @param data QByteArray containing JSON data
 * @return true if deserialization succeeded, false otherwise
 */
bool TransactionMessage::deserialize(const QByteArray &data) {
    // Parse JSON
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        return false;
    }

    QJsonObject json = doc.object();

    // Validate message type
    if (json["type"].toString() != "TRANSACTION") {
        return false;
    }

    // Extract metadata
    m_sourceModule = json["sourceModule"].toString();
    m_targetModule = json["targetModule"].toString();
    m_messageId = json["messageId"].toString();
    m_timestamp = static_cast<quint64>(json["timestamp"].toDouble());
    m_isRequest = json["isRequest"].toBool();

    // Extract message-specific data
    m_toAccount = json["toAccount"].toString();
    m_amount = json["amount"].toDouble();
    m_txnId = json["transactionId"].toString();
    m_isSuccessful = json["isSuccessful"].toBool();

    // Validate after deserialization
    return validate();
}

} // namespace Banking
