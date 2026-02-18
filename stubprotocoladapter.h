// stubprotocoladapter.h
#ifndef STUBPROTOCOLADAPTER_H
#define STUBPROTOCOLADAPTER_H

#include "iprotocol_adapter.h"

class StubProtocolAdapter : public IProtocolAdapter {
public:
    bool parseIncomingPacket(const QByteArray& data, ProtocolEvent& event) override {
        // Заглушка: всегда возвращаем Unknown
        event.type = ProtocolEvent::Unknown;
        return false;
    }

    QByteArray buildRequest(uint16_t address, TechCommand cmd, const QByteArray& payload = {}) override {
        return QByteArray();
    }

    QByteArray buildFURequest(uint16_t address, uint8_t period, const uint8_t fuData[3] = nullptr) override {
        return QByteArray();
    }

    QVector<DataPacket> extractDataPackets(const QByteArray& payload) override {
        return QVector<DataPacket>();
    }
};

#endif // STUBPROTOCOLADAPTER_H
