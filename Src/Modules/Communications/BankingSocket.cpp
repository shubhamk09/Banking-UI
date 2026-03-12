#include "BankingSocket.hpp"
#include <QHostAddress>
#include <QDebug>

namespace Banking {
namespace Communications {

BankingSocket::BankingSocket(QObject *parent)
    : QTcpSocket(parent)
{
    // Connect internal socket signals to our slots
    connect(this, &QTcpSocket::connected, this, &BankingSocket::handleConnected);
    connect(this, &QTcpSocket::disconnected, this, &BankingSocket::handleDisconnected);
    connect(this, &QTcpSocket::errorOccurred, this, &BankingSocket::handleError);
    connect(this, &QTcpSocket::readyRead, this, &BankingSocket::handleReadyRead);
}

BankingSocket::~BankingSocket()
{
    disconnectFromServer();
}

bool BankingSocket::connectToServer(const std::string &host, quint16 port)
{
    // Close any existing connection
    if (state() == QAbstractSocket::ConnectedState) {
        disconnectFromServer();
    }

    // Connect to the server
    QHostAddress address(QString::fromStdString(host));
    connectToHost(address, port);

    // Wait for connection
    return waitForConnected(TIMEOUT_MS);
}

void BankingSocket::disconnectFromServer()
{
    if (state() != QAbstractSocket::UnconnectedState) {
        disconnectFromHost();
        if (state() != QAbstractSocket::UnconnectedState) {
            waitForDisconnected(TIMEOUT_MS);
        }
    }
}

bool BankingSocket::sendData(const QByteArray &data)
{
    if (state() != QAbstractSocket::ConnectedState) {
        emit errorOccurred("Not connected to server");
        return false;
    }

    qint64 bytesWritten = write(data);
    if (bytesWritten != data.size()) {
        emit errorOccurred("Failed to write all data");
        return false;
    }

    return waitForBytesWritten(TIMEOUT_MS);
}

QByteArray BankingSocket::receiveData()
{
    if (state() != QAbstractSocket::ConnectedState) {
        emit errorOccurred("Not connected to server");
        return QByteArray();
    }

    if (!waitForReadyRead(TIMEOUT_MS)) {
        emit errorOccurred("Timeout waiting for data");
        return QByteArray();
    }

    return readAll();
}

void BankingSocket::handleConnected()
{
    emit connectionStatusChanged(true);
    qDebug() << "Connected to server";
}

void BankingSocket::handleDisconnected()
{
    emit connectionStatusChanged(false);
    qDebug() << "Disconnected from server";
}

void BankingSocket::handleError(QAbstractSocket::SocketError socketError)
{
    QString errorMessage = QString("Socket error: %1").arg(errorString());
    emit errorOccurred(errorMessage);
    qDebug() << errorMessage;
}

void BankingSocket::handleReadyRead()
{
    QByteArray data = readAll();
    m_buffer.append(data);
    emit dataReceived(data);
}

} // namespace communications
} // namespace Banking