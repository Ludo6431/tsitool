.PHONY:all dist-clean clean

EXEC := tsitool
OBJS := tsidump.o tsitool.o

# --------

CC = gcc
CFLAGS = -Wall -g -I.
LDFLAGS = #-lm

all:$(EXEC)

dist-clean:
	rm $(OBJS)

clean:
	rm $(EXEC) $(OBJS)

%.o:%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(EXEC):$(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

