
#ifndef COMMANDINTERFACE_H
#define COMMANDINTERFACE_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include "../comm_dependencies.h"
#include <QVariant>
enum class PPBState;
struct DataPacket;
struct PPBResponse;
struct BridgeResponse;

class CommandInterface : public QObject
{
    Q_OBJECT
public:
    explicit CommandInterface(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~CommandInterface() = default;

    //INTERFACE
    virtual void setState(PPBState) = 0;
    virtual void startTimeoutTimer(int ms) = 0;
    virtual void stopTimeoutTimer() = 0;
    virtual void completeCurrentOperation(bool success, const QString& message="") = 0;
    virtual void sendPacket(const QByteArray& packet, const QString& description) = 0; //одиночная
    virtual void sendDataPackets(const QVector<DataPacket>& packets) = 0;              //вектор
    virtual QVector<DataPacket> getGeneratedPackets() const = 0;                       //генерация

    // Устанавливает результат парсинга (успех/ошибка + сообщение)
    virtual void setParseResult(bool success, const QString& message) = 0;

    // Устанавливает дополнительные распарсенные данные
    virtual void setParseData(const QVariant& parsedData) = 0;

    //  методы для уведомления о пакетах
    virtual void notifySentPackets(const QVector<DataPacket>& packets) = 0;
    virtual void notifyReceivedPackets(const QVector<DataPacket>& packets) = 0;
    virtual void requestClearPacketData() = 0;

signals:
    void statusDataReady(const QVector <QByteArray>& data);
    void testDataReady(const QVector<DataPacket>& data);
    void commandProgress(int current, int total);

    void parsingComplete(bool success, const QString& message, const QVariant& data = QVariant()); //сигнал о завершение парсинга
};

#endif // COMMANDINTERFACE_H
