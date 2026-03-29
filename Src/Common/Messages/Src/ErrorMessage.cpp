/**
 * @file ErrorMessage.cpp
 * @brief Implementation of error notification messages
 */

#include "../inc/ErrorMessage.hpp"
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>
#include <QDateTime>

namespace Banking {

/**
 * @brief Construct error message
 */
ErrorMessage::ErrorMessage(const QString &sourceModule,
                          const QString &errorDesc,
                          int errorCode)
    : m_sourceModule(sourceModule),
      m_errorDesc(errorDesc),
      m_errorCode(errorCode) {

    // Generate unique message ID
    m_messageId = QUuid::createUuid().toString();

    // Set current timestamp in milliseconds
    m_timestamp = QDateTime::currentMSecsSinceEpoch();

    // If no error code provided, use default
    if (m_errorCode <= 0) {
        m_errorCode = 9999;  // Unknown error code
    }
}

/**
 * @brief Serialize error message to JSON
 *
 * @return QByteArray containing JSON serialization
 */
QByteArray ErrorMessage::serialize() const {
    QJsonObject json;

    // Message metadata
    json["type"] = "ERROR";
    json["sourceModule"] = m_sourceModule;
    json["targetModule"] = getTargetModule();  // "Broadcast"
    json["messageId"] = m_messageId;
    json["timestamp"] = static_cast<qint64>(m_timestamp);
    json["isRequest"] = false;

    // Message-specific data
    json["errorDescription"] = m_errorDesc;
    json["errorCode"] = m_errorCode;

    // Convert to JSON document and serialize
    QJsonDocument doc(json);
    return doc.toJson(QJsonDocument::Compact);
}

/**
 * @brief Deserialize error message from JSON
 *
 * @param data QByteArray containing JSON data
 * @return true if deserialization succeeded, false otherwise
 */
bool ErrorMessage::deserialize(const QByteArray &data) {
    // Parse JSON
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        return false;
    }

    QJsonObject json = doc.object();

    // Validate message type
    if (json["type"].toString() != "ERROR") {
        return false;
    }

    // Extract metadata
    m_sourceModule = json["sourceModule"].toString();
    m_messageId = json["messageId"].toString();
    m_timestamp = static_cast<quint64>(json["timestamp"].toDouble());

    // Extract message-specific data
    m_errorDesc = json["errorDescription"].toString();
    m_errorCode = json["errorCode"].toInt();

    // Validate after deserialization
    return validate();
}

} // namespace Banking
