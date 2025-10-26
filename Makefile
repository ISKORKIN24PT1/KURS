CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic
LDFLAGS = -lssl -lcrypto

SOURCES = main.cpp server.cpp logger.cpp authenticator.cpp calculator.cpp client_handler.cpp network_helper.cpp
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = server

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: clean
