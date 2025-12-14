CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread -D_GNU_SOURCE
LIBS = -lUnitTest++ -lcrypto++

# Основные цели
TARGET = server
TEST_TARGET = test_runner

# Исходные файлы
SRCS = main.cpp server.cpp authenticator.cpp calculator.cpp logger.cpp network_helper.cpp client_handler.cpp
TEST_SRCS = test_main.cpp test_authenticator.cpp test_calculator.cpp test_logger.cpp test_client_handler.cpp

# Объектные файлы
OBJS = $(SRCS:.cpp=.o)
TEST_OBJS = $(TEST_SRCS:.cpp=.o)

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

test: $(TEST_TARGET)
	@echo "Тесты скомпилированы. Запустите ./test_runner для выполнения тестов."

$(TEST_TARGET): $(filter-out main.o, $(OBJS)) $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(TEST_TARGET) $(OBJS) $(TEST_OBJS)
	rm -f *.log test_*.txt

# Зависимости
main.o: main.cpp server.h
server.o: server.cpp server.h logger.h authenticator.h network_helper.h client_handler.h
authenticator.o: authenticator.cpp authenticator.h
calculator.o: calculator.cpp calculator.h
logger.o: logger.cpp logger.h
network_helper.o: network_helper.cpp network_helper.h
client_handler.o: client_handler.cpp client_handler.h network_helper.h authenticator.h calculator.h logger.h

# Тестовые зависимости
test_authenticator.o: test_authenticator.cpp test_authenticator.h authenticator.h
test_calculator.o: test_calculator.cpp test_calculator.h calculator.h
test_logger.o: test_logger.cpp test_logger.h logger.h
test_client_handler.o: test_client_handler.cpp test_client_handler.h client_handler.h
