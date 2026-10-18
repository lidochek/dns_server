CC=g++
CFLAGS=-c -Wall
LDFLAGS=
SOURCES=main.cpp parser.hpp parser.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=dns_server

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o dns_server
