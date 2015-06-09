GCC = gcc
CFLAGS = -g -Wall -isystem. -isystem.. -I../
SRC = $(wildcard *.c)
INCLUDES = $(wildcard ../*.h)
LIBS = $(wildcard ../*.c)
OBJS = $(LIBS:.c=.o)
PROGRAMS = $(patsubst %.c,%,$(SRC))

all: $(OBJS) $(PROGRAMS)

$(OBJS): $(LIBS)
	$(GCC) $(CFLAGS) -o $@ -c $(patsubst %.o,%.c,$@)

blink: $(OBJS) blink.c
	$(GCC) $(CFLAGS) -o $@ $(patsubst %,%.c,$@) $(OBJS)

read: $(OBJS) read.c
	$(GCC) $(CFLAGS) -o $@ $(patsubst %,%.c,$@) $(OBJS)

interrupt: $(OBJS) interrupt.c
	$(GCC) $(CFLAGS) -o $@ $(patsubst %,%.c,$@) $(OBJS) -pthread

clean:
	rm *.so* || true
	rm *.a* || true