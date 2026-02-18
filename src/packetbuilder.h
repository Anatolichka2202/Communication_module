#ifndef PACKETBUILDER_H
#define PACKETBUILDER_H

#include "../comm_dependencies.h"
#include <QByteArray>
#include <QVector>

class PacketBuilder
{
public:
    // Преобразование индекса ППБ (1-16) в битовый адрес
    static uint16_t indexToAddress(int ppbNumber) {
        if (ppbNumber < 1 || ppbNumber > 16)
            return BROADCAST_ADDRESS;
        return (1 << (ppbNumber - 1));
    }

    // Проверка, является ли адрес валидным для одного ППБ
    static bool isSinglePPBAddress(uint16_t address) {
        return address && !(address & (address - 1));
    }

    // === ФОРМИРОВАНИЕ ЗАПРОСОВ ===

    // Создать базовый TU-запрос (без дополнительных данных)
    static QByteArray createTURequest(uint16_t address, TechCommand command);

    // Создать TU-запрос с дополнительными данными (payload)
    // payload будет добавлен сразу после BaseRequest
    static QByteArray createTURequestWithData(uint16_t address, TechCommand command,
                                              const QByteArray& payload = QByteArray());

    // Создать базовый FU-запрос (период и данные ФУ)
    static QByteArray createFURequest(uint16_t address, uint8_t period,
                                      const uint8_t fuData[3] = nullptr);

    // === КОНКРЕТНЫЕ КОМАНДЫ ТУ ===

    // Команда TS (опрос состояния)
    static QByteArray createStatusRequest(uint16_t address);

    // Команда сброса (TC)
    static QByteArray createResetRequest(uint16_t address);

    // Запуск тестовой последовательности ПЕРЕДАЧИ (PRBS_M2S)
    static QByteArray createPRBSTransmitRequest(uint16_t address,
                                                const QVector<DataPacket>& testPackets);

    // Запуск тестовой последовательности ПРИЕМА (PRBS_S2M)
    static QByteArray createPRBSReceiveRequest(uint16_t address);

    // Запрос отброшенных пакетов ФУ (DROP)
    static QByteArray createDroppedPacketsRequest(uint16_t address);

    // === ФУ КОМАНДЫ ===

    // ФУ передача (команда 0, период 0)
    static QByteArray createFUTransmitRequest(uint16_t address);

    // ФУ прием с периодом и опциональными данными
    static QByteArray createFUReceiveRequest(uint16_t address, uint8_t period,
                                             const uint8_t fuData[3] = nullptr);

    // === ПАРСИНГ ОТВЕТОВ ===

    // Парсинг ответа на ТУ команду (заголовок + данные)
    // Возвращает true, если размер >= sizeof(TUResponseHeader)
    // header – заполненный заголовок, payload – оставшиеся данные
    static bool parseTUResponse(const QByteArray& data,
                                TUResponseHeader& header,
                                QByteArray& payload);

    // Парсинг ответа от бриджа (ФУ команды) – без изменений
    static bool parseBridgeResponse(const QByteArray& data, BridgeResponse& response);

    // === РАБОТА С ДАННЫМИ (для анализа) ===

    // Извлечь список DataPacket из payload (предполагается, что payload
    // содержит последовательность трёхбайтовых блоков: data[0], data[1], counter)
    static QVector<DataPacket> extractDataPackets(const QByteArray& payload);

    // Создать payload из списка DataPacket (для отправки)
    static QByteArray buildPayloadFromPackets(const QVector<DataPacket>& packets);

    // === УТИЛИТЫ ===

    static constexpr size_t baseRequestSize() { return sizeof(BaseRequest); }
    static constexpr size_t tuResponseHeaderSize() { return sizeof(TUResponseHeader); }
    static constexpr size_t bridgeResponseSize() { return sizeof(BridgeResponse); }
    static constexpr size_t dataPacketSize() { return sizeof(DataPacket); }

    // Проверка широковещательного адреса
    static bool isBroadcastAddress(uint16_t address) {
        return address == BROADCAST_ADDRESS;
    }

private:
    // Внутренний метод для формирования запроса (без payload)
    static QByteArray createBaseRequest(uint16_t address, uint8_t command,
                                        Sign sign, uint8_t period = 0,
                                        const uint8_t fuData[3] = nullptr);
};

#endif // PACKETBUILDER_H
