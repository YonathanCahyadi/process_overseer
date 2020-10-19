#!/bin/bash

MAIN_PROG=../controler.out localhost 1234
TEST_PROG=./testing/test_exec 1234

# clear the screen
clear


NC='\033[0m'
Purple='\033[0;35m'
RED='\033[0;31m' 
Green='\033[0;32m'     

PROGRAM=../controler.out
TESTING_PROGRAM=./testing/test_exec

echo -e "${Purple}Begin Testing${NC}"

echo -e "${Green}Execute With no arguments and no options${NC}"
echo -e "${PROGRAM} localhost 1234"
${PROGRAM} localhost 1234

echo -e "${Green}Execute with no option 5x${NC}"
for i in {1..5}
do
    echo -e "${PROGRAM} localhost 1234 ./testing/test ${i}"
    ${PROGRAM} localhost 1234 ${TESTING_PROGRAM} ${i}
done


echo -e "${Green}Execute with out option 5x${NC}"
for i in {1..5}
do
    echo -e "${PROGRAM} localhost 1234 -o './testing/output/${i}.log' ./testing/test ${i}"
    ${PROGRAM} localhost 1234 -o "./testing/output/${i}.log" ${TESTING_PROGRAM} ${i}
done

echo -e "${Green}Execute with log option 5x${NC}"
for i in {6..10}
do
    echo -e "${PROGRAM} localhost 1234 -log './testing/log/${i}.log' ./testing/test ${i}"
    ${PROGRAM} localhost 1234 -log "./testing/log/${i}.log" ${TESTING_PROGRAM} ${i}
done

echo -e "${Green}Execute with log and out option 5x${NC}"
for i in {11..15}
do
    echo -e "${PROGRAM} localhost 1234 -log './testing/log/${i}.log' -o './testing/output/${i}.log' ./test ${i}"
    ${PROGRAM} localhost 1234  -o "./testing/output/${i}.log" -log "./testing/log/${i}.log" ${TESTING_PROGRAM} ${i}
done

echo -e "${Green}Execute With wrong arguments file${NC}"
echo -e "${PROGRAM} localhost 1234 -o /'asd'/ -t 10 -log /'asd'/asduh/ /test 2"
${PROGRAM} localhost 1234 -o /"asd"/ -t 10 -log /"asd"/asduh/ /test 2
