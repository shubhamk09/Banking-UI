/**
 * @class IMessage
 * @brief Abstract interface for all inter-module messages
 *
 * All message types must inherit from IMessage and implement
 * validation, serialization, and routing logic.
 *
 * Key Design Principles:
 * - Use virtual methods for all operations (no unsafe downcasting)
 * - MessageType as enum for fast routing
 * - All other fields as QString for Qt integration
 */

 #pragma once

 // Includes
#include <QString>
#include <QByteArray>
#include <QSharedPointer>
#include <cstdint>


namespace Banking {

/**
 * @brief Message type enumeration for type-safe routing
 *
 * Use enums instead of strings for message types to enable:
 * - O(1) comparison vs O(n) string comparison
 * - Compile-time type safety
 * - Efficient dispatcher routing
 */
enum class MessageType : quint8 {
    AUTH = 0,              ///< Authentication/login messages
    TRANSACTION = 1,       ///< Banking transaction messages
    ERROR = 2,             ///< Error notifications
    STATUS = 3             ///< Connection status updates
};


class IMessage {
public:
    enum class Priority : uint8_t {
        LOW = 0,       ///< Normal operations (status, info)
        NORMAL = 50,   ///< Regular requests/responses
        HIGH = 75,     ///< Important operations (auth)
        CRITICAL = 100 ///< Errors, system failures
    };

    enum class Status : uint8_t {
        PENDING,       ///< Waiting to be processed
        PROCESSING,    ///< Currently being handled
        COMPLETED,     ///< Successfully completed
        FAILED,        ///< Failed execution
        TIMEOUT        ///< Request timeout
    };

    virtual ~IMessage() = default;

    // CRITICAL: getMessageType() returns ENUM, not QString - for O(1) dispatcher routing
    // This is the ONLY enum used for routing; all module names remain as QString
    /**
     * @brief Get message type as enum (HYBRID DESIGN)
     *
     * RETURNS ENUM for high-performance dispatcher routing (O(1) enum comparison).
     * This is the ONLY message property that uses enum; all module names remain QString.
     *
     * @return MessageType enum value (efficient O(1) comparison for dispatcher)
     */
    virtual MessageType getMessageType() const = 0;

    // Core properties - Module routing (QString for flexibility)
    /**
     * @brief Get source module name
     * @return QString module name (e.g., "LoginModule", "TransactionModule")
     */
    virtual QString getSourceModule() const = 0;

    /**
     * @brief Get target module name
     * @return QString module name or "Broadcast" for all
     */
    virtual QString getTargetModule() const = 0;

    // Priority & routing
    /**
     * @brief Get message priority level
     * @return Priority enum (determines queue order)
     */
    virtual Priority getPriority() const = 0;

    /**
     * @brief Check if message expects a response
     * @return true for request messages, false for fire-and-forget
     */
    virtual bool isRequest() const = 0;

    /**
     * @brief Get unique message ID
     * @return QString ID for request-reply matching
     */
    virtual QString getMessageId() const = 0;

    // Validation & security
    /**
     * @brief Validate message payload
     * @return true if valid, false otherwise
     */
    virtual bool validate() const = 0;

    /**
     * @brief Check if message requires encryption
     * @return true if encryption needed (e.g., passwords, tokens)
     */
    virtual bool requiresEncryption() const = 0;

    /**
     * @brief Check if message should be audit logged
     * @return true to include in audit trail
     */
    virtual bool requiresAudit() const = 0;

    // Serialization
    /**
     * @brief Serialize message to bytes
     * @return QByteArray containing serialized message
     */
    virtual QByteArray serialize() const = 0;

    /**
     * @brief Deserialize message from bytes
     * @param data Serialized message data
     * @return true if deserialization succeeded
     */
    virtual bool deserialize(const QByteArray &data) = 0;

    // Metadata
    /**
     * @brief Get message creation timestamp
     * @return Milliseconds since epoch
     */
    virtual quint64 getTimestamp() const = 0;

    /**
     * @brief Get request-reply timeout
     * @return Timeout in milliseconds (default 5000ms)
     */
    virtual int getTimeoutMs() const { return 5000; }

    /**
     * @brief Get human-readable payload summary
     *
     * Used for audit logging. Should NOT include sensitive data
     * (e.g., passwords, tokens). Example: "Txn: TXN001 Amount: 1000.00"
     *
     * @return QString summary for audit trail
     */
    virtual QString getPayloadSummary() const = 0;

    /**
     * @brief Helper to convert MessageType enum to string
     *
     * Useful for logging, debugging, UI display.
     * Example: messageTypeToString(MessageType::AUTH) → "AUTH"
     *
     * @param type MessageType enum value
     * @return QString representation of type
     */
    static QString messageTypeToString(MessageType type) {
        switch (type) {
            case MessageType::AUTH: return "AUTH";
            case MessageType::TRANSACTION: return "TRANSACTION";
            case MessageType::ERROR: return "ERROR";
            case MessageType::STATUS: return "STATUS";
            default: return "UNKNOWN";
        }
    }

    /**
     * @brief Helper to convert string to MessageType enum
     *
     * Used for deserialization or config parsing.
     *
     * @param typeStr String representation ("AUTH", "TRANSACTION", etc.)
     * @return MessageType enum if found, or MessageType::ERROR as fallback
     */
    static MessageType stringToMessageType(const QString &typeStr) {
        if (typeStr == "AUTH") return MessageType::AUTH;
        if (typeStr == "TRANSACTION") return MessageType::TRANSACTION;
        if (typeStr == "ERROR") return MessageType::ERROR;
        if (typeStr == "STATUS") return MessageType::STATUS;
        return MessageType::ERROR; // Fallback to ERROR for unknown types
    }
};

} // namespace Banking