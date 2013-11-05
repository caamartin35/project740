# macros
CC=gcc
CFLAGS=-c -Wall
LDFLAGS=
SOURCES=test.c
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=driver

# targets
all: $(SOURCES) $(EXECUTABLE)


$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@


.cpp.o:
	$(CC) $(CFLAGS) $< -o $@


# clean
clean:
	rm -f $(EXECUTABLE) $(OBJECTS)
