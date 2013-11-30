# macros
CC=gcc
CFLAGS=-c -Wall
LDFLAGS=
SOURCES=../trace.c test.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=test

# targets
all: $(SOURCES) $(EXECUTABLE)


$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@


.c.o:
	$(CC) $(CFLAGS) $< -o $@


# clean
clean:
	rm -f $(EXECUTABLE) $(OBJECTS)
