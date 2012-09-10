CC = clang
LD = $(CC)

CFLAGS = -Wall -pipe -Iinclude/
OFLAGS = 
LFLAGS = -lm -lc -lSDL -lGL

SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)

DEBUG = no
PROFILE = no
OPTIMIZATION = -O3

ifeq ($(DEBUG), yes)
	CFLAGS += -g -DDEBUG
	OPTIMIZATION = -O0
endif

ifeq ($(PROFILE), yes)
	CFLAGS += -g -pg -DPROFILE
endif

CFLAGS += $(OPTIMIZATION)

all: tinyflock

tinyflock: $(OBJECTS)
	$(LD) $(LFLAGS) $(OBJECTS) -o tinyflock

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf tinyflock  gmon.out *.save *.o core* vgcore*

rebuild: clean all

.PHONY : clean
.SILENT : clean
