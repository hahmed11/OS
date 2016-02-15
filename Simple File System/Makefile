CFLAGS = -c -g -Wall -std=gnu99 `pkg-config fuse --cflags --libs`

LDFLAGS = `pkg-config fuse --cflags --libs`

# Uncomment on of the following three lines to compile
SOURCES_TEST1= disk_emu.c sfs_api.c sfs_test.c sfs_api.h
SOURCES_TEST2= disk_emu.c sfs_api.c sfs_test2.c sfs_api.h
SOURCES= disk_emu.c sfs_api.c fuse_wrappers.c sfs_api.h

OBJECTS=$(SOURCES:.c=.o)
OBJECTS_TEST1=$(SOURCES_TEST1:.c=.o)
OBJECTS_TEST2=$(SOURCES_TEST2:.c=.o)
EXECUTABLE=sfs

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	gcc $(OBJECTS) $(LDFLAGS) -o $@

.c.o:
	gcc $(CFLAGS) $< -o $@

test1: $(OBJECTS_TEST1)
	gcc $(OBJECTS_TEST1) $(LDFLAGS) -o sfs_test1

test2: $(OBJECTS_TEST2)
	gcc $(OBJECTS_TEST2) $(LDFLAGS) -o sfs_test2

clean:
	rm -rf *.o *~ $(EXECUTABLE) sfs_test1 sfs_test2
