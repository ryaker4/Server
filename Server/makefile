# Компилятор и флаги
CXX      = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -pthread
LIBS     = -lboost_program_options -lcryptopp

# Исходники (в корне проекта)
SRC = main.cpp \
      serverInterface.cpp \
      logger.cpp \
      authdb.cpp \
      vector_processor.cpp \
      network_server.cpp

# каталоги для сборки
OBJ_DIR = build
BIN_DIR = bin

# объектные файлы кладём в OBJ_DIR, сохраняя имена
OBJ = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRC))

TARGET = $(BIN_DIR)/tcp_server

# PHONY цели
.PHONY: all clean run help rebuild dirs

# создать каталоги, если нужно
dirs:
	mkdir -p $(OBJ_DIR) $(BIN_DIR)

# Главное: создаём каталоги, собираем объектники и линковка
all: dirs $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ) $(LIBS)

# правило компиляции по одному файлу (объектный файл в build/)
# файл-зависимость: если build/ не существует, цель dirs создаст его
$(OBJ_DIR)/%.o: %.cpp | dirs
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Удаление всех результатов сборки
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Пересобрать всё
rebuild: clean all

# Удобные цели
run: all
	./$(TARGET)

help: all
	./$(TARGET) --help

