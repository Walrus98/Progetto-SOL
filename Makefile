
CC			= gcc
CFLAGS		= -pedantic -Wall -g
INCLUDES	= -I .

TARGETS		= server.o server_config.o utils.o server_storage.o server_cache_handler.o \
			server_cache_fifo.o list_utils.o icl_hash.o server_network.o server_network_dispatcher.o \
			server_network_worker.o server_network_handler.o server_signal_handler.o

.PHONY: all test clean $(TARGETS)
.SUFFIXES: .c .h .o

server: $(TARGETS)
	-rm mysock
	gcc -pthread build/obj/*.o -o build/server
	cp -r * /mnt/d/Desktop/Progetto-SOL/
	rm -R build/obj/*.o
#	build/server
	valgrind build/server --leak-check=full --track-origins=yes --tool=memcheck

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

list_utils.o:
	gcc -c server/src/list_utils.c -o build/obj/$@

icl_hash.o:
	gcc -c server/src/icl_hash.c -o build/obj/$@

server_network.o:
	gcc -pthread -c server/src/server_network.c -o build/obj/$@

server_network_dispatcher.o:
	gcc -c server/src/server_network_dispatcher.c -o build/obj/$@

server_network_worker.o:
	gcc -c server/src/server_network_worker.c -o build/obj/$@

server_network_handler.o:
	gcc -pthread -c server/src/server_network_handler.c -o build/obj/$@

server_signal_handler.o:
	gcc -pthread -c server/src/server_signal_handler.c -o build/obj/$@

start:
	build/server

commit:
	cp -r * /mnt/d/Desktop/Progetto-SOL/

valgrind:
	valgrind build/server –-leak-check=full

clear:
	rm -R build/obj/*.o

	

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