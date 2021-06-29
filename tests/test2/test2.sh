#!/bin/bash

RED='\033[0;31m'
NC='\033[0m'

echo -e "${RED}Avvio Test2...\n${NC}"

echo -e "${RED}Compilo il Server e il Client\n${NC}"

cd ../..
make build-server
make build-client

echo -e "${RED}\nCompilazione terminata!\n${NC}"

sleep 1

echo -e "${RED}\nAvvio Server Test2!\n${NC}"

valgrind --leak-check=full build/server build/config-test2.txt &
SERVER_PID=$! 

sleep 1

echo -e "${RED}\nAvvio il Client 1 che inserisce un file > 1MB\n${NC}"

build/client -f temp/mysock -t 200 -W tests/test2/file0.txt &

echo -e "${RED}\nAvvio il Client 2 che inserisce 5 file da 0.25MB circa\n${NC}"

build/client -f temp/mysock -t 200 -w tests/test2/files1/ &

echo -e "${RED}\nAvvio il Client 3 che inserisce più di 10 file\n${NC}"

build/client -f temp/mysock -t 200 -w tests/test2/files2/ &

sleep 5
kill -SIGHUP ${SERVER_PID}
echo -e "\n"
sleep 1

echo -e "${RED}\nTest terminato con successo!\n${NC}"