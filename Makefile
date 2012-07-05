CC = gcc
AS = as

CFLAGS = -Wall -pipe -Iinclude/
OFLAGS = 
LFLAGS = $(CFLAGS) -Llib/ -lm
ASFLAGS =
PEDANTIC_FLAGS = -ansi -pedantic -pedantic-errors

SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)

BIN_DIR =

DEBUG = no
PROFILE = no
PEDANTIC = no
OPTIMIZATION = -O3

ifeq ($(DEBUG), yes)
	CFLAGS += -g
	OPTIMIZATION = -O0
endif

ifeq ($(PROFILE), yes)
	CFLAGS += -pg
endif

ifeq ($(PEDANTIC), yes)
	CFLAGS += $(PEDANTIC_FLAGS)
endif

CFLAGS += $(OPTIMIZATION)

all: tinyflock

install: libtvm tvmi
	cp -f $(BIN_DIR)/tvmi $(INSTALL_PREFIX)/bin/
	cp -rf $(INC_DIR)/tvm $(INSTALL_PREFIX)/include/
	cp -f $(LIB_DIR)/libtvm* $(INSTALL_PREFIX)/lib/

uninstall:
	rm -rf /usr/bin/tvmi
	rm -rf /usr/include/tvmi
	rm -rf /usr/lib/libtvm*

libtvm: $(LIBTVM_OBJECTS)
	ar rcs $(LIB_DIR)/libtvm.a $(LIBTVM_OBJECTS)

# Build the TVM interpreter
tinyflock: $(OBJECTS)
	$(CC) $(LFLAGS) $(OBJECTS) -lSDL -lGL -o tinyflock

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf tinyflock  gmon.out *.save *.o core* vgcore*

rebuild: clean all

.PHONY : clean
.SILENT : clean
