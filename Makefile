
CC			= gcc
CFLAGS		= -pedantic -Wall -g
INCLUDES	= -I .

TARGETS		= server.o server_config.o utils.o server_storage.o server_cache_handler.o server_cache_fifo.o icl_hash.o

.PHONY: all test clean $(TARGETS)
.SUFFIXES: .c .h .o

server: $(TARGETS)
	gcc build/obj/*.o -o build/server
	cp -r * /mnt/d/Desktop/Progetto-SOL/
#	build/server
	valgrind build/server –-leak-check=full

server.o:
	gcc -c server/src/server.c -o build/obj/$@

server_config.o:
	gcc -c server/src/server_config.c -o build/obj/$@

utils.o:
	gcc -c core/src/utils.c -o build/obj/$@

server_storage.o:
	gcc -c server/src/server_storage.c -o build/obj/$@

server_cache_handler.o:
	gcc -c server/src/server_cache_handler.c -o build/obj/$@

server_cache_fifo.o:
	gcc -c server/src/server_cache_fifo.c -o build/obj/$@

icl_hash.o:
	gcc -c server/src/icl_hash.c -o build/obj/$@

start:
	build/server

commit:
	cp -r * /mnt/d/Desktop/Progetto-SOL/

valgrind:
	valgrind build/server –-leak-check=full

	

# all: $(TARGETS)
# 	$(CC) $^

# main.o: main.c utils.h
# $(CC) $(CFLAGS) $(INCLUDES) -c $<
# $(CC) $(CFLAGS) $(INCLUDES) -c main.c

# utils.o: utils.c utils.h
# $(CC) $(CFLAGS) $(INCLUDES) -c main.c

# %.o: %.c %.h

# test:
# 	./a.out input.txt output.txt

# clean:
# 	-rm a.out