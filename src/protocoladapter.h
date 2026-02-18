#ifndef PROTOCOLADAPTER_H
#define PROTOCOLADAPTER_H

#include "iprotocol_adapter.h"
#include "packetbuilder.h"
#include "../comm_dependencies.h"

class ProtocolAdapter : public IProtocolAdapter {
public:
    ProtocolAdapter() = default;
    ~ProtocolAdapter() override = default;

    // IProtocolAdapter interface
    bool parseIncomingPacket(const QByteArray& data, ProtocolEvent& event) override;
    QByteArray buildRequest(uint16_t address, TechCommand cmd, const QByteArray& payload = {}) override;
    QByteArray buildFURequest(uint16_t address, uint8_t period, const uint8_t fuData[3] = nullptr) override;
    QVector<DataPacket> extractDataPackets(const QByteArray& payload) override;
};

#endif // PROTOCOLADAPTER_H
