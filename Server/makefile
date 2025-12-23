# Компилятор и флаги
CXX      = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -pthread
TEST_CXXFLAGS = -std=c++11 -g -Wall -Wextra
LIBS     = -lboost_program_options -lcryptopp
TEST_LIBS = -lUnitTest++ -lboost_program_options -lcryptopp

# Исходники (в корне проекта)
SRC = main.cpp \
      serverInterface.cpp \
      logger.cpp \
      authdb.cpp \
      vector_processor.cpp \
      network_server.cpp \
      auth_handler.cpp \
      network_utils.cpp \
      vector_handler.cpp

# каталоги для сборки
OBJ_DIR = build
BIN_DIR = bin

# объектные файлы кладём в OBJ_DIR, сохраняя имена
OBJ = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRC))

TARGET = $(BIN_DIR)/tcp_server
TEST_TARGET = $(BIN_DIR)/test_server

# Файлы для тестирования
TEST_SRC = test_server.cpp \
           vector_handler.cpp \
           vector_processor.cpp \
           logger.cpp \
           network_utils.cpp \
           authdb.cpp \
           auth_handler.cpp \
           serverInterface.cpp

# PHONY цели
.PHONY: all clean run help rebuild dirs test

# Главное: создаём каталоги, собираем объектники и линковка
all: dirs $(TARGET)

# Цель для сборки тестов
test: dirs $(TEST_TARGET)

# создать каталоги, если нужно
dirs:
	mkdir -p $(OBJ_DIR) $(BIN_DIR)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ) $(LIBS)

# правило компиляции по одному файлу (объектный файл в build/)
# файл-зависимость: если build/ не существует, цель dirs создаст его
$(OBJ_DIR)/%.o: %.cpp | dirs
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Сборка тестового приложения
$(TEST_TARGET): $(TEST_SRC)
	$(CXX) $(TEST_CXXFLAGS) -o $@ $(TEST_SRC) $(TEST_LIBS)

# Удаление всех результатов сборки
clean:
	rm -rf $(OBJ_DIR) $(TARGET) $(TEST_TARGET)

# Пересобрать всё
rebuild: clean all

# Удобные цели
run: all
	./$(TARGET)

help: all
	./$(TARGET) --help

# Запуск тестов
run-test: test
	./$(TEST_TARGET)
