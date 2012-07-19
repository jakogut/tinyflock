CC = gcc
LD = gold

CFLAGS = -Wall -pipe -Iinclude/
OFLAGS = 
LFLAGS = $(CFLAGS) -lm
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

tinyflock: $(OBJECTS)
	$(CC) $(LFLAGS) $(OBJECTS) -lSDL -lGL -o tinyflock

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf tinyflock  gmon.out *.save *.o core* vgcore*

rebuild: clean all

.PHONY : clean
.SILENT : clean
