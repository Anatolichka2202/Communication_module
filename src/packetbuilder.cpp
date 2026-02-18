#include "packetbuilder.h"
#include <QDebug>
#include <cstring>

QByteArray PacketBuilder::createTURequest(uint16_t address, TechCommand command)
{
    return createBaseRequest(address, static_cast<uint8_t>(command), Sign::TU, 0, nullptr);
}

QByteArray PacketBuilder::createTURequestWithData(uint16_t address, TechCommand command,
                                                  const QByteArray& payload)
{
    QByteArray request = createBaseRequest(address, static_cast<uint8_t>(command),
                                           Sign::TU, 0, nullptr);
    if (!payload.isEmpty()) {
        request.append(payload);
    }
    return request;
}

QByteArray PacketBuilder::createFURequest(uint16_t address, uint8_t period,
                                          const uint8_t fuData[3])
{
    return createBaseRequest(address, 0, Sign::FU, period, fuData);
}

QByteArray PacketBuilder::createStatusRequest(uint16_t address)
{
    return createTURequest(address, TechCommand::TS);
}

QByteArray PacketBuilder::createResetRequest(uint16_t address)
{
    return createTURequest(address, TechCommand::TC);
}

QByteArray PacketBuilder::createPRBSTransmitRequest(uint16_t address,
                                                    const QVector<DataPacket>& testPackets)
{
    QByteArray payload = buildPayloadFromPackets(testPackets);
    return createTURequestWithData(address, TechCommand::PRBS_M2S, payload);
}

QByteArray PacketBuilder::createPRBSReceiveRequest(uint16_t address)
{
    return createTURequest(address, TechCommand::PRBS_S2M);
}

QByteArray PacketBuilder::createDroppedPacketsRequest(uint16_t address)
{
    return createTURequest(address, TechCommand::DROP);
}

QByteArray PacketBuilder::createFUTransmitRequest(uint16_t address)
{
    return createFURequest(address, 0, nullptr);
}

QByteArray PacketBuilder::createFUReceiveRequest(uint16_t address, uint8_t period,
                                                 const uint8_t fuData[3])
{
    return createFURequest(address, period, fuData);
}

bool PacketBuilder::parseTUResponse(const QByteArray& data,
                                    TUResponseHeader& header,
                                    QByteArray& payload)
{
    if (data.size() < static_cast<int>(sizeof(TUResponseHeader))) {
        qDebug() << "parseTUResponse: недостаточно данных, размер" << data.size();
        return false;
    }

    // Копируем заголовок
    memcpy(&header, data.constData(), sizeof(TUResponseHeader));

    // Остаток – данные
    if (data.size() > static_cast<int>(sizeof(TUResponseHeader))) {
        payload = data.mid(sizeof(TUResponseHeader));
    } else {
        payload.clear();
    }

    qDebug() << "TU ответ: адрес=0x" << QString::number(header.address, 16).right(4).toUpper()
             << ", статус=" << (int)header.status
             << ", размер данных=" << payload.size();

    return true;
}

bool PacketBuilder::parseBridgeResponse(const QByteArray& data, BridgeResponse& response)
{
    if (data.size() != static_cast<int>(sizeof(BridgeResponse))) {
        qDebug() << "parseBridgeResponse: неверный размер, ожидалось"
                 << sizeof(BridgeResponse) << ", получено" << data.size();
        return false;
    }

    memcpy(&response, data.constData(), sizeof(BridgeResponse));
    qDebug() << "Bridge ответ: адрес=" << response.address
             << ", команда=0x" << QString::number(response.command, 16)
             << ", статус=" << (int)response.status;
    return true;
}

QVector<DataPacket> PacketBuilder::extractDataPackets(const QByteArray& payload)
{
    QVector<DataPacket> packets;
    const size_t packetSize = sizeof(DataPacket); // 3 байта

    if (payload.size() % static_cast<int>(packetSize) != 0) {
        qDebug() << "extractDataPackets: размер payload не кратен" << packetSize;
    }

    int count = payload.size() / static_cast<int>(packetSize);
    packets.reserve(count);

    for (int i = 0; i < count; ++i) {
        DataPacket pkt;
        memcpy(&pkt, payload.constData() + i * packetSize, packetSize);
        packets.append(pkt);
    }

    return packets;
}

QByteArray PacketBuilder::buildPayloadFromPackets(const QVector<DataPacket>& packets)
{
    QByteArray payload;
    payload.reserve(packets.size() * static_cast<int>(sizeof(DataPacket)));

    for (const DataPacket& pkt : packets) {
        payload.append(reinterpret_cast<const char*>(&pkt), sizeof(DataPacket));
    }

    return payload;
}

// Приватный метод: формирует только BaseRequest
QByteArray PacketBuilder::createBaseRequest(uint16_t address, uint8_t command,
                                            Sign sign, uint8_t period,
                                            const uint8_t fuData[3])
{
    BaseRequest req;
    req.address = address;
    req.command = command;
    req.sign = static_cast<uint8_t>(sign);
    req.fu_period = period;

    if (fuData) {
        memcpy(req.fu_data, fuData, 3);
    } else {
        memset(req.fu_data, 0, 3);
    }

    return QByteArray(reinterpret_cast<const char*>(&req), sizeof(req));
}
