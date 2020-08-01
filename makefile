OPT = -g1 -Og

LIB_SOURCES1 = main.c alloc.c filesystem.c
LIB_SOURCES = $(addprefix src/, $(LIB_SOURCES1))

CC = gcc
TARGET = awsmvm
LIB_OBJECTS =$(LIB_SOURCES:.c=.o)
LDFLAGS= -L. $(OPT) -Wall -Wextra -L../wasmrun/
LIBS= -lm -lawsm -liron
ALL= $(TARGET)
CFLAGS = -Isrc/ -Iinclude/ -I../wasmrun/include -std=gnu11 -c $(OPT) -Wall  -Wextra -Werror=implicit-function-declaration -Wformat=0 -D_GNU_SOURCE -Wwrite-strings -Werror -Werror=maybe-uninitialized  #-fprofile-use=profile.out -v -fprofile-generate=./profile.out   #

ifneq ($(BUILD),release)
    CFLAGS += -DDEBUG
    OPT = -g3 -O0
endif

$(TARGET): $(LIB_OBJECTS)
	$(CC) $(LDFLAGS) $(LIB_OBJECTS) $(LIBS) -o $@

release debug:
	$(MAKE) BUILD=$@

all: $(ALL)

.c.o: $(HEADERS) $(LEVEL_CS)
	$(CC) $(CFLAGS) $< -o $@ -MMD -MF $@.depends

depend: h-depend
clean:
	rm -f $(LIB_OBJECTS) $(ALL) src/*.o.depends src/*.o

-include $(LIB_OBJECTS:.o=.o.depends)
