TARGETS = line_processor 

SRCS = line_processor.c

all: $(TARGETS)

line_processor : line_processor.c
	gcc -Wall -g -lm -pthread --std=gnu99 -o $@ $< 

clean:
	rm -f $(TARGETS) *.o
