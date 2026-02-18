#include "commandandoperation.h"

#include "../comm_dependencies.h"
#include <QDataStream>
#include <QtEndian>

// Вспомогательная функция для преобразования QByteArray в uint32_t
static uint32_t bytesToUInt32LE(const QByteArray& data) {
    if (data.size() < 4) return 0;
    uint32_t result;
    memcpy(&result, data.constData(), 4);
    return qFromLittleEndian(result); // или qFromBigEndian - зависит от протокола
}

// Функция для парсинга 2 пакетов в uint32_t (для команд, где 2 пакета = 4 байта данных)
static uint32_t parseTwoPackets(const QVector<QByteArray>& data) {
    if (data.size() < 2) return 0;

    // Извлекаем данные из DataPacket (первые 2 байта - данные, остальное - служебные)
    QByteArray combined;
    for (const QByteArray& packet : data) {
        if (packet.size() >= 2) {
            combined.append(packet.left(2)); // Берем только data[0] и data[1]
        }
    }

    return bytesToUInt32LE(combined);
}

// Реализация метода create
std::unique_ptr<PPBCommand> CommandFactory::create(TechCommand cmd) {
    switch (cmd) {
    case TechCommand::TS: return std::make_unique<StatusCommand>();
    case TechCommand::TC: return std::make_unique<ResetCommand>();
    case TechCommand::VERS: return std::make_unique<VersCommand>();
    case TechCommand::VOLUME: return std::make_unique<VolumeCommand>();
    case TechCommand::CHECKSUM: return std::make_unique<CheckSumCommand>();
    case TechCommand::PROGRAMM: return std::make_unique<ProgrammCommand>();
    case TechCommand::CLEAN: return std::make_unique<CleanCommand>();
    case TechCommand::DROP: return std::make_unique<DROPCommand>();
    case TechCommand::PRBS_M2S: return std::make_unique<PRBS_M2SCommand>();
    case TechCommand::PRBS_S2M: return std::make_unique<PRBS_S2MCommand>();
    case TechCommand::BER_T: return std::make_unique<BER_TCommand>();
    case TechCommand::BER_F: return std::make_unique<BER_FCommand>();
    default: return nullptr;
    }
}

QString CommandFactory::commandName(TechCommand cmd) {
    static const QMap<TechCommand, QString> names = {
                                                     {TechCommand::TS, "Опрос состояния"},
                                                     {TechCommand::TC, "Сброс"},
                                                     {TechCommand::VERS, "Запрос версии"},
                                                     {TechCommand::VOLUME, "Принять том исполняемого ПО"},
                                                     {TechCommand::CHECKSUM, "Выдать контрольную сумму"},
                                                     {TechCommand::PROGRAMM, "Обновить исполняемый файл ПО"},
                                                     {TechCommand::CLEAN, "Очистить временный файл ПО"},
                                                     {TechCommand::DROP, "Отброшенные пакеты ФУ"},
                                                     {TechCommand::PRBS_M2S, "Принять тестовую последовательность данных"},
                                                     {TechCommand::PRBS_S2M, "Выдать тестовую последовательность"},
                                                     {TechCommand::BER_T, "Коэффициент ошибок линии ТУ"},
                                                     {TechCommand::BER_F, "Коэффициент ошибок линии ФУ"},
                                                     };
    return names.value(cmd, "Неизвестная команда");
}

// TS

bool StatusCommand::parseResponseData(const QVector<QByteArray>& data,
                                      QString& outMessage,
                                      QVariant& outParsedData) const {
    outMessage = "Статус получен";

    // Сохраняем сырые данные для дальнейшего парсинга в UI
    QVariantList packetsList;
    for (const auto& packet : data) {
        packetsList.append(packet);
    }
    outParsedData = packetsList;

    return true;
}

void StatusCommand::onDataReceived(CommandInterface* comm, const QVector<QByteArray>& data) const {
    if (!comm) {
        LOG_CAT_WARNING("Command","StatusCommand::onDataReceived: comm is nullptr!");
        return;
    }

    // Сначала парсим данные
    QString message;
    QVariant parsedData;
    if (parseResponseData(data, message, parsedData)) {
        // Устанавливаем результат парсинга
        comm->setParseResult(true, message);
        comm->setParseData(parsedData);

        // Отправляем сырые данные для UI
        emit comm->statusDataReady(data);

        // Команда TS успешно выполнена только после получения всех пакетов
        // Завершение операции будет вызвано в communicationengine::completeOperation()
    } else {
        comm->setParseResult(false, "Ошибка парсинга статуса");
    }
}
// VERS
bool VersCommand::parseResponseData(const QVector<QByteArray>& data,
                                    QString& outMessage,
                                    QVariant& outParsedData) const {
    if (data.size() < 2) {
        outMessage = "Ошибка: получено меньше 2 пакетов";
        return false;
    }

    uint32_t crc = parseTwoPackets(data);
    outMessage = QString("Версия ПО. CRC32: 0x%1").arg(crc, 8, 16, QChar('0'));

    QVariantMap extraData;
    extraData["crc32"] = crc;
    extraData["rawPackets"] = data.size();
    outParsedData = extraData;

    return true;
}

void VersCommand::onDataReceived(CommandInterface* comm, const QVector<QByteArray>& data) const {
    if (!comm) {
        LOG_CAT_WARNING("Command","VersCommand::onDataReceived: comm is nullptr!");
        return;
    }

    // 1. Проверяем количество пакетов
    if (data.size() != expectedResponsePackets()) {
        // Частичные данные - устанавливаем соответствующий результат
        QString message = QString("Получено %1 из %2 пакетов")
                              .arg(data.size())
                              .arg(expectedResponsePackets());

        // Можно сохранить то, что получили для анализа
        QVariantMap extraData;
        extraData["received"] = data.size();
        extraData["expected"] = expectedResponsePackets();
        extraData["partial"] = true;

        comm->setParseResult(false, message);
        comm->setParseData(extraData);
        return;
    }

    // Парсим данные
    QString message;
    QVariant parsedData;
    if (parseResponseData(data, message, parsedData)) {
        comm->setParseResult(true, message);
        comm->setParseData(parsedData);
    } else {
        comm->setParseResult(false, message);
    }

    // Отправляем сырые данные для UI (если нужно)
    emit comm->statusDataReady(data);
}

// CHECKSUM
bool CheckSumCommand::parseResponseData(const QVector<QByteArray>& data,
                                        QString& outMessage,
                                        QVariant& outParsedData) const {
    if (data.size() < 2) {
        outMessage = "Ошибка: получено меньше 2 пакетов";
        return false;
    }

    uint32_t checksum = parseTwoPackets(data);
    outMessage = QString("Контрольная сумма VOLUME: 0x%1").arg(checksum, 8, 16, QChar('0'));

    QVariantMap extraData;
    extraData["checksum"] = checksum;
    outParsedData = extraData;

    return true;
}

void CheckSumCommand::onDataReceived(CommandInterface* comm, const QVector<QByteArray>& data) const {
    if (!comm) {
        LOG_CAT_WARNING("Command","CheckSumCommand::onDataReceived: comm is nullptr!");
        return;
    }

    QString message;
    QVariant parsedData;
    if (parseResponseData(data, message, parsedData)) {
        comm->setParseResult(true, message);
        comm->setParseData(parsedData);
    } else {
        comm->setParseResult(false, message);
    }
}

// DROP
bool DROPCommand::parseResponseData(const QVector<QByteArray>& data,
                                    QString& outMessage,
                                    QVariant& outParsedData) const {
    if (data.size() < 2) {
        outMessage = "Ошибка: получено меньше 2 пакетов";
        return false;
    }

    uint32_t dropped = parseTwoPackets(data);
    outMessage = QString("Отброшенных пакетов ФУ: %1").arg(dropped);

    QVariantMap extraData;
    extraData["dropped"] = dropped;
    outParsedData = extraData;

    return true;
}

void DROPCommand::onDataReceived(CommandInterface* comm, const QVector<QByteArray>& data) const {
    if (!comm) {
        LOG_CAT_WARNING("Command","DROPCommand::onDataReceived: comm is nullptr!");
        return;
    }

    QString message;
    QVariant parsedData;
    if (parseResponseData(data, message, parsedData)) {
        comm->setParseResult(true, message);
        comm->setParseData(parsedData);
    } else {
        comm->setParseResult(false, message);
    }
}

// ===== BER_TCommand =====
bool BER_TCommand::parseResponseData(const QVector<QByteArray>& data,
                                     QString& outMessage,
                                     QVariant& outParsedData) const {
    if (data.size() < 2) {
        outMessage = "Ошибка: получено меньше 2 пакетов";
        return false;
    }

    uint32_t errors = parseTwoPackets(data);
    float ber = errors / 1000000.0f;
    outMessage = QString("Коэффициент ошибок ТУ: %1 (ошибок: %2)").arg(ber, 0, 'f', 6).arg(errors);

    QVariantMap extraData;
    extraData["errors"] = errors;
    extraData["ber"] = ber;
    outParsedData = extraData;

    return true;
}

void BER_TCommand::onDataReceived(CommandInterface* comm, const QVector<QByteArray>& data) const {
    if (!comm) {
        LOG_CAT_WARNING("Command","BER_TCommand::onDataReceived: comm is nullptr!");
        return;
    }

    QString message;
    QVariant parsedData;
    if (parseResponseData(data, message, parsedData)) {
        comm->setParseResult(true, message);
        comm->setParseData(parsedData);
    } else {
        comm->setParseResult(false, message);
    }
}

// ===== BER_FCommand =====
bool BER_FCommand::parseResponseData(const QVector<QByteArray>& data,
                                     QString& outMessage,
                                     QVariant& outParsedData) const {
    if (data.size() < 2) {
        outMessage = "Ошибка: получено меньше 2 пакетов";
        return false;
    }

    uint32_t errors = parseTwoPackets(data);
    float ber = errors / 1000000.0f;
    outMessage = QString("Коэффициент ошибок ФУ: %1 (ошибок: %2)").arg(ber, 0, 'f', 6).arg(errors);

    QVariantMap extraData;
    extraData["errors"] = errors;
    extraData["ber"] = ber;
    outParsedData = extraData;

    return true;
}

void BER_FCommand::onDataReceived(CommandInterface* comm, const QVector<QByteArray>& data) const {
    if (!comm) {
        LOG_CAT_WARNING("Command","BER_FCommand::onDataReceived: comm is nullptr!");
        return;
    }

    QString message;
    QVariant parsedData;
    if (parseResponseData(data, message, parsedData)) {
        comm->setParseResult(true, message);
        comm->setParseData(parsedData);
    } else {
        comm->setParseResult(false, message);
    }
}

// PRBS_M2S
void PRBS_M2SCommand::onOkReceived(CommandInterface* comm, uint16_t address) const {
    if (!comm) {
        LOG_CAT_WARNING("Command","StatusCommand::onDataReceived: comm is nullptr!");
        return;
    }
    // Генерируем тестовые пакеты прямо здесь
    QVector<DataPacket> testPackets;

    // Генерация 256 пакетов по протоколу
    uint8_t lfsr = 0x01; // Начальное значение

    for (int i = 0; i < PPBConstants::TEST_PACKET_COUNT; ++i) {
        DataPacket packet;
        packet.data[0] = lfsr;
        packet.data[1] = lfsr ^ 0x55; // XOR для разнообразия
        packet.counter = i; // Номер пакета

        testPackets.append(packet);

        // LFSR сдвиг
        lfsr = (lfsr >> 1) | ((lfsr ^ (lfsr >> 1)) << 7);
    }
    // Уведомляем о новых отправленных пакетах
    comm->notifySentPackets(testPackets);

    // Отправляем пакеты
    comm->sendDataPackets(testPackets);


}

// ===== PRBS_S2MCommand =====
bool PRBS_S2MCommand::parseResponseData(const QVector<QByteArray>& data,
                                        QString& outMessage,
                                        QVariant& outParsedData) const
{
    // Простая проверка количества
    if (data.size() != expectedResponsePackets()) {
        outMessage = QString("Неверное количество пакетов: %1 (ожидалось %2)")
                         .arg(data.size())
                         .arg(expectedResponsePackets());
        return false;
    }

    outMessage = QString("Получено %1 пакетов тестовой последовательности").arg(data.size());

    QVariantMap extraData;
    extraData["packetCount"] = data.size();
    outParsedData = extraData;

    return true;
}

void PRBS_S2MCommand::onDataReceived(CommandInterface* comm, const QVector<QByteArray>& data) const
{
    if (!comm) {
        LOG_CAT_WARNING("Command","PRBS_S2MCommand::onDataReceived: comm is nullptr!");
        return;
    }

    // 1. Парсим полученные пакеты
    QVector<DataPacket> receivedPackets;
    int parseErrors = 0;

    for (const auto& packetData : data) {
        DataPacket packet;
        if (PacketBuilder::parseDataPacket(packetData, packet)) {
            receivedPackets.append(packet);
        } else {
            parseErrors++;
            LOG_CAT_WARNING("Command",QString("Ошибка парсинга пакета %1").arg(receivedPackets.size() + parseErrors));
        }
    }

    // Уведомляем о полученных пакетах
    comm->notifyReceivedPackets(receivedPackets);

    // 2. Проверяем, получили ли мы все пакеты
    if (receivedPackets.size() != PPBConstants::TEST_PACKET_COUNT) {
        QString error = QString("Получено %1 из %2 пакетов (ошибок парсинга: %3)")
                            .arg(receivedPackets.size())
                            .arg(PPBConstants::TEST_PACKET_COUNT)
                            .arg(parseErrors);
        comm->setParseResult(false, error);
        return;
    }

    // 3. Формируем сообщение об успехе
    QString message = QString("Получено %1 тестовых пакетов").arg(receivedPackets.size());

    // 4. Сохраняем полученные пакеты для последующего сравнения
    QVariantMap extraData;

    // Сохраняем количество пакетов
    extraData["packetCount"] = receivedPackets.size();
    extraData["parseErrors"] = parseErrors;

    // Если нужно сохранить сами пакеты, можно сделать так:
    // extraData["receivedPackets"] = QVariant::fromValue(receivedPackets);
    // Но для этого нужно зарегистрировать тип DataPacket как метатип

    // 5. Устанавливаем результат парсинга
    comm->setParseResult(true, message);
    comm->setParseData(extraData);

    // 6. Дополнительно: эмитим сигнал с полученными пакетами
    // emit comm->testDataReady(receivedPackets); - если нужно для UI
}

// VOLUME
void VolumeCommand::onOkReceived(CommandInterface* comm, uint16_t address) const {
    if (!comm) {
        LOG_CAT_WARNING("Command","VolumeCommand::onOkReceived: comm is nullptr!");
        return;
    }

    // После получения OK нужно начать отправку ПО
    // В реальности здесь нужно загрузить файл ПО и разбить н
    QVector<DataPacket> programPackets;

    // TODO: Загрузить реальный файл ПО
    // Пока заглушка - генерируем тестовые данные
    for (int i = 0; i < 256; ++i) {
        DataPacket packet;
        packet.data[0] = i & 0xFF;
        packet.data[1] = (i >> 8) & 0xFF;
        packet.counter = i;

        // CRC


        programPackets.append(packet);
    }

    // Отправляем пакеты ПО
    comm->sendDataPackets(programPackets);
    //comm->completeCurrentOperation(true, "Начата отправка ПО (256 пакетов)");
}

