# Server

## Структура проекта
.
├── main.cpp
├── serverInterface.cpp / .h      // Обработка параметров командной строки (Boost)
├── network_server.cpp / .h       // TCP-сервер, обработка клиента, векторы
├── vector_processor.cpp / .h     // Обработка векторов (сумма)
├── authdb.cpp / .h               // Журнал базы пользователей
├── logger.cpp / .h               // Логирование
├── Makefile                      // Сборка проекта
└── README.md                     // Этот файл

## Зависимости
Для сборки требуется:
- g++ (C++17)
- Make
- Boost.Program_options
- Crypto++
- ОС Linux

````
sudo apt install libboost-program-options-dev libcrypto++-dev
````

## Сборка проекта
make - может не собрать проекта, а лишь создать папки bin и build
make all - работает всегда
После сборки проект будет:
bin/tcp_server










