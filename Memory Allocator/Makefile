CFLAGS = -c -Wall
CFLAGS+= -g
LDFLAGS = 

SOURCES= malloc.c test.c

OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=malloc

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	gcc $(OBJECTS) $(LDFLAGS) -o $@

.c.o:
	gcc $(CFLAGS) $< -o $@

clean:
	rm -rf *.o *~ 
