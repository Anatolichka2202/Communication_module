#include "udpclient.h"
#include "../comm_dependencies.h"
#include <QNetworkDatagram>
#include <QDebug>
#include <QThread>
#include <QVariant>
UDPClient::UDPClient(QObject* parent)
    : QObject(parent)
    , m_socket(nullptr)
    , m_isBound(false)
    , m_boundPort(0)
{
    // Сокет будет создан позже в initializeInThread()
}

UDPClient::~UDPClient()
{
    LOG_CAT_INFO("UDP", "UDPClient деструктор");

    if (m_socket) {
        m_socket->close();
        delete m_socket;
        m_socket = nullptr;
    }
}

void UDPClient::initializeInThread()
{
    LOG_INFO("UDPClient::initializeInThread - поток: " +
             QString::number((qulonglong)QThread::currentThreadId()));

    // Этот метод должен вызываться в том потоке, где будет работать UDPClient
    if (m_socket) {
        LOG_CAT_WARNING("UDP", " уже инициализирован");
        return;
    }

    try {
        setupSocket();
        LOG_CAT_INFO("UDP","успешно инициализирован в потоке");
        emit initialized();

    } catch (const std::exception& e) {
        LOG_CAT_ERROR("UDP", "Ошибка инициализации: " + QString(e.what()));
        emit errorOccurred(QString("Ошибка инициализации UDPClient: %1").arg(e.what()));
    }
}

void UDPClient::setupSocket()
{
    LOG_CAT_INFO("UDP", "::setupSocket");

    // Создаем сокет
    m_socket = new QUdpSocket(this);

    // Настройки сокета для улучшения производительности
    m_socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    m_socket->setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, 65536);
    m_socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 65536);

    // Пробуем привязаться к порту 101
    if (!bind(101)) {
        throw std::runtime_error("Не удалось привязаться к порту 101");
    }

    // Подключаем сигналы
    connect(m_socket, &QUdpSocket::readyRead, this, &UDPClient::readPendingDatagrams);

    // Сигнал об ошибках сокета
    connect(m_socket, &QUdpSocket::errorOccurred,
            this, [this](QAbstractSocket::SocketError error) {
                QString errorMsg = QString("Ошибка сокета: %1 - %2")
                                       .arg(error)
                                       .arg(m_socket->errorString());
                LOG_ERROR(errorMsg);
                emit errorOccurred(errorMsg);
            });

    LOG_CAT_INFO("UDP", " сокет настроен и привязан к порту " + QString::number(m_boundPort));
}

bool UDPClient::bind(quint16 port)
{
    if (!m_socket) {
        LOG_CAT_ERROR("UDP", "::bind - сокет не создан");
        return false;
    }

    if (m_isBound) {
        LOG_CAT_INFO("UDP", "уже привязан к порту " + QString::number(m_boundPort));
        return true;
    }

    LOG_CAT_INFO("UDP","::bind - попытка привязки к порту " + QString::number(port));

    // Пробуем привязаться


    if (m_socket->bind(QHostAddress("192.168.0.246"), port)) {//ping  -t QHostAddress::AnyIPv4

        m_isBound = true;
        m_boundPort = m_socket->localPort();
        m_boundAddress = m_socket->localAddress();

        LOG_CAT_INFO("UDP", "успешно привязан к порту " + QString::number(m_boundPort) +
                 " на адресе " + m_boundAddress.toString());

        emit bindingChanged(true);
        return true;
    } else {
        QString errorMsg = QString("Не удалось привязаться к порту %1: %2")
                               .arg(port)
                               .arg(m_socket->errorString());
        LOG_CAT_ERROR("UDP", errorMsg);
        emit errorOccurred(errorMsg);
        return false;
    }
}

void UDPClient::unbind()
{
    LOG_CAT_INFO("UDP", "::unbind");

    if (m_socket && m_isBound) {
        m_socket->close();
        m_isBound = false;
        m_boundPort = 0;
        m_boundAddress.clear();

        emit bindingChanged(false);
        LOG_CAT_INFO("UDP","отвязан от порта");
    }
}

bool UDPClient::isBound() const
{
    return m_isBound && m_socket != nullptr &&
           m_socket->state() == QAbstractSocket::BoundState;
}

qint64 UDPClient::sendTo(const QByteArray& data, const QString& address, quint16 port)
{
    LOG_CAT_DEBUG("UDP", QString("::sendTo: адрес=%1, порт=%2, размер=%3 байт")
                  .arg(address).arg(port).arg(data.size()));

    if (!m_socket) {
        LOG_CAT_ERROR("UDP", "::sendTo - сокет не инициализирован");
        emit errorOccurred("Сокет не инициализирован");
        return -1;
    }

    if (!m_isBound) {
        LOG_CAT_ERROR("UDP", "::sendTo - сокет не привязан");
        emit errorOccurred("Сокет не привязан к порту");
        return -1;
    }

    QHostAddress hostAddress;
    if (!hostAddress.setAddress(address)) {
        LOG_CAT_ERROR("UDP","::sendTo - неверный адрес: " + address);
        emit errorOccurred("Неверный адрес: " + address);
        return -1;
    }

    qint64 bytesSent = m_socket->writeDatagram(data, hostAddress, port);

    if (bytesSent == -1) {
        QString errorMsg = QString("Ошибка отправки на %1:%2: %3")
                               .arg(address)
                               .arg(port)
                               .arg(m_socket->errorString());
        LOG_CAT_ERROR("UDP", errorMsg);
        emit errorOccurred(errorMsg);
    } else {
        LOG_CAT_DEBUG("UDP", QString("отправлено %1 байт на %2:%3")
                      .arg(bytesSent).arg(address).arg(port));
        emit dataSent(bytesSent);
    }

    return bytesSent;
}

qint64 UDPClient::sendBroadcast(const QByteArray& data, quint16 port)
{
    LOG_DEBUG(QString("UDPClient::sendBroadcast: порт=%1, размер=%2 байт")
                  .arg(port).arg(data.size()));

    if (!m_socket) {
        LOG_CAT_ERROR("UDP","::sendBroadcast - сокет не инициализирован");
        emit errorOccurred("Сокет не инициализирован");
        return -1;
    }

    if (!m_isBound) {
        LOG_CAT_ERROR("UDP","::sendBroadcast - сокет не привязан");
        emit errorOccurred("Сокет не привязан к порту");
        return -1;
    }

    // Включаем возможность отправки широковещательных пакетов
    bool wasBroadcastEnabled = m_socket->socketOption(QAbstractSocket::MulticastTtlOption).toBool();
    if (!wasBroadcastEnabled) {
        // На некоторых платформах нужно явно разрешить широковещание
        m_socket->setSocketOption(QAbstractSocket::MulticastTtlOption, QVariant(1));
    }

    qint64 bytesSent = m_socket->writeDatagram(data, QHostAddress::Broadcast, port);

    if (!wasBroadcastEnabled) {
        m_socket->setSocketOption(QAbstractSocket::MulticastTtlOption, QVariant(0));
    }

    if (bytesSent == -1) {
        QString errorMsg = QString("Ошибка широковещательной отправки на порт %1: %2")
                               .arg(port)
                               .arg(m_socket->errorString());
        LOG_CAT_ERROR("UDP",errorMsg);
        emit errorOccurred(errorMsg);
    } else {
        LOG_CAT_DEBUG("UDP",QString(" отправлено широковещательно %1 байт на порт %2")
                      .arg(bytesSent).arg(port));
        emit dataSent(bytesSent);
    }

    return bytesSent;
}

void UDPClient::readPendingDatagrams()
{
    if (!m_socket) {
        return;
    }

    while (m_socket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = m_socket->receiveDatagram();

        if (datagram.isValid()) {
            QByteArray data = datagram.data();
            QHostAddress sender = datagram.senderAddress();
            std::swap(data[0],data[1]);
            quint16 port = datagram.senderPort();

            LOG_CAT_DEBUG("UDP",QString("UDPClient получено %1 байт от %2:%3")
                          .arg(data.size())
                          .arg(sender.toString())
                          .arg(port));

            // Логируем первые несколько байт для отладки
            if (data.size() > 0) {
                QString hexData;
                for (int i = 0; i < qMin(data.size(), 8); ++i) {
                    hexData += QString("%1 ").arg((uint8_t)data[i], 2, 16, QChar('0')).toUpper();
                }
                if (data.size() > 8) {
                    hexData += "...";
                }
                LOG_CAT_DEBUG("UDP", QString("Данные [%1 байт]: %2").arg(data.size()).arg(hexData));
            }

            emit dataReceived(data, sender, port);
        } else {
            LOG_CAT_WARNING("UDP","получил невалидную датаграмму");
        }
    }
}
