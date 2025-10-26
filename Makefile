CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic
LIBS = -lssl -lcrypto
TARGET = server
SOURCES = main.cpp server.cpp logger.cpp authenticator.cpp calculator.cpp \
          client_handler.cpp network_helper.cpp
OBJECTS = $(SOURCES:.cpp=.o)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: clean
