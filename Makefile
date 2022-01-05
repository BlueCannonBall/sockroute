CXX = g++
LIBS = -lboost_program_options
CXXFLAGS = -Wall -s -O2 -flto -pthread -Bsymbolic -fno-semantic-interposition -fno-stack-protector
TARGET = sockroute
PREFIX = /usr/local

$(TARGET): main.cpp json.hpp
	$(CXX) $< $(LIBS) $(CXXFLAGS) -o $@

.PHONY: clean install

clean:
	$(RM) $(TARGET)

install:
	cp $(TARGET) $(PREFIX)/bin/$(TARGET)