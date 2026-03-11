
#pragma once
#include <QTcpSocket>
#include <QHostAddress>

namespace Banking {
namespace Communications {

/**
 * @class BankingSocket
 * @brief TCP socket wrapper for banking communications.
 *
 * Provides connection management and data transmission for banking modules.
 */
class BankingSocket : public QTcpSocket
{
    Q_OBJECT

public:
    /**
     * @brief Constructor.
     * @param parent Parent QObject.
     */
    explicit BankingSocket(QObject *parent = nullptr);

    /**
     * @brief Destructor.
     */
    ~BankingSocket();

    /**
     * @brief Connect to a TCP server.
     * @param host Host address as string.
     * @param port Port number.
     * @return True if connected successfully, false otherwise.
     */
    bool connectToServer(const QString &host, quint16 port);

    /**
     * @brief Disconnect from the server.
     */
    void disconnectFromServer();

    /**
     * @brief Send data to the server.
     * @param data Data to send.
     * @return True if all data sent successfully, false otherwise.
     */
    bool sendData(const QByteArray &data);

    /**
     * @brief Receive data from the server.
     * @return Received data as QByteArray.
     */
    QByteArray receiveData();

signals:
    /**
     * @brief Emitted when data is received from the server.
     * @param data The received data.
     */
    void dataReceived(const QByteArray &data);

    /**
     * @brief Emitted when the connection status changes.
     * @param connected True if connected, false otherwise.
     */
    void connectionStatusChanged(bool connected);

    /**
     * @brief Emitted when a socket error occurs.
     * @param error Error message.
     */
    void errorOccurred(const QString &error);

private slots:
    /** @brief Handles successful connection event. */
    void handleConnected();

    /** @brief Handles disconnection event. */
    void handleDisconnected();

    /** @brief Handles socket error event. */
    void handleError(QAbstractSocket::SocketError socketError);

    /** @brief Handles ready-read event (incoming data). */
    void handleReadyRead();

private:
    static const int TIMEOUT_MS = 5000;  ///< 5 second timeout for socket operations.
    QByteArray m_buffer;                 ///< Buffer for incoming data.
};

} // namespace Communications
} // namespace Banking


