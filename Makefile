ARGUMENT	= -f temp/mysock -w .,2 -R 2 -t 1000
# ARGUMENT	= -f mysock -r Makefile -d prova/ -p
CC			= gcc
CFLAGS		= -pedantic -Wall -g

TARGETS_CLIENT	= client_network.o client.o list_utils.o utils.o

TARGETS_SERVER	= server.o server_config.o server_storage.o server_network.o						\
				server_network_dispatcher.o server_network_worker.o server_network_handler.o		\
			 	server_signal_handler.o server_packet_handler.o utils.o list_utils.o icl_hash.o		\

.PHONY: all clear $(TARGETS_SERVER) $(TARGETS_CLIENT)
.SUFFIXES: .c .h .o

all: build-client build-server clear
	-rm temp/mysock
	cp -r * /mnt/d/Desktop/Progetto-SOL/

build-client: $(TARGETS_CLIENT)
	$(CC) $(CFLAGS) build/obj/client/*.o build/obj/core/*.o -o build/client
	
build-server: $(TARGETS_SERVER)
	-rm temp/mysock
	$(CC) $(CFLAGS) -pthread build/obj/server/*.o build/obj/core/*.o -o build/server

clear:
	rm -R build/obj/client/*.o
	rm -R build/obj/server/*.o
	rm -R build/obj/core/*.o

client: build-client
	clear
	valgrind --leak-check=full --track-origins=yes --tool=memcheck build/client $(ARGUMENT)

server: build-server
	clear
	valgrind --leak-check=full --track-origins=yes --tool=memcheck build/server

# ================================= TEST1 =================================

test1: build-client build-server-test1 clear
	-rm temp/mysock
	cp -r * /mnt/d/Desktop/Progetto-SOL/

build-server-test1: $(TARGETS_SERVER)
	-rm temp/mysock
	$(CC) $(CFLAGS) -DCONFIG_PATH=../build/config-test1.txt -pthread build/obj/server/*.o build/obj/core/*.o -o build/server-test1

server-test1: build-server-test1
	clear
	valgrind --leak-check=full --track-origins=yes --tool=memcheck build/server-test1

# ================================= TEST2 =================================

test2: build-client build-server-test2 clear
	-rm temp/mysock
	cp -r * /mnt/d/Desktop/Progetto-SOL/

build-server-test2: $(TARGETS_SERVER)
	-rm temp/mysock
	$(CC) $(CFLAGS) -DCONFIG_PATH=build/config-test2.txt -pthread build/obj/server/*.o build/obj/core/*.o -o build/server-test2

server-test2: build-server-test2
	clear
	valgrind --leak-check=full --track-origins=yes --tool=memcheck build/server-test2

# ================================= CLIENT =================================

client_network.o:
	$(CC) $(CFLAGS) -c client/src/client_network.c -o build/obj/client/$@
client.o:
	$(CC) $(CFLAGS) -c client/src/client.c -o build/obj/client/$@

# ================================= CORE ==================================

utils.o:
	$(CC) $(CFLAGS) -c core/src/utils.c -o build/obj/core/$@

list_utils.o:
	$(CC) $(CFLAGS) -c core/src/list_utils.c -o build/obj/core/$@

# ================================= SERVER =================================

server.o:
	$(CC) $(CFLAGS) -c server/src/server.c -o build/obj/server/$@

server_config.o:
	$(CC) $(CFLAGS) -c server/src/server_config.c -o build/obj/server/$@

server_storage.o:
	$(CC) $(CFLAGS) -c -pthread server/src/server_storage.c -o build/obj/server/$@

icl_hash.o:
	$(CC) $(CFLAGS) -c server/src/icl_hash.c -o build/obj/server/$@

server_network.o:
	$(CC) $(CFLAGS) -pthread -c server/src/server_network.c -o build/obj/server/$@

server_network_dispatcher.o:
	$(CC) $(CFLAGS) -c server/src/server_network_dispatcher.c -o build/obj/server/$@

server_network_worker.o:
	$(CC) $(CFLAGS) -c server/src/server_network_worker.c -o build/obj/server/$@

server_network_handler.o:
	$(CC) $(CFLAGS) -pthread -c server/src/server_network_handler.c -o build/obj/server/$@

server_signal_handler.o:
	$(CC) $(CFLAGS) -pthread -c server/src/server_signal_handler.c -o build/obj/server/$@

server_packet_handler.o:
	$(CC) $(CFLAGS) -c server/src/server_packet_handler.c -o build/obj/server/$@


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

# clear:
# 	-rm a.out