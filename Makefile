CC      ?= gcc
CFLAGS  ?= -O2 -Wall -Wextra
CFLAGS  += -DHAVE_STDIO

OBJS    := bitstream.o flac.o portme.o main.o

flacdemo: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

clean:
	rm -f flacdemo $(OBJS) result.bin

.PHONY: clean
