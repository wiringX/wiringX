GCC = gcc
CFLAGS = -g -Wall -isystem. -isystem.. -I../
SRC = $(wildcard *.c)
INCLUDES = $(wildcard ../*.h)
LIBS = $(wildcard ../*.c)
OBJS = $(SRC:.c=.o)
PROGRAMS = $(patsubst %.c,%,$(SRC))

all: $(PROGRAMS)

$(PROGRAMS): $(LIBS)
	$(GCC) $(CFLAGS) -o $@ $(patsubst %,%.c,$@) $(LIBS)
	
clean:
	rm *.so* || true
	rm *.a* || true