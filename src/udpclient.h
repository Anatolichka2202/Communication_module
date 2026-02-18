
#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>

class UDPClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isBound READ isBound NOTIFY bindingChanged)

public:
    explicit UDPClient(QObject* parent = nullptr);
    ~UDPClient() override;

    Q_INVOKABLE void initializeInThread();

    bool bind(quint16 port = 101);
    void unbind();
    bool isBound() const;

    qint64 sendTo(const QByteArray& data, const QString& address, quint16 port);
    qint64 sendBroadcast(const QByteArray& data, quint16 port);

    quint16 boundPort() const { return m_boundPort; }
    QHostAddress boundAddress() const { return m_boundAddress; }

signals:
    void dataReceived(const QByteArray& data, const QHostAddress& sender, quint16 port);
    void bindingChanged(bool bound);
    void errorOccurred(const QString& error);
    void dataSent(qint64 bytes);
    void initialized();

private slots:
    void readPendingDatagrams();

private:
    void setupSocket();

private:
    QUdpSocket* m_socket;
    bool m_isBound;
    quint16 m_boundPort;
    QHostAddress m_boundAddress;
};

#endif // UDPCLIENT_H
