/**
 * @file StatusMessage.cpp
 * @brief Implementation of connection status notification messages
 */

#include "../inc/StatusMessage.hpp"
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>
#include <QDateTime>

namespace Banking {

/**
 * @brief Construct status message
 */
StatusMessage::StatusMessage(const QString &sourceModule, ConnectionStatus status)
    : m_sourceModule(sourceModule),
      m_status(status) {

    // Generate unique message ID
    m_messageId = QUuid::createUuid().toString();

    // Set current timestamp in milliseconds
    m_timestamp = QDateTime::currentMSecsSinceEpoch();
}

/**
 * @brief Serialize status message to JSON
 *
 * @return QByteArray containing JSON serialization
 */
QByteArray StatusMessage::serialize() const {
    QJsonObject json;

    // Message metadata
    json["type"] = "STATUS";
    json["sourceModule"] = m_sourceModule;
    json["targetModule"] = getTargetModule();  // "Broadcast"
    json["messageId"] = m_messageId;
    json["timestamp"] = static_cast<qint64>(m_timestamp);
    json["isRequest"] = false;

    // Convert status enum to string
    QString statusStr;
    switch (m_status) {
        case ConnectionStatus::DISCONNECTED:
            statusStr = "DISCONNECTED";
            break;
        case ConnectionStatus::CONNECTING:
            statusStr = "CONNECTING";
            break;
        case ConnectionStatus::CONNECTED:
            statusStr = "CONNECTED";
            break;
        case ConnectionStatus::RECONNECTING:
            statusStr = "RECONNECTING";
            break;
    }

    // Message-specific data
    json["status"] = statusStr;

    // Convert to JSON document and serialize
    QJsonDocument doc(json);
    return doc.toJson(QJsonDocument::Compact);
}

/**
 * @brief Deserialize status message from JSON
 *
 * @param data QByteArray containing JSON data
 * @return true if deserialization succeeded, false otherwise
 */
bool StatusMessage::deserialize(const QByteArray &data) {
    // Parse JSON
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        return false;
    }

    QJsonObject json = doc.object();

    // Validate message type
    if (json["type"].toString() != "STATUS") {
        return false;
    }

    // Extract metadata
    m_sourceModule = json["sourceModule"].toString();
    m_messageId = json["messageId"].toString();
    m_timestamp = static_cast<quint64>(json["timestamp"].toDouble());

    // Extract and convert status string to enum
    QString statusStr = json["status"].toString();
    if (statusStr == "DISCONNECTED") {
        m_status = ConnectionStatus::DISCONNECTED;
    } else if (statusStr == "CONNECTING") {
        m_status = ConnectionStatus::CONNECTING;
    } else if (statusStr == "CONNECTED") {
        m_status = ConnectionStatus::CONNECTED;
    } else if (statusStr == "RECONNECTING") {
        m_status = ConnectionStatus::RECONNECTING;
    } else {
        return false;  // Invalid status
    }

    // Validate after deserialization
    return validate();
}

} // namespace Banking
