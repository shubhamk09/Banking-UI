/**
 * @file AuthMessage.cpp
 * @brief Implementation of authentication request/response messages
 */

#include "../inc/AuthMessage.hpp"
#include <QJsonDocument>
#include <QJsonObject>
#include <QCryptographicHash>
#include <QUuid>
#include <QDateTime>
#include <QRegularExpression>

namespace Banking {

/**
 * @brief Construct authentication message
 */
AuthMessage::AuthMessage(const QString &sourceModule,
                         const QString &username,
                         const QString &password,
                         bool isRequest)
    : m_sourceModule(sourceModule),
      m_username(username),
      m_password(password),
      m_isRequest(isRequest) {

    // Generate unique message ID
    m_messageId = QUuid::createUuid().toString();

    // Set current timestamp in milliseconds
    m_timestamp = QDateTime::currentMSecsSinceEpoch();
}

/**
 * @brief Validate authentication message
 *
 * Requirements:
 * - Username must not be empty
 * - Username must contain only alphanumeric and underscore
 * - Password must not be empty (for requests only)
 * - Password minimum length 6 characters (for requests only)
 *
 * @return true if valid, false otherwise
 */
bool AuthMessage::validate() const {
    // Username validation
    if (m_username.isEmpty()) {
        return false;
    }

    // Check username format: alphanumeric + underscore only
    QRegularExpression usernameRegex("^[a-zA-Z0-9_]+$");
    if (!usernameRegex.match(m_username).hasMatch()) {
        return false;
    }

    // For requests: validate password
    if (m_isRequest) {
        if (m_password.isEmpty()) {
            return false;
        }
        if (m_password.length() < 6) {
            return false;
        }
    }

    return true;
}

/**
 * @brief Serialize authentication message to JSON
 *
 * Security: Passwords are hashed using SHA256, never stored plaintext
 *
 * @return QByteArray containing JSON serialization
 */
QByteArray AuthMessage::serialize() const {
    QJsonObject json;

    // Message metadata (required for all message types)
    json["type"] = "AUTH";
    json["sourceModule"] = m_sourceModule;
    json["targetModule"] = m_targetModule;
    json["messageId"] = m_messageId;
    json["timestamp"] = static_cast<qint64>(m_timestamp);
    json["isRequest"] = m_isRequest;

    // Message-specific data
    json["username"] = m_username;

    // Password handling: never send plaintext
    if (m_isRequest) {
        // Hash password before transmission
        QByteArray passwordHash = QCryptographicHash::hash(
            m_password.toUtf8(),
            QCryptographicHash::Sha256
        );
        json["passwordHash"] = QString(passwordHash.toHex());
    } else {
        // Response: include authentication result
        json["authenticated"] = m_isAuthenticated;
    }

    // Convert to JSON document and serialize
    QJsonDocument doc(json);
    return doc.toJson(QJsonDocument::Compact);
}

/**
 * @brief Deserialize authentication message from JSON
 *
 * @param data QByteArray containing JSON data
 * @return true if deserialization succeeded, false otherwise
 */
bool AuthMessage::deserialize(const QByteArray &data) {
    // Parse JSON
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        return false;
    }

    QJsonObject json = doc.object();

    // Validate message type
    if (json["type"].toString() != "AUTH") {
        return false;
    }

    // Extract metadata
    m_sourceModule = json["sourceModule"].toString();
    m_targetModule = json["targetModule"].toString();
    m_messageId = json["messageId"].toString();
    m_timestamp = static_cast<quint64>(json["timestamp"].toDouble());
    m_isRequest = json["isRequest"].toBool();

    // Extract message-specific data
    m_username = json["username"].toString();

    // Handle password/authentication based on message type
    if (m_isRequest) {
        // For requests, password comes as hash
        QString passwordHashStr = json["passwordHash"].toString();
        m_password = passwordHashStr;  // Store hash, not plaintext
    } else {
        // For responses, extract authentication result
        m_isAuthenticated = json["authenticated"].toBool();
        m_password = "";  // Clear password in response
    }

    // Validate after deserialization
    return validate();
}

} // namespace Banking
