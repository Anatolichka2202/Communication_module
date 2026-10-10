# Communication_module

Исторический эксперимент выделения библиотеки обмена ППБ. Поддерживаемая интегрированная линия — [PPB_Project](https://github.com/Anatolichka2202/PPB_Project), `PPBCommunicationLib`. Этот репозиторий сохраняется для сравнения полезных отличий.

## Состав

- `include/` — интерфейсы связи, протокольного адаптера и транспорта.
- `src/` — обмен, сборка пакетов, UDP и очередь команд.
- `stubs/` — автономные экспериментальные зависимости.
- `tests/test_runner.cpp` — тестовый драйвер.

## Сборка

CMake 3.16+, C++17, Qt6 Core/Network. По умолчанию: `USE_REAL_DEPS=OFF`, `BUILD_TESTING=ON`.

```sh
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/Qt
cmake --build build
```

`USE_REAL_DEPS=ON` требует согласования зависимостей монолита и не является готовым подключением текущего PPB SDK. Сборка и аппаратный обмен в рамках оформления не проверялись.
