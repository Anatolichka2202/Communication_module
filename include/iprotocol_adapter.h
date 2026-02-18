#ifndef IPROTOCOL_ADAPTER_H
#define IPROTOCOL_ADAPTER_H

#include <QByteArray>
#include <QVector>
#include <cstdint>

#include "../comm_dependencies.h"  // для TechCommand, DataPacket и т.д.

struct ProtocolEvent {
    enum Type {
        Unknown,
        Ok,               // ППБ ответил OK без данных
        Error,            // ППБ ответил ошибкой (status != 0)
        Data,             // ППБ ответил OK и есть данные в payload
        BridgeResponse    // Ответ от бриджа (для ФУ команд)
    } type = Unknown;

    uint16_t address = 0;
    uint8_t status = 0;   // Код статуса (для Error – код ошибки, для BridgeResponse – 1/0)
    QByteArray payload;   // Сырые данные (для Data и возможно BridgeResponse)
};

class IProtocolAdapter {
public:
    virtual ~IProtocolAdapter() = default;

    // Парсинг входящего UDP-пакета. Возвращает true, если пакет распознан как ответ ППБ или бриджа,
    // и заполняет структуру события.
    virtual bool parseIncomingPacket(const QByteArray& data, ProtocolEvent& event) = 0;

    // Построение запроса для TU команды
    virtual QByteArray buildRequest(uint16_t address, TechCommand cmd, const QByteArray& payload = {}) = 0;

    // Построение запроса для FU команды
    virtual QByteArray buildFURequest(uint16_t address, uint8_t period, const uint8_t fuData[3] = nullptr) = 0;

    // Разбор payload на массив DataPacket (если нужно для тестовых последовательностей)
    virtual QVector<DataPacket> extractDataPackets(const QByteArray& payload) = 0;
};

#endif // IPROTOCOL_ADAPTER_H
