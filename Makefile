CXX = g++
LIBS = -lboost_program_options
CXXFLAGS = -Wall -std=c++11 -s -O2 -flto -pthread
TARGET = sockroute
PREFIX = /usr/local

$(TARGET): main.cpp json.hpp
	$(CXX) $< $(LIBS) $(CXXFLAGS) -o $@

.PHONY: clean install

clean:
	$(RM) $(TARGET)

install:
	cp $(TARGET) $(PREFIX)/bin/$(TARGET)
