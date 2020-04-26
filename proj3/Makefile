#
# Name: pikachu
# Date: 14 feb 2016
# 
# Note: Makefiles begin with a captal M
#   Indentation MUST be with a tab
#
# tagets build, test, clean and submit are required.
# 

SHELL= /bin/bash

SCP_FILES= ttftp.c ttftp-client.c ttftp-server.c ttftp.h Makefile
SCP_HOST= 18.235.255.35
SCP_ALT= 54.173.147.78
SCP_PEM= ../keys/csc424-keypair.pem
SVN_USR= class

COPTS= -g -Wall
#COPTS= -Wall

# adjust these macro values accordingly.
# you may have to adjust PORT= to claim an available port
PORT= 33031
FILE_S = file-on-server.txt
FILE_C = file-on-client.txt
SERVER_HOST = localhost

all:
	@echo "Targets: build, test, clean, submit"

build:
	make ttftp

ttftp-server.o: ttftp-server.c ttftp.h
	cc ${COPTS} -c -o $@ $<

ttftp-client.o: ttftp-client.c ttftp.h
	cc ${COPTS} -c -o $@ $<

ttftp: ttftp.c ttftp-server.o ttftp-client.o ttftp.h
	cc ${COPTS} -o $@ $< ttftp-client.o ttftp-server.o 

run-server: ttftp
	echo `date` >> ${FILE_S}
	./ttftp ${PORT}
	
test: ttftp
	echo `date` >> ${FILE_S}
	./ttftp -L ${PORT} &
	@echo "server started"	
	./ttftp -h ${SERVER_HOST} -f ${FILE_S} ${PORT} > ${FILE_C}
	diff ${FILE_S} ${FILE_C}


scp-all:
	for F in ${SCP_FILES}; do \
	scp -i ${SCP_PEM} $$F ubuntu@${SCP_HOST}:~/csc424.svn/${SVN_USR}/proj3/  ;\
	[ -z "${SCP_ALT}" ] || scp -i ${SCP_PEM} $$F ubuntu@${SCP_ALT}:~/csc424.svn/${SVN_USR}/proj3/  ;\
	done

clean:
	-rm ttftp ttftp-server.o ttftp-client.o ${FILE_S} ${FILE_C}

submit:
	@echo svn add your work
	svn commit -m "submitted for grading"


