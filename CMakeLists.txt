cmake_minimum_required(VERSION 3.16)
project(CommunicationLib VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# Опции
option(USE_REAL_DEPS "Use real dependencies from monolith" OFF)
option(BUILD_TESTING "Build test runner" ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network)

# Исходные файлы библиотеки (все .cpp из src)
set(LIB_SOURCES
    src/commandandoperation.cpp
    src/communicationengine.cpp
    src/packetbuilder.cpp
    src/udpclient.cpp
    src/protocoladapter.cpp
)

# Заголовочные файлы библиотеки (для отображения в IDE)
set(LIB_HEADERS
    src/commandandoperation.h
    src/communicationengine.h
    src/packetbuilder.h
    src/ppbprotocol.h
    src/udpclient.h
    src/commandinterface.h
    src/protocoladapter.h

)

# Публичные интерфейсы (будут установлены)
set(PUBLIC_INTERFACES
    include/icommunication.h
    include/iprotocol_adapter.h
    include/itransport.h
)

# Заглушки (для отображения и использования при USE_REAL_DEPS=OFF)
set(STUB_HEADERS
    stubs/logging_stub.h
    stubs/protocol_stub.h
    stubprotocoladapter.h
)

# Корневой заголовок для управления зависимостями
set(COMM_DEPENDENCIES
    comm_dependencies.h
)

# Собираем статическую библиотеку
add_library(${PROJECT_NAME} STATIC
    ${LIB_SOURCES}
    ${LIB_HEADERS}
    ${PUBLIC_INTERFACES}
    ${STUB_HEADERS}
    ${COMM_DEPENDENCIES}


)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if(USE_REAL_DEPS)
    # При использовании реальных зависимостей нужно указать пути к заголовкам монолита
    # Например:
    # target_include_directories(${PROJECT_NAME} PRIVATE /path/to/monolith)
    # target_compile_definitions(${PROJECT_NAME} PRIVATE USE_REAL_DEPS)
    message("Building with real dependencies (requires proper include paths)")
    # Пока просто добавляем define, чтобы в коде понимали
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_REAL_DEPS)
else()
    # Режим заглушек: подключаем папку stubs
    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stubs)
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_STUBS) # опционально
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Network)

# Группировка файлов в IDE (опционально)
source_group("Source Files" FILES ${LIB_SOURCES})
source_group("Header Files\\Src" FILES ${LIB_HEADERS})
source_group("Header Files\\Include" FILES ${PUBLIC_INTERFACES})
source_group("Stubs" FILES ${STUB_HEADERS})
source_group("Config" FILES ${COMM_DEPENDENCIES})

# Тестовый драйвер
if(BUILD_TESTING)
    add_executable(test_runner tests/test_runner.cpp)
    target_link_libraries(test_runner PRIVATE ${PROJECT_NAME} Qt6::Core)
    target_include_directories(test_runner PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    # Тестовая программа также может использовать заглушки или реальные зависимости
    if(NOT USE_REAL_DEPS)
        target_include_directories(test_runner PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stubs)
    endif()
endif()
