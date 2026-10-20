CC=g++
CFLAGS=-c -Wall -std=c++14
LDFLAGS=-pthread
SOURCES=main.cpp parser.cpp helper.cpp resolver.cpp cache.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=dns_server

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

$(OBJECTS): parser.hpp helper.hpp resolver.hpp cache.hpp

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f *.o dns_server tests/parser_test tests/cache_test


tests/parser_test: tests/parser_test.cpp parser.cpp parser.hpp helper.cpp helper.hpp
	$(CC) -Wall -std=c++14 -pthread -I. tests/parser_test.cpp parser.cpp helper.cpp -o $@

tests/cache_test: tests/cache_test.cpp tests/cache_shared.cpp cache.cpp cache.hpp parser.cpp parser.hpp helper.cpp helper.hpp
	$(CC) -Wall -std=c++14 -pthread -I. tests/cache_test.cpp tests/cache_shared.cpp cache.cpp parser.cpp helper.cpp -o $@

test: all tests/parser_test tests/cache_test
	./tests/parser_test
	./tests/cache_test
	sudo unshare --net python3 tests/server_test.py ./dns_server

.PHONY: all test clean
