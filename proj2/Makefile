#
# makefile for packet passaround
# 3 feb 2019
# bjr
#

PORT= 
# these will be the IP address given by ifconfig on the two
# AWS instances (or lab computers)
MC= 
MS= 

all:
	make passaround

passaround: passaround.c passaround.h 
	cc -o $@ $< 
	
test1: passaround
	./passaround -n 3 -m localhost:localhost:localhost ${PORT} > test1.out
	-diff test1.ref test1.out

test2:
	@echo make test2-s on ${MS} then make test2-c on ${MC}

test2-s: passaround test2-ref
	@echo Run this on ${MS}
	@echo Then run make test2-c on ${MC}
	./passaround -n 2 ${PORT} > test2-s.out
	-diff test2-s.ref test2-s.out
	
test2-c: passaround test2-ref
	@echo First run make test2-s on ${MS}
	@echo The run this on ${MC}
	./passaround -n 3 -m ${MS}:${MC}:${MS}:${MC} ${PORT} > test2-c.out
	-diff test2-c.ref test2-c.out

test2-ref:
	@echo "S: ${MS}:${PORT} |${MC}:${MS}:${MC}|" > test2-c.ref
	@echo "R: ${MS}:${PORT} |${MS}:${MC}|" >> test2-c.ref
	@echo "S: ${MS}:${PORT} |${MC}|" >> test2-c.ref
	@echo "R: ${MS}:${PORT} ||" >> test2-c.ref
	@echo "R: ${MC}:${PORT} |${MC}:${MS}:${MC}|" > test2-s.ref
	@echo "S: ${MC}:${PORT} |${MS}:${MC}|" >> test2-s.ref
	@echo "R: ${MC}:${PORT} |${MC}|" >> test2-s.ref
	@echo "S: ${MC}:${PORT} ||" >> test2-s.ref
	
clean:
	-rm passaround test1.out test2-s.out test2-c.out test2-c.ref test2-s.ref
