#ifndef COMM_DEPENDENCIES_H
#define COMM_DEPENDENCIES_H

#ifdef USE_REAL_DEPS
// Реальные зависимости (пути будут добавлены в CMake)
#include "core/logging/logging_unified.h"
#include "core/communication/ppbprotocol.h"
#else
// Заглушки
#include "logging_stub.h"
#include "protocol_stub.h"
#endif

#endif // COMM_DEPENDENCIES_H
