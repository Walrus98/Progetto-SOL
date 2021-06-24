#!/bin/bash

SERVER=build/server-test1
CLIENT=../build/client
SOCK_NAME=../temp/mysock

clear 

echo -e "Avvio Test1...\n"

echo -e "Compilo il Server e il Client\n"
cd ..
make build-server-test1
make build-client

valgrind --leak-check=full ${SERVER} &
SERVER_PID=$! 

cd scripts/
sleep 2

echo -e "Avvio Client 1 con -W\n"
${CLIENT} -f ${SOCK_NAME} -t 2000 -W ../tests/test1/file0.txt -r ../tests/test1/file0.txt

echo -e "Avvio Client 2 -w\n"
${CLIENT} -f ${SOCK_NAME} -t 2000 -w ../tests/test1/,2

echo -e "Avvio Client 3 -r\n";
${CLIENT} -f ${SOCK_NAME} -t 2000 -r ../tests/test1/file0.txt

sleep 5
kill -SIGHUP ${SERVER_PID}
echo -e "\n"
sleep 1

echo -e "Test Terminato!\n"
