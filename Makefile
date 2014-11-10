CC = clang
LD = $(CC)

CFLAGS = -Wall -pipe -Iinclude/ -std=gnu11 -march=native
OFLAGS = 
LFLAGS = -lfann -lm -lc -lGL -lglfw -lpthread

SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)

DEBUG = no
PROFILE = no

OPTIMIZATION = -O3

ifeq ($(DEBUG), yes)
	CFLAGS += -g -DDEBUG
	OPTIMIZATION = -O2
endif

ifeq ($(PROFILE), yes)
	CFLAGS += -g -pg -DPROFILE
endif

CFLAGS += $(OPTIMIZATION)

all: tinyflock nn_trainer

tinyflock: modules $(OBJECTS)
	$(LD) $(LFLAGS) $(OBJECTS) -o tinyflock

nn_trainer: tinyflock
	$(CC) $(LFLAGS) nn_trainer/flock_nn.c -o nn_trainer/flock_nn

modules:
	git submodule update --init

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf tinyflock nn_trainer/flock_nn gmon.out *.save *.o core* vgcore*

rebuild: clean all

.PHONY : clean modules
.SILENT : clean modules
