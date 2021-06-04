
CC			= gcc
CFLAGS		= -pedantic -Wall -g
INCLUDES	= -I .

TARGETS		= server.o server_config.o utils.o server_storage.o server_cache_handler.o \
			server_cache_fifo.o list_utils.o server_network.o server_network_dispatcher.o \
			server_network_worker.o server_network_handler.o server_signal_handler.o \
			server_packet_handler.o icl_hash.o

.PHONY: all test clean $(TARGETS)
.SUFFIXES: .c .h .o

server: $(TARGETS)
	-rm mysock
	gcc $(CFLAGS) -pthread build/obj/*.o -o build/server
	cp -r * /mnt/d/Desktop/Progetto-SOL/
	rm -R build/obj/*.o
#	build/server
	valgrind build/server --leak-check=full --track-origins=yes --tool=memcheck

server.o:
	gcc $(CFLAGS) -c server/src/server.c -o build/obj/$@

server_config.o:
	gcc $(CFLAGS) -c server/src/server_config.c -o build/obj/$@

utils.o:
	gcc $(CFLAGS) -c core/src/utils.c -o build/obj/$@

server_storage.o:
	gcc $(CFLAGS) -c server/src/server_storage.c -o build/obj/$@

server_cache_handler.o:
	gcc $(CFLAGS) -pthread -c server/src/server_cache_handler.c -o build/obj/$@

server_cache_fifo.o:
	gcc $(CFLAGS) -c server/src/server_cache_fifo.c -o build/obj/$@

list_utils.o:
	gcc $(CFLAGS) -c server/src/list_utils.c -o build/obj/$@

icl_hash.o:
	gcc $(CFLAGS) -c server/src/icl_hash.c -o build/obj/$@

server_network.o:
	gcc $(CFLAGS) -pthread -c server/src/server_network.c -o build/obj/$@

server_network_dispatcher.o:
	gcc $(CFLAGS) -c server/src/server_network_dispatcher.c -o build/obj/$@

server_network_worker.o:
	gcc $(CFLAGS) -c server/src/server_network_worker.c -o build/obj/$@

server_network_handler.o:
	gcc $(CFLAGS) -pthread -c server/src/server_network_handler.c -o build/obj/$@

server_signal_handler.o:
	gcc $(CFLAGS) -pthread -c server/src/server_signal_handler.c -o build/obj/$@

server_packet_handler.o:
	gcc $(CFLAGS) -c server/src/server_packet_handler.c -o build/obj/$@

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