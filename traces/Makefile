# macros
CC=gcc
CFLAGS=-c -Wall
LDFLAGS=
SOURCES=test.c
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=test

# targets
all: $(SOURCES) $(EXECUTABLE)


$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@


.cpp.o:
	$(CC) $(CFLAGS) $< -o $@


# clean
clean:
	rm -f $(EXECUTABLE) $(OBJECTS)
