#include <QCoreApplication>
#include <QDebug>
#include "../src/protocoladapter.h"
#include "../comm_dependencies.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    ProtocolAdapter adapter;

    // Тест 1: парсинг успешного ответа с данными
    {
        TUResponseHeader header;
        header.address = 0x1234;
        header.status = 0;
        QByteArray payload(4, 0xAA);
        QByteArray packet(reinterpret_cast<const char*>(&header), sizeof(header));
        packet.append(payload);

        ProtocolEvent event;
        if (adapter.parseIncomingPacket(packet, event)) {
            qDebug() << "Test 1 passed, type=" << event.type
                     << "addr=" << event.address
                     << "status=" << event.status
                     << "payload size=" << event.payload.size();
        } else {
            qDebug() << "Test 1 failed";
        }
    }

    // Тест 2: парсинг ошибки
    {
        TUResponseHeader header;
        header.address = 0x1234;
        header.status = 0x01; // ошибка
        QByteArray packet(reinterpret_cast<const char*>(&header), sizeof(header));

        ProtocolEvent event;
        if (adapter.parseIncomingPacket(packet, event)) {
            qDebug() << "Test 2 passed, type=" << event.type;
        } else {
            qDebug() << "Test 2 failed";
        }
    }

    // Тест 3: парсинг ответа бриджа
    {
        BridgeResponse response;
        response.address = 0x5678;
        response.command = 0;
        response.status = 1;
        QByteArray packet(reinterpret_cast<const char*>(&response), sizeof(response));

        ProtocolEvent event;
        if (adapter.parseIncomingPacket(packet, event)) {
            qDebug() << "Test 3 passed, type=" << event.type
                     << "addr=" << event.address
                     << "status=" << event.status;
        } else {
            qDebug() << "Test 3 failed";
        }
    }

    return 0;
}
