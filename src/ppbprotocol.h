#ifndef PPBPROTOCOL_H
#define PPBPROTOCOL_H

#pragma once

#include <cstdint>

// Состояния машины (используются в движке)
enum class PPBState {
    Idle,               // Нет подключения
    Ready,              // Связь есть, можно отправлять команды
    SendingCommand      // Отправили команду, ждём ответ
};

// ===== ОСНОВНЫЕ КОНСТАНТЫ =====
constexpr uint16_t BROADCAST_ADDRESS = 0xFFFF;

// ===== КОМАНДЫ ТУ ===
enum class TechCommand : uint8_t {
    TS = 0x00,          // выдать тех.состояние
    TC = 0x01,          // принять тех.управление
    VERS = 0x02,        // выдать версию
    VOLUME = 0x03,      // принять том исполняемого ПО
    CHECKSUM = 0x04,    // выдать контрольную сумму
    PROGRAMM = 0x05,    // обновить исп файл ПО
    CLEAN = 0x0A,       // очистить временный файл ПО
    PRBS_M2S = 0x0B,    // принять тестовую последовательность данных
    BER_T = 0x0C,       // выдать коэф ошибок ТУ
    BER_F = 0x0D,       // выдать коэф ошибок ФУ
    PRBS_S2M = 0x0E,    // выдать тестовую последовательность
    DROP = 0x0F         // выдать кол-во отброшенных пакетов ФУ по контр сумме
};

// ===== ФУ КОМАНДЫ =====
enum class FuCommand : uint8_t {
    TRANSMIT  = 0,
    RECEIVE   = 1
};

// ===== ПРИЗНАК ПАКЕТА =====
enum class Sign : uint8_t {
    TU = 0,
    FU = 1
};

// ===== БАЗОВЫЙ ЗАПРОС (8 байт) =====
#pragma pack(push, 1)
struct BaseRequest {
    uint16_t address;     // 2 байта - адрес ППБ (0-15)
    uint8_t command;      // 1 байт - команда (TechCommand)
    uint8_t sign;         // 1 байт - признак TU/FU (Sign)
    uint8_t fu_period;    // 1 байт - период ФУ (0 для команды 0)
    uint8_t fu_data[3];   // 3 байта - данные ФУ
};
#pragma pack(pop)

// ===== ЗАГОЛОВОК ОТВЕТА ОТ ППБ ДЛЯ ТУ КОМАНД (первые 3 байта ответа) =====
#pragma pack(push, 1)
struct TUResponseHeader {
    uint16_t address;     // 2 байта - адрес ППБ
    uint8_t status;       // 1 байт - статус (0x00 - успех, 0x01 - ошибка)
};
#pragma pack(pop)

// ===== ОТВЕТ ОТ БРИДЖА ДЛЯ ФУ КОМАНД (без CRC) =====
// Остаётся без изменений для обратной совместимости.
#pragma pack(push, 1)
struct BridgeResponse {
    uint16_t address;     // 2 байта - адрес ППБ
    uint8_t command;      // 1 байт - команда
    uint8_t status;       // 1 байт - статус (0 - ошибка, 1 - ОК)
};
#pragma pack(pop)

// ===== ПАКЕТ ДАННЫХ (внутреннее представление, для анализа) =====
// Используется для хранения логических пакетов (например, тестовых последовательностей).
#pragma pack(push, 1)
struct DataPacket {
    uint8_t data[2];      // 2 байта данных
    uint8_t counter;      // 1 байт номера пакета (от 0)
    // CRC больше нет
};
#pragma pack(pop)

// ===== СТАТУС ППБ (для внутреннего использования) =====
struct PPBStatus {
    uint8_t address;
    bool powerOk;
    bool isTransmitMode;
    bool isReceiveMode;

    struct Channel {
        float power;        // Мощность, Вт
        float temperature;  // Температура, °C
        float vswr;         // КСВН
        bool isOk;
    } channel1, channel2;

    uint32_t pulseDuration;  // Длительность импульса, мкс
    uint8_t dutyCycle;       // Скважность
    uint32_t pulseDelay;     // Задержка импульса, мкс

    bool hasErrors;
    bool droppedPackets;
};

#endif // PPBPROTOCOL_H

 /*
 * при ту командах мы общаемся с ппб через переходник RS-Ethernet. Он не отбрасывет CRC из пакеты. Нам же не нужно проверять, тк мы получаем UDP пакет со своим СРС.
 * НО нам нужно отправлять пакеты, которые содержат ДАННЫЕ, с СРС.
 *
 * при ФУ командах есть бридж. поэтому срс здесь не высчитываем, бридж сам сделает это.
 */
