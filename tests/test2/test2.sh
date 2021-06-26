#!/bin/bash

RED='\033[0;31m'
NC='\033[0m'

echo -e "${RED}Avvio test 2...\n${NC}"

echo -e "${RED}Compilo il Server e il Client\n${NC}"

cd ../..
make build-server-test2
make build-client

echo -e "${RED}\nCompilazione terminata!\n${NC}"

sleep 1

echo -e "${RED}\nAvvio Server test2!\n${NC}"

valgrind --leak-check=full build/server-test2 &
SERVER_PID=$! 

sleep 1

echo -e "${RED}\nAvvio il Client 1\n${NC}"

build/client -f temp/mysock -t 200 -W tests/test2/file0.txt -r tests/test2/file0.txt -p &

echo -e "${RED}\nAvvio il Client 2\n${NC}"

build/client -f temp/mysock -t 200 -W tests/test2/file1.txt -r tests/test2/file1.txt -p &

echo -e "${RED}\nAvvio il Client 3\n${NC}"

build/client -f temp/mysock -t 200 -W tests/test2/file2.txt -r tests/test2/file2.txt -p &

sleep 5
kill -SIGHUP ${SERVER_PID}
echo -e "\n"
sleep 1

echo -e "${RED}\nTest terminato con successo!\n${NC}"