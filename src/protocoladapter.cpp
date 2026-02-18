#include "protocoladapter.h"
#include "../comm_dependencies.h"  // для логирования (если нужно)
#include <cstring>

bool ProtocolAdapter::parseIncomingPacket(const QByteArray& data, ProtocolEvent& event) {
    // Сбрасываем событие
    event = ProtocolEvent();
    event.type = ProtocolEvent::Unknown;

    // Проверяем, является ли пакет ответом ППБ (TU)
    // Минимальный размер ответа ППБ - заголовок TUResponseHeader (3 байта)
    if (data.size() >= static_cast<int>(sizeof(TUResponseHeader))) {
        TUResponseHeader header;
        std::memcpy(&header, data.constData(), sizeof(header));

        // Проверяем, что адрес не нулевой (это признак валидного ответа)
        if (header.address != 0) {
            event.address = header.address;
            event.status = header.status;

            if (header.status == 0) {
                // Успешный ответ
                if (data.size() > static_cast<int>(sizeof(header))) {
                    // Есть данные
                    event.type = ProtocolEvent::Data;
                    event.payload = data.mid(sizeof(header));
                } else {
                    event.type = ProtocolEvent::Ok;
                }
            } else {
                // Ошибка
                event.type = ProtocolEvent::Error;
            }
            return true;
        }
    }

    // Проверяем, является ли пакет ответом бриджа (FU)
    if (data.size() == static_cast<int>(sizeof(BridgeResponse))) {
        BridgeResponse response;
        std::memcpy(&response, data.constData(), sizeof(response));

        event.address = response.address;
        event.status = response.status;  // 1 - OK, 0 - ошибка
        event.type = ProtocolEvent::BridgeResponse;
        // payload не заполняем, так как у бриджа нет дополнительных данных
        return true;
    }

    // Неизвестный пакет
    return false;
}

QByteArray ProtocolAdapter::buildRequest(uint16_t address, TechCommand cmd, const QByteArray& payload) {
    if (payload.isEmpty()) {
        return PacketBuilder::createTURequest(address, cmd);
    } else {
        return PacketBuilder::createTURequestWithData(address, cmd, payload);
    }
}

QByteArray ProtocolAdapter::buildFURequest(uint16_t address, uint8_t period, const uint8_t fuData[3]) {
    return PacketBuilder::createFURequest(address, period, fuData);
}

QVector<DataPacket> ProtocolAdapter::extractDataPackets(const QByteArray& payload) {
    return PacketBuilder::extractDataPackets(payload);
}
