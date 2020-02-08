#
# Basic makefile for csc424-202
# author: bjr
# date: 24 jan 2020
#

MF= Makefile
F= arrange

RED= \033[0;31m
GREEN= \033[0;32m
NC= \033[0m

all:
	make -f ${MF} clean
	make -f ${MF} basic-test

$F: $F.c
	cc -o $@ $<

basic-test: $F
	./$F -r c:a:f:e a:b:c:d:e > test.out
	diff test.out basic-test.ref
	@echo "***\n*** ${GREEN}passed the basic test${NC}\n***"

run-verbose: $F
	./$F -vvr c:a:f:e a:b:c:d:e

clean:
	-rm $F test.out

