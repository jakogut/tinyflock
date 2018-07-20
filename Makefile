CC = clang
LD = $(CC)

CFLAGS = -Wall -pipe -Iinclude/ -std=gnu11
OFLAGS = 
LFLAGS = -lm -lc -lGL -lglfw

SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)

DEBUG = no
PROFILE = no

OPTIMIZATION = -O3

OUTPUT = tinyflock

ifeq ($(CC), emcc)
	LFLAGS += -s LEGACY_GL_EMULATION=1 -s USE_GLFW=3
	OUTPUT = tinyflock.html
endif

ifeq ($(DEBUG), yes)
	CFLAGS += -g -DDEBUG
	OPTIMIZATION = -O2
endif

ifeq ($(PROFILE), yes)
	CFLAGS += -g -pg -DPROFILE
endif

CFLAGS += $(OPTIMIZATION)

all: tinyflock

tinyflock: modules $(OBJECTS)
	$(LD) $(LFLAGS) $(OBJECTS) -o $(OUTPUT)

modules:
	git submodule update --init

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf tinyflock  gmon.out *.save *.o core* vgcore* *.html *.js

rebuild: clean all

.PHONY : clean modules
.SILENT : clean modules
