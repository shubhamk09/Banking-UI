# MessageQueue Complete Implementation Plan
## Banking-UI Inter-Module Communication System

**Date**: March 2026
**Status**: Ready for Implementation
**Project**: Banking-UI
**Author**: Claude Code
**Document**: Complete Reference Guide

---

## TABLE OF CONTENTS

1. [Executive Summary](#executive-summary)
2. [Quick Reference](#quick-reference)
3. [Problem Analysis](#problem-analysis)
4. [Requirements](#requirements)
5. [Architectural Design](#architectural-design)
6. [Core Components](#core-components)
7. [Design Patterns](#design-patterns)
8. [Communication Patterns](#communication-patterns)
9. [Sequence Diagrams](#sequence-diagrams)
10. [Data Flow](#data-flow)
11. [Inheritance & Polymorphism](#inheritance--polymorphism)
12. [Implementation Architecture](#implementation-architecture)
13. [Testing Strategy](#testing-strategy)
14. [Implementation Phases](#implementation-phases)
15. [Risk Mitigation](#risk-mitigation)
16. [Success Metrics](#success-metrics)

---

## EXECUTIVE SUMMARY

Implement a robust, thread-safe **MessageQueue system** for inter-module communication in the Banking-UI application. This resolves the current architectural gap where modules operate in isolation.

### What This Solves
- ❌ **Current Problem**: Modules cannot communicate with each other
- ✅ **Solution**: Unified, type-safe message passing infrastructure
- ✅ **Result**: Decoupled, extensible, maintainable architecture

### Key Features
- ✅ **Async Communication** (Fire-and-Forget)
- ✅ **Sync Communication** (Request-Reply with timeout)
- ✅ **Priority Queue** (CRITICAL > HIGH > NORMAL)
- ✅ **Type-Safe Messaging** (Virtual methods, NO unsafe downcasting)
- ✅ **Multi-threaded Safety** (QMutex-protected)
- ✅ **Full Audit Trail** (Banking compliance)
- ✅ **Zero External Dependencies** (Qt-native only)

### Business Value
| Aspect | Benefit |
|--------|---------|
| **Modularity** | Modules decoupled, independently testable |
| **Extensibility** | New message types/modules without core changes |
| **Reliability** | Thread-safe, priority-based, timeout handling |
| **Compliance** | Complete audit trail for banking regulations |
| **Performance** | <10ms latency, 1000+ msg/sec throughput |
| **Maintainability** | Clean virtual method design, no unsafe casting |

---

## QUICK REFERENCE

### File Structure at a Glance

```
Src/Common/
├── Interfaces/
│   ├── IModule.hpp                   # Existing
│   └── IMessage.hpp                  # Abstract message base (MOVED HERE)
│
├── Messages/
│   ├── inc/
│   │   ├── AuthMessage.hpp           # Auth messages
│   │   ├── TransactionMessage.hpp    # Transaction messages
│   │   ├── ErrorMessage.hpp          # Error messages
│   │   └── StatusMessage.hpp         # Status messages
│   │ 
│   ├── AuthMessage.cpp
│   ├── TransactionMessage.cpp
│   ├── ErrorMessage.cpp
│   └── StatusMessage.cpp
│
├── inc/
│   ├── MessageQueue.hpp              # Singleton queue
│   ├── MessageDispatcher.hpp         # Message router
│   └── AuditLogger.hpp               # Compliance logging
│
└── src/
    ├── MessageQueue.cpp
    ├── MessageDispatcher.cpp
    └── AuditLogger.cpp
```

### Message Inheritance Hierarchy

```
IMessage (Abstract Base Class)
├─ AuthMessage           (Priority: HIGH=75)
├─ TransactionMessage    (Priority: NORMAL=50)
├─ ErrorMessage          (Priority: CRITICAL=100)
└─ StatusMessage         (Priority: NORMAL=50)
```

### Core Classes

| Class | Purpose | Pattern |
|-------|---------|---------|
| `IMessage` | Abstract message interface | Template Method |
| `MessageQueue` | Thread-safe priority queue | Singleton |
| `MessageDispatcher` | Routes messages to handlers | Observer |
| `AuditLogger` | Compliance logging | Singleton |

### Key Design Principle: HYBRID TYPE SYSTEM

**CRITICAL DESIGN DECISION**:
- `getMessageType()` returns **MessageType ENUM** (not QString) for **O(1) dispatcher routing**
- All other fields (module names, message IDs, etc.) remain as **QString** for Qt ecosystem compatibility
- This is the ONLY enum used for type identification - everything else is QString

| Property | Type | Reason |
|----------|------|--------|
| `getMessageType()` | `MessageType` enum | Fast dispatcher routing (O(1) vs O(n)) |
| `getSourceModule()` | `QString` | Module name flexibility, Qt integration |
| `getTargetModule()` | `QString` | Dynamic routing, broadcasts to multiple |
| `getMessageId()` | `QString` | Unique ID for request-reply matching |
| All metadata | `QString` | Qt signals/slots, database compatibility |

**Example: Dispatcher Routing**
```cpp
// ✅ FAST enumeration comparison (O(1))
MessageType type = msg->getMessageType();  // Returns enum, not string
switch(type) {
    case MessageType::AUTH: routeToAuthHandlers();
    case MessageType::TRANSACTION: routeToTxnHandlers();
    // ... fast comparison
}

// ❌ SLOW string comparison (O(n)) - DO NOT USE
if (msg->getMessageType() == "AUTH") { ... }  // This method RETURNS ENUM, not string
```

### Key Design: NO Downcasting Required

```cpp
// ✅ PREFERRED - Virtual methods
void processMessage(QSharedPointer<IMessage> msg) {
    msg->validate();              // Correct override called based on type
    msg->getPayloadSummary();     // Type-specific implementation invoked
    // No downcasting needed!
}

// ✅ RARELY NEEDED - Safe casting
if (auto authMsg = qobject_cast<AuthMessage*>(msg.get())) {
    bool authenticated = authMsg->isAuthenticated();  // Type-specific field
}

// ❌ AVOID - Unsafe casting
AuthMessage* auth = static_cast<AuthMessage*>(msg.get());  // DANGEROUS!
```

---

## PROBLEM ANALYSIS

### Current Architecture Gaps

**Current State:**
```
┌─────────────────────────────────────┐
│  Login Module                       │
│  [Isolated - No Communication]      │
└─────────────────────────────────────┘

┌─────────────────────────────────────┐
│  BankingSocket Module               │
│  [Isolated - No Communication]      │
└─────────────────────────────────────┘

├─ ❌ Login cannot send auth requests
├─ ❌ BankingSocket cannot notify of connection status
├─ ❌ Errors are not propagated
└─ ❌ No inter-module coordination
```

**Issues:**
1. No way for Login module to authenticate through BankingSocket
2. Connection status changes cannot be communicated to UI
3. Transaction requests have no routing mechanism
4. Error handling is localized to individual modules
5. No audit trail for compliance

### Target State

```
┌────────────────┐
│ Login Module   │──┐
└────────────────┘  │
                    │
┌────────────────┐  │    ┌──────────────────────┐
│ Transaction    │──┼───►│  MessageQueue        │────┐
│ Module         │  │    │  (Core System)       │    │
└────────────────┘  │    │  ├─ Priority Queue  │    │
                    │    │  ├─ Thread Safety   │    │
┌────────────────┐  │    │  └─ Audit Logging  │    │
│ BankingSocket  │──┤    └──────────────────────┘    │
│ Module         │  │                               │
└────────────────┘  └───────────────────────────────┘
                    [Unified Communication]

✅ All modules can send/receive messages
✅ Priority-based processing
✅ Complete audit trail
✅ Thread-safe multi-module coordination
```

---

## REQUIREMENTS

### 2.1 Functional Requirements

| ID | Requirement | Priority | Description |
|----|-------------|----------|-------------|
| **FR1** | Fire-and-Forget Async | HIGH | Module sends message, continues without waiting |
| **FR2** | Request-Reply Sync | HIGH | Module sends request, blocks until response received |
| **FR3** | Priority Queue | HIGH | Process by priority: CRITICAL(100) > HIGH(75) > NORMAL(50) > LOW(0) |
| **FR4** | Type-Safe Messages | HIGH | IMessage interface with polymorphic subclasses |
| **FR5** | Thread Safety | HIGH | Safe ops across multiple threads, no data races |
| **FR6** | Message Routing | HIGH | Route messages from source to target module |
| **FR7** | Audit Trail | MEDIUM | Log all messages with timestamp, source, target, status |
| **FR8** | Message Validation | MEDIUM | Each message type validates its own payload |
| **FR9** | Error Handling | MEDIUM | Failed messages logged and reported |
| **FR10** | Timeout Handling | MEDIUM | Request-Reply messages timeout after config duration |

### 2.2 Non-Functional Requirements

| Requirement | Target |
|-------------|--------|
| **Thread Safety** | Zero data races, mutex-protected |
| **Throughput** | ≥1000 messages/sec |
| **Latency** | <10ms per queue operation |
| **Memory** | Dynamic queue size, bounded by RAM |
| **Extensibility** | Add new message types without modifying queue code |
| **Code Coverage** | ≥90% for core components |
| **Compliance** | Full audit trail for financial transactions |

### 2.3 Design Constraints

1. **Qt-Only**: No external dependencies (ZeroMQ, Boost, etc.)
2. **Multi-threaded**: Modules may run on different threads
3. **Memory-Safe**: Use smart pointers, no raw memory management
4. **Logging**: Complete audit trail for banking compliance
5. **Integration**: Must integrate with existing `IModule` interface and `StartupManager`

---

## ARCHITECTURAL DESIGN

### System Architecture Diagram

```
┌────────────────────────────────────────────────────────────────┐
│                    Banking-UI Application                      │
├────────────────────────────────────────────────────────────────┤
│                                                                │
│  ┌──────────────────┐     ┌──────────────────────────────┐    │
│  │  Login Module    │     │  MessageQueue (Core)         │    │
│  ├──────────────────┤     ├──────────────────────────────┤    │
│  │ Sends: AUTH_REQ  ├────►│ - Priority Queue            │    │
│  │ Gets: AUTH_RESP  │◄────│ - Thread-safe (Mutex)       │    │
│  └──────────────────┘     │ - Audit Logger              │    │
│                           │ - Message Router            │    │
│  ┌──────────────────┐     │ - Request-Reply Matching    │    │
│  │ Transaction Mod  │────►├──────────────────────────────┤    │
│  ├──────────────────┤     │ IMessage Interface          │    │
│  │ Sends: TXN_REQ   │◄────│ ├─ AuthMessage             │    │
│  │ Gets: TXN_RESP   │     │ ├─ TransactionMessage      │    │
│  └──────────────────┘     │ ├─ ErrorMessage            │    │
│                           │ └─ StatusMessage           │    │
│  ┌──────────────────┐     └──────────────────────────────┘    │
│  │ BankingSocket    │                                         │
│  │ (Communications) │         ┌──────────────────────┐        │
│  ├──────────────────┤         │  Audit Logger        │        │
│  │ Sends: STATUS    ├────────►├──────────────────────┤        │
│  │ Receives: all    │         │ - File Logging       │        │
│  └──────────────────┘         │ - Timestamps         │        │
│                               │ - Compliance         │        │
│  [Future Modules...]          └──────────────────────┘        │
│                                                                │
└────────────────────────────────────────────────────────────────┘
```

### Application Startup Sequence

```
main()
  │
  ├─► QGuiApplication created
  │
  ├─► QQmlApplicationEngine loads Main.qml
  │
  ├─► StartupManager constructor
  │   │
  │   ├─► registerModules()
  │   │   └─► CommunicationInitializer registered
  │   │
  │   ├─► loadConfig()
  │   │   └─► Load modules.json from resource
  │   │
  │   ├─► initCore()  ◄── KEY CHANGE (Initialize MessageQueue)
  │   │   │
  │   │   ├─► MessageQueue::instance() [singleton created]
  │   │   │   └─► m_workerThread started
  │   │   │
  │   │   ├─► MessageDispatcher created
  │   │   │   └─► registerModuleHandlers()
  │   │   │
  │   │   ├─► AuditLogger initialized
  │   │   │   └─► setLogPath("logs/messages_audit.log")
  │   │   │
  │   │   └─► Signals/Slots connected
  │   │
  │   └─► initModules()
  │       │
  │       ├─► CommunicationInitializer->init()
  │       │   └─► BankingSocket created
  │       │
  │       ├─► CommunicationInitializer->start()
  │       │   └─► connectToServer("127.0.0.1", 5020)
  │       │
  │       └─► [Future modules initialized here]
  │
  └─► app.exec()
      └─► Application ready for inter-module communication
```

---

## CORE COMPONENTS

### 1. IMessage Interface

**Location**: `Src/Common/Messages/inc/IMessage.hpp`

**Design Decision**: Hybrid approach for optimal performance and type safety:
- ✅ `getMessageType()` returns **Enum** (O(1) routing, type-safe)
- ✅ Module names, IDs, summaries use **QString** (Qt integration, flexibility)

```cpp
namespace Banking {
namespace Common {

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

/**
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
class IMessage {
public:
    enum class Priority : quint8 {
        LOW = 0,       ///< Normal operations (status, info)
        NORMAL = 50,   ///< Regular requests/responses
        HIGH = 75,     ///< Important operations (auth)
        CRITICAL = 100 ///< Errors, system failures
    };

    enum class Status : quint8 {
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

} // namespace Common
} // namespace Banking
```

**Why This Hybrid Design?**

| Feature | Enum | QString | Our Approach |
|---------|------|---------|--------------|
| **Type Safety** | ✅ Yes | ❌ No | Use Enum for types |
| **Routing Speed** | ✅ O(1) | ❌ O(n) | Use Enum for types |
| **Qt Integration** | ❌ No | ✅ Yes | Use QString for modules/info |
| **Flexibility** | ❌ Limited | ✅ Yes | Use QString for names |
| **Database/Logging** | ⚠️ Numeric | ✅ String | Support both via helpers |

### 2. Concrete Message Types

#### AuthMessage
**Purpose**: Login authentication requests and responses

```cpp
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
    bool requiresAudit() const override { return false; } // Don't audit passwords
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
    bool m_isAuthenticated = false;
    bool m_isRequest;
};
```

**Usage Example**:
- **Request**: Login module sends AUTH_REQ with username/password
- **Response**: BankingSocket sends AUTH_RESP with result
- **Pattern**: Request-Reply (synchronous)
- **Priority**: HIGH (75)

#### TransactionMessage
**Purpose**: Banking transaction requests and confirmations

```cpp
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
```

**Usage Example**:
- **Request**: Transaction module sends TXN_REQ
- **Response**: BankingSocket confirms TXN_RESP
- **Pattern**: Request-Reply (synchronous)
- **Priority**: NORMAL (50)
- **Audit**: Full transaction details logged

#### ErrorMessage
**Purpose**: System error notifications

```cpp
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
     * @return true if error description is non-empty
     */
    bool validate() const override { return !m_errorDesc.isEmpty(); }

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
```

**Usage Example**:
- **Pattern**: Fire-and-Forget (asynchronous)
- **Priority**: CRITICAL (100) - processed immediately
- **Route**: Broadcast to all modules for error handling

#### StatusMessage
**Purpose**: Connection status updates

```cpp
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
```

**Usage Example**:
- **Pattern**: Fire-and-Forget (asynchronous)
- **Priority**: NORMAL (50)
- **Route**: Broadcast for UI updates

### 3. MessageQueue (Singleton)

**Location**: `Src/Common/inc/MessageQueue.hpp`

Thread-safe priority queue managing all inter-module messages.

**KEY DESIGN NOTE**: Routing uses `MessageType` enum via `getMessageType()` for O(1) performance:
```cpp
// Fast dispatcher routing using enum comparison (O(1)):
switch(msg->getMessageType()) {
    case MessageType::AUTH:        dispatcher->routeToAuthHandlers(msg);
    case MessageType::TRANSACTION: dispatcher->routeToTxnHandlers(msg);
    case MessageType::ERROR:       dispatcher->routeToErrorHandlers(msg);
    case MessageType::STATUS:      dispatcher->routeToStatusHandlers(msg);
}
// Instead of slower string comparison: msg->getMessageType() == "AUTH"
```

```cpp
class MessageQueue : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Get singleton instance
     * @return Reference to MessageQueue singleton
     */
    static MessageQueue& instance();

    /**
     * @brief Enqueue message for asynchronous processing
     *
     * Message is inserted into priority queue and processed by worker thread.
     * Returns immediately (Fire-and-Forget pattern).
     *
     * @param message Message to enqueue
     */
    void enqueueMessage(QSharedPointer<IMessage> message);

    /**
     * @brief Dequeue next message from queue
     *
     * Retrieves message with highest priority. Blocks if queue empty.
     *
     * @param message Output parameter for dequeued message
     * @param maxWaitMs Maximum wait time in milliseconds
     * @return true if message available, false if timeout
     */
    bool dequeueMessage(QSharedPointer<IMessage> &message, int maxWaitMs = 100);

    /**
     * @brief Send request and wait for response (Request-Reply pattern)
     *
     * Enqueues request, blocks until response received or timeout.
     * Response matched using message ID.
     *
     * @param request Request message to send
     * @param response Output parameter for response message
     * @param timeoutMs Maximum wait time (default 5000ms)
     * @return true if response received before timeout
     */
    bool sendRequestAndWait(QSharedPointer<IMessage> request,
                           QSharedPointer<IMessage> &response,
                           int timeoutMs = 5000);

    /**
     * @brief Subscribe to messages of specific type
     *
     * Uses MessageType enum for dispatcher routing.
     *
     * @param messageType Message type enum
     * @param receiver Qt object to receive signal
     * @param slotName Slot name to invoke
     */
    void subscribe(MessageType messageType, QObject *receiver,
                  const char *slotName);

    // Status queries
    int getQueueSize() const;
    int getProcessedMessageCount() const;
    int getFailedMessageCount() const;

    // Configuration
    void setMaxQueueSize(int size);
    void setAuditLogging(bool enabled, const QString &logPath);

signals:
    void messageEnqueued(QSharedPointer<IMessage> message);
    void messageProcessed(QSharedPointer<IMessage> message);
    void messageError(QSharedPointer<IMessage> message, const QString &error);

private slots:
    void processQueuedMessages();  ///< Worker thread processes messages

private:
    MessageQueue();

    mutable QMutex m_mutex;

    // Priority queue: higher priority = lower index
    QVector<QSharedPointer<IMessage>> m_priorityQueue;

    // For request-reply pattern: messageId -> response message
    QMap<QString, QSharedPointer<IMessage>> m_pendingResponses;
    QMap<QString, QWaitCondition> m_waitConditions;

    // Subscriptions: MessageType enum -> list of (receiver, slot)
    // Using enum key instead of QString for O(1) dispatcher lookup
    QMap<quint8, QList<QPair<QObject*, const char*>>> m_subscriptions;

    // Audit logging
    QFile m_auditLog;
    bool m_auditEnabled;

    // Statistics
    int m_processedCount;
    int m_failedCount;
    int m_maxQueueSize;

    // Worker thread
    QThread *m_workerThread;
};
```

### 4. MessageDispatcher

**Location**: `Src/Common/inc/MessageDispatcher.hpp`

Routes messages to appropriate module handlers using **MessageType enum for O(1) lookup**.

```cpp
class MessageDispatcher : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param queue Reference to MessageQueue for subscribing
     */
    explicit MessageDispatcher(MessageQueue &queue);

    /**
     * @brief Register handler for specific message type
     *
     * Uses MessageType enum for fast dispatcher routing.
     *
     * @param messageType Message type enum (not string!)
     * @param handler Qt object to receive messages
     * @param slotName Slot method name (e.g., SLOT(onAuthMessage(QSharedPointer<IMessage>)))
     */
    void registerHandler(MessageType messageType,
                        QObject *handler,
                        const char *slotName);

    /**
     * @brief Register all handlers for a module
     *
     * @param module IModule instance with handler slots
     */
    void registerModuleHandler(IModule *module);

    /**
     * @brief Start dispatcher on worker thread
     */
    void start();

    /**
     * @brief Stop dispatcher
     */
    void stop();

private slots:
    /**
     * @brief Process next message from queue
     *
     * Called internally by worker thread dispatcher loop.
     * Uses enum comparison (O(1)) to route based on message type.
     */
    void onMessageAvailable(QSharedPointer<IMessage> message);

private:
    MessageQueue &m_queue;

    // Route: MessageType enum (as quint8) -> list of handler slots
    // Using enum instead of QString for O(1) dispatcher performance
    QMap<quint8, QList<QPair<QObject*, const char*>>> m_handlers;

    QThread *m_dispatcherThread;
    bool m_running;
};
```

**Routing Example Using Enum**:
```cpp
// In MessageDispatcher::onMessageAvailable():
void MessageDispatcher::onMessageAvailable(QSharedPointer<IMessage> message) {
    // O(1) enum-based lookup instead of string comparison
    MessageType type = message->getMessageType();  // Returns enum, not string!

    // Fast enum comparison
    auto handlers_it = m_handlers.find(static_cast<quint8>(type));
    if (handlers_it != m_handlers.end()) {
        // Invoke all registered handlers for this message type
        for (auto& [receiver, slotName] : handlers_it.value()) {
            QMetaObject::invokeMethod(receiver, slotName,
                Qt::QueuedConnection,
                Q_ARG(QSharedPointer<IMessage>, message));
        }
    }
}

### 5. AuditLogger

**Location**: `Src/Common/inc/AuditLogger.hpp`

Banking-compliant audit trail logging.

```cpp
class AuditLogger {
public:
    static AuditLogger& instance();

    void logMessage(const QSharedPointer<IMessage> &message,
                   const QString &status,
                   const QString &details = "");

    void setLogPath(const QString &path);
    bool startLogging();
    void stopLogging();

private:
    AuditLogger();

    // Log format: [TIMESTAMP] [MESSAGE_TYPE] [SOURCE] -> [TARGET] [STATUS]
    // Example: [2026-03-22 14:23:45.123] [AUTH] [LoginModule] -> [BankingSocket] [COMPLETED]
    // Note: Passwords and sensitive data excluded from logs

    QFile m_logFile;
    QMutex m_logMutex;
};
```

**Log Format Example**:
```
[2026-03-22 14:23:45.123] [AUTH] [LoginModule] -> [BankingSocket] [COMPLETED]
[2026-03-22 14:23:46.456] [TRANSACTION] [TransactionModule] -> [BankingSocket] [COMPLETED]
[2026-03-22 14:23:47.789] [ERROR] [BankingSocket] -> [All] [FAILED] - Connection timeout
[2026-03-22 14:23:48.012] [STATUS] [BankingSocket] -> [All] [COMPLETED] - CONNECTED
```

---

## DESIGN PATTERNS

| Pattern | Usage | Location | Benefit |
|---------|-------|----------|---------|
| **Singleton** | Single instance per app | MessageQueue, AuditLogger | Global access, prevents duplicates |
| **Factory** | Create message instances | message subclasses | Encapsulation, extensibility |
| **Observer** | Notify handlers | MessageDispatcher | Loose coupling, event-driven |
| **Priority Queue** | Process by priority | MessageQueue sorting | Critical messages process first |
| **Producer-Consumer** | Async processing | Worker thread pattern | Non-blocking message handling |
| **Request-Response** | Sync communication | sendRequestAndWait() | Blocking until response |
| **Template Method** | Type-specific behavior | IMessage virtual methods | Polymorphism without casting |
| **RAII** | Resource cleanup | QSharedPointer<IMessage> | Automatic memory management |

---

## COMMUNICATION PATTERNS

### Pattern 1: Fire-and-Forget (Async)

**Timeline**: ~1-5ms total

```
Module A                MessageQueue            Module B Handler
   │ enqueueMessage()           │
   ├───────────────────────────►│
   │ (returns immediately)      │ dispatchToHandlers()
   │                            ├──────────────┐
   │                            │           [executes]
   └────────────────────────────┼──────────────┘
        (no wait for result)     │ messageProcessed signal
```

**Use Cases**:
- Error notifications
- Status updates
- Fire-and-forget operations

**Example**:
```cpp
// Send error without waiting
auto errorMsg = std::make_shared<Banking::ErrorMessage>(
    "BankingSocket", "Connection failed");
Banking::MessageQueue::instance().enqueueMessage(errorMsg);
// Returns immediately - no blocking
```

### Pattern 2: Request-Reply (Sync)

**Timeline**: ~10-50ms (depends on handler)

```
Module A             MessageQueue            Module B               Module A
   │ sendRequestAndWait()        │
   │ [BLOCKED - waits]           │
   ├──────────────────────────────►│ dispatch to handler
   │                             │ ├─────────────┐
   │                             │           [processes request]
   │                             │ ├─────────────┘
   │                             │ enqueueMessage(response)
   │                             │◄─ [Response enqueued]
   │ [UNBLOCKED]                 │
   │◄────────────────────────────────┤
   │ [continues with response]    │ │

TIMEOUT: 5000ms (configurable)
```

**Use Cases**:
- Authentication requests
- Transactions (wait for confirmation)
- Synchronous API calls

**Example**:
```cpp
// Send auth request and wait for response
auto authRequest = std::make_shared<Banking::AuthMessage>(
    "LoginModule", "user", "password", true);

QSharedPointer<Banking::IMessage> response;
bool received = Banking::MessageQueue::instance()
    .sendRequestAndWait(authRequest, response, 5000);

if (received) {
    auto authReply = dynamic_cast<AuthMessage*>(response.get());
    if (authReply && authReply->isAuthenticated()) {
        // Handle successful auth
    }
}
```

### Pattern 3: Priority Queue Processing

**Order**: CRITICAL(100) → HIGH(75) → NORMAL(50) → LOW(0)

```
Message Arrival Order:           Queue Processing Order:
1. TxnMsg        (NORMAL=50)      1. ErrorMsg   (CRITICAL=100) ← FIRST
2. ErrorMsg      (CRITICAL=100)   2. AuthMsg    (HIGH=75)
3. StatusMsg     (NORMAL=50)      3. TxnMsg     (NORMAL=50)
4. AuthMsg       (HIGH=75)        4. StatusMsg  (NORMAL=50)
5. AnotherTxnMsg (NORMAL=50)      5. AnotherTxnMsg (NORMAL=50)
```

**Behavior**:
- Messages with same priority processed FIFO
- Higher priority messages bypass the queue
- Prevents error starvation
- Ensures critical operations complete quickly

---

## SEQUENCE DIAGRAMS

### Complete Request-Reply Flow

```
Login Module           MessageQueue           BankingSocket             Audit Logger
    │                      │                        │                         │
    │ 1. Create AuthMsg    │                        │                         │
    ├─ set credentials     │                        │                         │
    │                      │                        │                         │
    │ 2. sendRequestAndWait│                        │                         │
    ├─────────────────────►│                        │                         │
    │ [BLOCKED - waits]    │                        │                         │
    │                      │                        │                         │
    │                      │ 3. Generate unique ID  │                         │
    │                      │ 4. Add to priority queue                         │
    │                      │ 5. Signal: messageEnqueued                       │
    │                      │ 6. Dispatch to handler │                         │
    │                      ├───────────────────────►│                         │
    │                      │                    [process auth]                │
    │                      │                        │ 7. logMessage()         │
    │                      │                        ├───────────────────────►│
    │                      │                        │   [Log: AUTH REQUEST]   │
    │                      │                        │                         │
    │                      │                    [verify credentials]          │
    │                      │                        │ 8. Create AuthMsg(RESP) │
    │                      │                        │ 9. Set authenticated=true
    │                      │                        │ 10. enqueueMessage(resp)
    │                      │◄───────────────────────┤                         │
    │                      │ 11. Match by message ID │                         │
    │                      │ 12. Signal wait condition                        │
    │ 13. [UNBLOCKED]      │                        │                         │
    │◄─────────────────────┤                        │                         │
    │ 14. Receive response │                        │ 15. logMessage()        │
    │ [continues]          │                        ├───────────────────────►│
    │                      │                        │   [Log: AUTH RESPONSE]  │
    │                      │                        │ [authenticated=true]    │
```

### Error Flow

```
Module                 MessageQueue              Audit Logger
   │ throw error            │
   ├─ Create ErrorMsg       │
   ├─ enqueueMessage()      │
   ├──────────────────────►│
   │ (returns immediately)  │
   │                        │ PRIORITY: CRITICAL(100)
   │                        │ Dispatch to all handlers
   │                        │ logMessage()
   │                        ├─────────────────────────►│
   │                        │ [Log: ERROR CRITICAL]    │
   │                        │
   │                        │ [All interested modules notified]
```

---

## DATA FLOW

```
MODULE LAYER (Business Logic)
├─ Login Module
├─ Transaction Module
├─ BankingSocket Module
└─ [Future Modules]
        │
        │ 1. Module calls:
        │    queue.enqueueMessage(authMsg);
        │ OR
        │    queue.sendRequestAndWait(authMsg, response);
        │
        ▼
MESSAGING LAYER (Core Infrastructure)
├─ MessageQueue (Priority Queue)
│  ├─ Thread-safe enqueue/dequeue
│  ├─ Priority-based sorting
│  ├─ Request-Reply matching
│  └─ Timeout handling
│
├─ MessageDispatcher (Router)
│  ├─ Routes to module handlers
│  ├─ Invokes Qt slot methods
│  └─ Handles signal delivery
│
└─ AuditLogger (Compliance)
   ├─ Timestamps all messages
   ├─ Writes to file
   └─ Tracks transaction history

        │
        │ 2. Process based on:
        │    - Priority level
        │    - Handler registration
        │    - Request-Reply matching
        │
        ▼
STORAGE LAYER
├─ Memory Queue (runtime)
├─ Audit Log File (logs/messages_audit.log)
├─ Pending Responses Map
└─ Handler Subscriptions
```

---

## INHERITANCE & POLYMORPHISM

### Key Design Principle: **NO Downcasting**

All message operations use **virtual methods on IMessage base class**. This eliminates the need for unsafe casting.

### Inheritance Hierarchy

```
IMessage (Abstract Base Class)
│
├─ All virtual methods defined here
│   ├─ getMessageType()
│   ├─ getPriority()
│   ├─ isRequest()
│   ├─ validate()
│   ├─ serialize()
│   ├─ deserialize()
│   ├─ getPayloadSummary()
│   └─ getTimeoutMs()
│
├─ AuthMessage
│   ├─ Override all virtual methods
│   ├─ Add type-specific fields (username, password)
│   ├─ Add type-specific methods (setAuthenticated)
│   └─ Type-specific validation
│
├─ TransactionMessage
│   ├─ Override all virtual methods
│   ├─ Add type-specific fields (toAccount, amount)
│   └─ Type-specific validation
│
├─ ErrorMessage
│   ├─ Override all virtual methods
│   ├─ Add type-specific fields (errorCode, errorDesc)
│   └─ Type-specific validation
│
└─ StatusMessage
    ├─ Override all virtual methods
    ├─ Add type-specific fields (connectionStatus)
    └─ Type-specific validation
```

### Virtual Method Polymorphism (Recommended Approach)

```cpp
// ✅ PREFERRED - No casting needed
void processMessage(QSharedPointer<IMessage> msg) {
    // Validates message - correct override called based on type
    if (!msg->validate()) {
        m_logger.logMessage(msg, "VALIDATION_FAILED");
        return;
    }

    // All operations use virtual methods
    m_logger.logMessage(msg, "VALIDATED");
    m_queue.enqueue(msg);

    // For serialization/transport
    QByteArray serialized = msg->serialize();
    // ...
}

// Handler doesn't need to know concrete type
void handleAnyMessage(QSharedPointer<IMessage> msg) {
    QString type = msg->getMessageType();        // Works for all types
    int priority = msg->getPriority();           // Works for all types
    QString summary = msg->getPayloadSummary();  // Type-specific impl
    bool isReq = msg->isRequest();               // Works for all types
}
```

### Type-Specific Access (Rare Cases Only)

```cpp
// ✅ ACCEPTABLE - Safe casting with type guard
void handleAuthResponse(QSharedPointer<IMessage> msg) {
    // Check type first
    if (msg->getMessageType() != "AUTH") {
        return;  // Wrong message type
    }

    // Safe downcasting (qobject_cast returns nullptr if wrong type)
    if (auto authMsg = qobject_cast<AuthMessage*>(msg.get())) {
        // Now safe to access type-specific fields
        bool authenticated = authMsg->isAuthenticated();
        if (authenticated) {
            // Handle successful auth
        }
    }
}

// ❌ AVOID - Unsafe casting
void badHandler(QSharedPointer<IMessage> msg) {
    // This is DANGEROUS - may not be AuthMessage!
    AuthMessage* auth = static_cast<AuthMessage*>(msg.get());
    bool authenticated = auth->isAuthenticated();  // May crash!
}
```

### Benefits of Virtual Method Design

| Aspect | Benefit |
|--------|---------|
| **Type Safety** | Compiler checks virtual method calls |
| **Simplicity** | No complex casting logic |
| **Maintainability** | New message types work automatically |
| **Performance** | Virtual dispatch has minimal overhead |
| **Testing** | Easy to mock and test with base class pointers |
| **Extensibility** | Add new message types without modifying dispatch code |

---

## IMPLEMENTATION ARCHITECTURE

### Integration Points

#### 1. Modify StartupManager::initCore()

```cpp
void StartupManager::initCore() {
    // Initialize MessageQueue (singleton)
    auto &queue = Banking::MessageQueue::instance();

    // Initialize AuditLogger
    auto &logger = Banking::AuditLogger::instance();
    logger.setLogPath("logs/messages_audit.log");
    logger.startLogging();

    // Initialize MessageDispatcher
    m_messageDispatcher = std::make_unique<Banking::MessageDispatcher>(queue);
    m_messageDispatcher->start();
}
```

#### 2. Modify StartupManager::initModules()

```cpp
void StartupManager::initModules() {
    // ... existing code ...

    // NEW: Register module handlers with dispatcher
    for (auto &module : m_modules) {
        m_messageDispatcher->registerModuleHandler(module.get());
    }

    // ... call init() and start() on each module ...
}
```

#### 3. Extend BankingSocket Signal Handlers

**Overview**: BankingSocket keeps its existing internal signal-slot connections to QTcpSocket. We EXTEND these handlers to publish messages to the MessageQueue so other modules get notified.

```cpp
// BankingSocket.hpp - Keep existing signals
class BankingSocket : public QTcpSocket {
    Q_OBJECT

private slots:
    void handleConnected();      // Existing - now EXTENDED
    void handleDisconnected();   // Existing - now EXTENDED
    void handleError();          // Existing - now EXTENDED
    void handleReadyRead();      // Existing - now EXTENDED
};

// BankingSocket.cpp - Extend existing handlers

// Constructor (existing code unchanged)
BankingSocket::BankingSocket() {
    // Connect internal socket signals to our slots (KEEP EXISTING)
    connect(this, &QTcpSocket::connected, this, &BankingSocket::handleConnected);
    connect(this, &QTcpSocket::disconnected, this, &BankingSocket::handleDisconnected);
    connect(this, &QTcpSocket::errorOccurred, this, &BankingSocket::handleError);
    connect(this, &QTcpSocket::readyRead, this, &BankingSocket::handleReadyRead);
}

// EXTENDED: handleConnected() - now publishes StatusMessage
void BankingSocket::handleConnected() {
    qDebug() << "Connected to banking server";

    // Step 1: Existing internal logic (if any)
    // ...

    // Step 2: NEW - Notify all modules via MessageQueue
    auto statusMsg = std::make_shared<Banking::StatusMessage>(
        "BankingSocket",
        Banking::StatusMessage::ConnectionStatus::CONNECTED
    );
    Banking::MessageQueue::instance().enqueueMessage(statusMsg);
}

// EXTENDED: handleDisconnected() - now publishes StatusMessage
void BankingSocket::handleDisconnected() {
    qDebug() << "Disconnected from banking server";

    // NEW - Notify all modules
    auto statusMsg = std::make_shared<Banking::StatusMessage>(
        "BankingSocket",
        Banking::StatusMessage::ConnectionStatus::DISCONNECTED
    );
    Banking::MessageQueue::instance().enqueueMessage(statusMsg);
}

// EXTENDED: handleError() - now publishes ErrorMessage
void BankingSocket::handleError() {
    QString errorDescription = this->errorString();
    int errorCode = this->error();

    qWarning() << "Socket error [" << errorCode << "]:" << errorDescription;

    // NEW - Broadcast error to ALL modules (CRITICAL priority)
    auto errorMsg = std::make_shared<Banking::ErrorMessage>(
        "BankingSocket",
        errorDescription,
        errorCode
    );
    Banking::MessageQueue::instance().enqueueMessage(errorMsg);
}

// EXTENDED: handleReadyRead() - now creates response messages
void BankingSocket::handleReadyRead() {
    QByteArray data = readAll();
    qDebug() << "Received data:" << data;

    // Step 1: Parse incoming data (existing logic)
    // This determines the message type (AUTH response, TXN response, etc.)

    // Step 2: NEW - Create appropriate Message object
    // Example: If server sent AUTH_SUCCESS response
    if (data.contains("AUTH_SUCCESS")) {
        auto authResponse = std::make_shared<Banking::AuthMessage>(
            "BankingSocket",          // source
            "authenticated_user",     // username (from server response)
            "",                       // password (empty in response)
            false                     // this is a response, not a request
        );
        authResponse->setAuthenticated(true);
        authResponse->setMessageId(m_lastAuthRequestId);  // Match with request

        // Step 3: Enqueue response → LoginModule waiting for this gets unblocked
        Banking::MessageQueue::instance().enqueueMessage(authResponse);
    }

    // Example: If server sent TRANSACTION_CONFIRMED
    else if (data.contains("TXN_CONFIRMED")) {
        auto txnResponse = std::make_shared<Banking::TransactionMessage>(
            "BankingSocket",
            "recipient_account",
            1000.00,
            false  // response
        );
        txnResponse->setSuccessful(true);
        txnResponse->setMessageId(m_lastTxnRequestId);

        Banking::MessageQueue::instance().enqueueMessage(txnResponse);
    }
}
```

**Register MessageHandlers** (in BankingSocket::start() or CommunicationInitializer):

```cpp
bool BankingSocket::start() {
    // ... existing code ...

    // NEW: Register as handler for incoming messages
    auto &dispatcher = Banking::MessageDispatcher::instance();

    // Subscribe to AUTH requests
    dispatcher.registerHandler(Banking::MessageType::AUTH, this,
        SLOT(onAuthMessageReceived(QSharedPointer<IMessage>)));

    // Subscribe to TRANSACTION requests
    dispatcher.registerHandler(Banking::MessageType::TRANSACTION, this,
        SLOT(onTransactionReceived(QSharedPointer<IMessage>)));

    return true;
}

// Handle incoming AUTH request from LoginModule
void BankingSocket::onAuthMessageReceived(QSharedPointer<Banking::IMessage> msg) {
    auto authRequest = qobject_cast<Banking::AuthMessage*>(msg.get());
    if (!authRequest) return;

    // Store request ID to match response later
    m_lastAuthRequestId = authRequest->getMessageId();

    // Send to server
    QString credentials = QString("%1:%2")
        .arg(authRequest->getUsername(), authRequest->getPassword());

    this->write(credentials.toUtf8());
}

// Handle incoming TRANSACTION request from other modules
void BankingSocket::onTransactionReceived(QSharedPointer<Banking::IMessage> msg) {
    auto txnRequest = qobject_cast<Banking::TransactionMessage*>(msg.get());
    if (!txnRequest) return;

    // Store request ID to match response later
    m_lastTxnRequestId = txnRequest->getMessageId();

    // Send to server
    QString txnData = QString("TXN|%1|%2")
        .arg(txnRequest->getToAccount(), QString::number(txnRequest->getAmount()));

    this->write(txnData.toUtf8());
}
```

#### 4. Other Modules Listen to Status/Error Updates

**Example: LoginModule listens to connection status and errors**

```cpp
// LoginModule.hpp
class LoginModule : public IModule {
    Q_OBJECT

private slots:
    void onConnectionStatusChanged(QSharedPointer<Banking::IMessage> msg);
    void onErrorOccurred(QSharedPointer<Banking::IMessage> msg);
};

// LoginModule.cpp
bool LoginModule::start() {
    auto &dispatcher = Banking::MessageDispatcher::instance();

    // Listen to connection status updates from BankingSocket
    dispatcher.registerHandler(Banking::MessageType::STATUS, this,
        SLOT(onConnectionStatusChanged(QSharedPointer<IMessage>)));

    // Listen to error notifications
    dispatcher.registerHandler(Banking::MessageType::ERROR, this,
        SLOT(onErrorOccurred(QSharedPointer<IMessage>)));

    return IModule::start();
}

void LoginModule::onConnectionStatusChanged(QSharedPointer<Banking::IMessage> msg) {
    auto statusMsg = qobject_cast<Banking::StatusMessage*>(msg.get());
    if (!statusMsg) return;

    switch (statusMsg->getStatus()) {
        case Banking::StatusMessage::ConnectionStatus::CONNECTED:
            qDebug() << "Server connected! UI can now accept login";
            enableLoginUI();
            break;

        case Banking::StatusMessage::ConnectionStatus::DISCONNECTED:
            qDebug() << "Server disconnected! Disable login UI";
            disableLoginUI();
            showOfflineMessage();
            break;

        case Banking::StatusMessage::ConnectionStatus::RECONNECTING:
            qDebug() << "Server reconnecting...";
            showReconnectingIndicator();
            break;

        case Banking::StatusMessage::ConnectionStatus::CONNECTING:
            qDebug() << "Connecting to server...";
            showConnectingIndicator();
            break;
    }
}

void LoginModule::onErrorOccurred(QSharedPointer<Banking::IMessage> msg) {
    auto errorMsg = qobject_cast<Banking::ErrorMessage*>(msg.get());
    if (!errorMsg) return;

    qWarning() << "Error from" << msg->getSourceModule()
               << ":" << errorMsg->getErrorDescription();

    // Handle error (show dialog, disable UI, etc.)
    showErrorDialog(errorMsg->getErrorDescription());
}

// Request-Reply Pattern: Send auth and wait for response
void LoginModule::attemptLogin(const QString &username, const QString &password) {
    // Step 1: Create AUTH request message
    auto authRequest = std::make_shared<Banking::AuthMessage>(
        "LoginModule",
        username,
        password,
        true  // This is a REQUEST
    );

    // Step 2: Send request and WAIT for response (blocking)
    QSharedPointer<Banking::IMessage> response;
    bool received = Banking::MessageQueue::instance()
        .sendRequestAndWait(authRequest, response, 5000);  // 5 second timeout

    // Step 3: Handle response
    if (!received) {
        showErrorDialog("Login timeout - server not responding");
        return;
    }

    auto authResponse = qobject_cast<Banking::AuthMessage*>(response.get());
    if (!authResponse) {
        showErrorDialog("Unexpected response from server");
        return;
    }

    if (authResponse->isAuthenticated()) {
        handleLoginSuccess();
    } else {
        handleLoginFailure("Invalid username or password");
    }
}
```

### Complete End-to-End Communication Flow

```
┌─────────────────────────────────────────────────────────────────────────┐
│ SCENARIO: User performs login                                           │
└─────────────────────────────────────────────────────────────────────────┘

PHASE 1: CONNECTION ESTABLISHMENT (Fire-and-Forget)
════════════════════════════════════════════════════

QTcpSocket                BankingSocket           MessageQueue            LoginModule
    │                           │                       │                      │
    │ connectToServer()         │                       │                      │
    │◄─ (existing code)         │                       │                      │
    │                           │                       │                      │
    │ connected signal          │                       │                      │
    ├──────────────────────────►│ handleConnected()    │                      │
    │                           ├─ Create StatusMsg   │                      │
    │                           ├─ enqueueMessage()   │                      │
    │                           ├──────────────────────►│                      │
    │                           │                  Dispatcher routes         │
    │                           │                       ├─ onConnectionStatusChanged()
    │                           │                       ├──────────────────────►│
    │                           │                       │               [Notify UI: Server Connected]
    │                           │                       │


PHASE 2: LOGIN REQUEST (Request-Reply, Blocking)
═════════════════════════════════════════════════

LoginModule             MessageQueue         MessageDispatcher      BankingSocket       QTcpSocket
    │                       │                       │                    │                  │
    │ User clicks Login     │                       │                    │                  │
    ├─ attemptLogin()      │                       │                    │                  │
    ├─ Create AUTH msg     │                       │                    │                  │
    ├─ sendRequestAndWait()│                       │                    │                  │
    ├──────────────────────►│                       │                    │                  │
    │ [BLOCKED - waiting]   │ enqueue(AUTH_REQ)    │                    │                  │
    │                       ├───────────────────────►│ dispatch          │                  │
    │                       │                       ├─ onAuthReceived() │                  │
    │                       │                       ├───────────────────►│                  │
    │                       │                       │           ├─ Extract credentials
    │                       │                       │           ├─ format request
    │                       │                       │           ├─────────────────────────►│
    │                       │                       │           │         write(credentials)
    │                       │                       │           │
    │                       │                       │           │(waits for server response)
    │
    │ Server responds with AUTH_SUCCESS
    │
    QTcpSocket fires readyRead signal
    │                       │                       │                    │                  │
    │                       │                       │                    │ readyRead signal │
    │                       │                       │                    │◄─────────────────┤
    │                       │                       │                    │
    │                       │                       │         handleReadyRead()
    │                       │                       │           ├─ Parse server response
    │                       │                       │           ├─ Create AUTH response msg
    │                       │                       │           ├─ Set: authenticated=true
    │                       │                       │           ├─ Set: messageId=[original request id]
    │                       │                       │           ├─ enqueueMessage()
    │                       │                       │                    │
    │                       │                       │  Match by messageId
    │                       │◄──────────────────────────────────────────┤
    │ [UNBLOCKED!]          │  Stored in QWaitCondition
    │ Response received     │  signal()
    │◄──────────────────────┤
    │ Parse response         │
    ├─ isAuthenticated==true
    ├─ handleLoginSuccess() │
    │ [User logged in!]      │


PHASE 3: ERROR SCENARIO (Fire-and-Forget)
═════════════════════════════════════════

Network fails (socket disconnect/error)

QTcpSocket             BankingSocket          MessageQueue           All Modules
    │                      │                       │                      │
    │ disconnectFromHost() │                       │                      │
    │◄─────────────────────┤                       │                      │
    │ errorOccurred signal │                       │                      │
    ├─────────────────────►│ handleError()         │                      │
    │                      ├─ Create ErrorMsg     │                      │
    │                      │   (CRITICAL priority)│                      │
    │                      ├─ enqueueMessage()   │                      │
    │                      ├──────────────────────►│                      │
    │                      │              BROADCAST ERROR
    │                      │              (all listening modules receive)
    │                      │                       ├─────────────────────►│
    │                      │                       │            onErrorOccurred()
    │                      │                       │            [Handle error: show dialog]
```

### File Structure

```
Src/Common/
├── Interfaces/
│   ├── IModule.hpp              [EXISTING]
│   └── IMessage.hpp             [NEW] - Abstract message base
│
├── inc/
│   ├── MessageQueue.hpp         [NEW]
│   ├── MessageDispatcher.hpp    [NEW]
│   └── AuditLogger.hpp          [NEW]
│
├── src/
│   ├── MessageQueue.cpp         [NEW]
│   ├── MessageDispatcher.cpp    [NEW]
│   └── AuditLogger.cpp          [NEW]
│
├── Messages/
│   ├── inc/
│   │   ├── AuthMessage.hpp      [NEW]
│   │   ├── TransactionMessage.hpp   [NEW]
│   │   ├── ErrorMessage.hpp     [NEW]
│   │   └── StatusMessage.hpp    [NEW]
│   │
│   └── Src/
│       ├── AuthMessage.cpp      [NEW]
│       ├── TransactionMessage.cpp   [NEW]
│       ├── ErrorMessage.cpp     [NEW]
│       └── StatusMessage.cpp    [NEW]
│
└── [Existing files remain unchanged]

Src/Modules/Communications/
├── inc/
│   └── BankingSocket.hpp        [MODIFY - Add handlers]
└── BankingSocket.cpp            [MODIFY - Implement handlers]
```

---

## TESTING STRATEGY

### Test Framework
**Qt Test** (built-in with Qt, no external dependencies)

### Test Coverage Plan

| Category | Test Count | Coverage |
|----------|-----------|----------|
| Message Classes | 12 tests | ✅ Validation, serialization, priorities |
| MessageQueue | 18 tests | ✅ Enqueue, dequeue, priority, request-reply, thread safety |
| MessageDispatcher | 10 tests | ✅ Routing, handler registration, slot invocation |
| AuditLogger | 8 tests | ✅ File logging, timestamps, sensitive data filtering |
| Integration Tests | 15 tests | ✅ End-to-end flows, module communication |
| Thread Safety Tests | 10 tests | ✅ Race conditions, deadlock detection, stress testing |
| Performance Tests | 6 tests | ✅ Throughput, latency, memory, scalability |
| **Total** | **79 tests** | **Complete coverage** |

### Test Categories

#### A. Message Classes Tests (12 tests)
```
✓ AuthMessage validation (valid/invalid username/password)
✓ AuthMessage serialization/deserialization
✓ AuthMessage requires encryption
✓ TransactionMessage validation (account, amount)
✓ TransactionMessage serialization
✓ TransactionMessage payload summary
✓ ErrorMessage validation
✓ ErrorMessage priority (CRITICAL)
✓ StatusMessage construction
✓ StatusMessage is not a request
✓ Message ID generation & uniqueness
✓ Message timestamp accuracy
```

#### B. MessageQueue Tests (18 tests)
```
✓ Singleton pattern (single instance)
✓ Enqueue single message
✓ Dequeue FIFO order
✓ Priority ordering (CRITICAL > HIGH > NORMAL)
✓ Request-Reply basic flow
✓ Request-Reply timeout
✓ Concurrent enqueue (thread safety)
✓ Max queue size enforcement
✓ Queue statistics (size, counts)
✓ Subscribe/unsubscribe handlers
✓ Message not found dequeue
✓ Priority processing consistency
✓ Empty queue dequeue timeout
✓ Multiple concurrent requests matching
✓ Thread-safe operations
✓ Audit logging enabled
✓ Pending response cleanup
✓ Message processed signal emission
```

#### C. MessageDispatcher Tests (10 tests)
```
✓ Register handler for message type
✓ Route to correct handler
✓ Multiple handlers for same type
✓ Handler invocation via Qt slot
✓ Unregister handler
✓ Start dispatcher
✓ Stop dispatcher
✓ Multiple message types routing
✓ Handler execution order
✓ Error handling in handlers
```

#### D. AuditLogger Tests (8 tests)
```
✓ Message logging to file
✓ Timestamp accuracy and format
✓ Sensitive data exclusion (passwords)
✓ Log file rotation when size exceeded
✓ Concurrent logging (thread-safe)
✓ Log format validation
✓ File path configuration
✓ Log startup/shutdown
```

#### E. Integration Tests (15 tests)
```
✓ Complete auth request-reply flow
✓ Complete transaction flow
✓ Multiple message types in queue simultaneously
✓ Priority precedence in processing
✓ Module communication end-to-end
✓ Audit trail consistency
✓ Error handling and propagation
✓ Module notification on message arrival
✓ Timeout behavior
✓ Message filtering by type
✓ Queue statistics updates
✓ Signal/slot connections
✓ Dispatcher routing accuracy
✓ Audit log contains all transactions
✓ System shutdown cleanup
```

#### F. Thread Safety Tests (10 tests)
```
✓ Concurrent enqueue from 100 threads
✓ Concurrent dequeue from 100 threads
✓ Concurrent enqueue & dequeue
✓ Request-Reply with multiple threads
✓ Audit logging multithread
✓ Race condition: message matching
✓ Deadlock detection (timeout-based)
✓ Memory isolation between threads
✓ Stress test: 1000 messages, 10 threads
✓ Priority consistency with concurrent access
```

#### G. Performance Tests (6 tests)
```
✓ Throughput: ≥1000 messages/sec
✓ Latency: <10ms per queue operation
✓ Memory: queue growth vs entry count
✓ CPU: stress with 10 concurrent writers
✓ Scalability: message count vs latency
✓ Audit logging overhead
```

---

## IMPLEMENTATION PHASES

### Phase 1: Core Infrastructure (Days 1-2)

**Deliverables:**
- [ ] IMessage interface with virtual methods
- [ ] AuthMessage implementation
- [ ] TransactionMessage implementation
- [ ] ErrorMessage implementation
- [ ] StatusMessage implementation
- [ ] MessageQueue singleton with priority queue
- [ ] 30 unit tests (messages + queue)

**Files Created:**
- `Src/Common/Interfaces/IMessage.hpp`             # Abstract base interface
- `Src/Common/Messages/inc/AuthMessage.hpp`
- `Src/Common/Messages/inc/TransactionMessage.hpp`
- `Src/Common/Messages/inc/ErrorMessage.hpp`
- `Src/Common/Messages/inc/StatusMessage.hpp`
- `Src/Common/Messages/Src/AuthMessage.cpp`
- `Src/Common/Messages/Src/TransactionMessage.cpp`
- `Src/Common/Messages/Src/ErrorMessage.cpp`
- `Src/Common/Messages/Src/StatusMessage.cpp`
- `Src/Common/inc/MessageQueue.hpp`
- `Src/Common/src/MessageQueue.cpp`

### Phase 2: Dispatcher & Routing (Days 2-3)

**Deliverables:**
- [ ] MessageDispatcher implementation
- [ ] Handler registration system
- [ ] Qt signal/slot integration
- [ ] Request-Reply pattern with timeout
- [ ] 10 unit tests (dispatcher)

**Files Created:**
- `Src/Common/inc/MessageDispatcher.hpp`
- `Src/Common/src/MessageDispatcher.cpp`

### Phase 3: Audit & Logging (Day 3)

**Deliverables:**
- [ ] AuditLogger singleton
- [ ] File logging mechanism
- [ ] Audit trail formatting
- [ ] Log rotation
- [ ] 8 unit tests (logger)

**Files Created:**
- `Src/Common/inc/AuditLogger.hpp`
- `Src/Common/src/AuditLogger.cpp`

### Phase 4: Integration & Testing (Days 4-5)

**Deliverables:**
- [ ] Modify StartupManager::initCore()
- [ ] Modify StartupManager::initModules()
- [ ] Add handlers to BankingSocket module
- [ ] Update CMakeLists.txt
- [ ] 25 integration + thread safety + performance tests
- [ ] All 79 tests passing

**Files Modified:**
- `Src/Common/inc/StartupManager.hpp`
- `Src/Common/StartupManager.cpp`
- `Src/Modules/Communications/inc/CommunicationInitializer.hpp`
- `Src/Modules/Communications/CommunicationInitializer.cpp`
- `CMakeLists.txt`

### Phase 5: Documentation (Day 5)

**Deliverables:**
- [ ] Doxygen comments for all classes
- [ ] Usage examples
- [ ] Developer guide for custom messages
- [ ] README updates
- [ ] Final integration testing

---

## VERIFICATION & ACCEPTANCE CRITERIA

### Build Verification
- ✓ All new files compile without errors
- ✓ Zero compilation warnings
- ✓ CMakeLists.txt updated correctly
- ✓ No breaking changes to existing modules

### Unit Test Verification
- ✓ All 79 unit tests pass (100% pass rate)
- ✓ Code coverage ≥90% for MessageQueue & Dispatcher
- ✓ All thread safety tests pass (no race conditions)
- ✓ Performance tests meet latency targets (<10ms)

### Integration Verification
- ✓ Login module can send AUTH request to BankingSocket
- ✓ BankingSocket sends AUTH response back to Login module
- ✓ Messages processed in correct priority order
- ✓ Audit log contains all transactions with timestamps
- ✓ Error handling propagates correctly

### Documentation Verification
- ✓ Full Doxygen documentation for all classes
- ✓ Usage examples for each message type
- ✓ Developer guide for adding custom messages
- ✓ README updated with MessageQueue overview

---

## REQUIREMENTS MATRIX

| Requirement | Implementation | Test Location | Test Count |
|-------------|-----------------|--------------|-----------|
| FR1: Fire-and-Forget | `MessageQueue::enqueueMessage()` | test_message_queue.cpp | 3 |
| FR2: Request-Reply | `MessageQueue::sendRequestAndWait()` | test_message_queue.cpp | 4 |
| FR3: Priority Queue | Priority ordering in queue | test_message_queue.cpp | 2 |
| FR4: Type-Safe Messages | IMessage + subclasses | test_messages.cpp | 12 |
| FR5: Thread Safety | QMutex protection | test_thread_safety.cpp | 10 |
| FR6: Message Routing | MessageDispatcher | test_dispatcher.cpp | 10 |
| FR7: Audit Trail | AuditLogger | test_audit_logger.cpp | 8 |
| FR8: Message Validation | IMessage::validate() | test_messages.cpp | 4 |
| FR9: Error Handling | ErrorMessage + routing | test_integration.cpp | 1 |
| FR10: Timeout Handling | sendRequestAndWait() | test_message_queue.cpp | 2 |

---

## RISK MITIGATION

| Risk | Impact | Probability | Mitigation Strategy |
|------|--------|-------------|-------------------|
| **Deadlocks in multi-threaded env** | HIGH | MEDIUM | Use QMutex with timeout, extensive thread safety tests, lock ordering analysis |
| **Memory leaks in shared_ptr** | MEDIUM | LOW | Careful ownership management, valgrind/AddressSanitizer testing |
| **Priority inversion** | MEDIUM | LOW | Use proven priority queue algorithm, extensive testing |
| **Audit log file size** | LOW | LOW | Implement log rotation when exceeded, monitor file size |
| **Message serialization errors** | MEDIUM | MEDIUM | Comprehensive validation in all message types, unit tests |
| **Handler execution exceptions** | MEDIUM | MEDIUM | Try-catch in dispatcher, error message propagation |
| **Qt version compatibility** | LOW | LOW | Test with target Qt version, use stable APIs |
| **Performance degradation** | LOW | LOW | Performance testing, latency monitoring |

---

## SUCCESS METRICS

| Metric | Target | Verification Method |
|--------|--------|------------------|
| **Code Quality** | 0 compiler warnings | Build log inspection |
| **Test Coverage** | ≥90% for core | Code coverage report |
| **Test Pass Rate** | 100% (79/79 tests) | Test execution |
| **Performance** | <10ms queue ops | Performance test suite |
| **Throughput** | ≥1000 msg/sec | Load testing |
| **Thread Safety** | 0 data races | Thread sanitizer + tests |
| **Documentation** | 100% of classes | Doxygen output |
| **Functionality** | All 10 FRs met | Integration test suite |

---

## SUMMARY

### What This Delivers

✅ **Robust Foundation**: Clean abstraction layer for inter-module communication
✅ **Enterprise Features**: Priority queues, audit logging, thread safety
✅ **Banking Ready**: Secure (encrypted auth), compliant (audit trail), reliable
✅ **Extensible**: New message types without core modifications
✅ **Well-Tested**: 79 comprehensive unit tests
✅ **Qt Native**: No external dependencies, leverages Qt ecosystem
✅ **Type-Safe**: Virtual method design eliminates unsafe casting
✅ **Performance**: <10ms latency, 1000+ msg/sec throughput

### Next Steps

1. ✅ **Approve this plan** - Confirm architecture and approach
2. ✅ **Start Phase 1** - Implement IMessage & concrete types
3. ✅ **Run unit tests** - Verify each component
4. ✅ **Complete Phase 2-4** - Full integration
5. ✅ **Final documentation** - Doxygen & examples
6. ✅ **Deploy** - Integrate with StartupManager

### Document References

- **This Document**: Complete implementation guide with all details
- **Plan File**: `/Users/shubhamkalihari/.claude/plans/magical-forging-finch.md`
- **Quick Reference**: `/Users/shubhamkalihari/Qt/QTProjects/Banking-UI/MESSAGEQUEUE_PLAN.md`

---

**Document Status**: Ready for Approval ✓
**Last Updated**: March 2026
**Prepared By**: Claude Code
**For**: Banking-UI Project Team

---

## APPENDIX: Quick Command Reference

### Building

```bash
cd /Users/shubhamkalihari/Qt/QTProjects/Banking-UI
mkdir build && cd build
cmake ..
make
```

### Running Tests (after implementation)

```bash
./tests/test_messages
./tests/test_message_queue
./tests/test_dispatcher
./tests/test_audit_logger
./tests/test_integration
./tests/test_thread_safety
./tests/test_performance
```

### Checking Audit Logs

```bash
tail -f logs/messages_audit.log
```

### Code Coverage (with lcov)

```bash
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON ..
make
make coverage
```

---

**END OF COMPLETE IMPLEMENTATION PLAN**
