#\
 Author: 			Ben Tomlin \
 Student Id:		btomlin \
 Student Nbr:		834198 \
 Date:			Apr 2018 \

CC			= gcc
CFLAG		= -l pthread -g
EXE			= server
LINK_OBJECT = server.o logger.o http.o httpStructures.o \
				tcpSocketIo.o byteString.o filesystem.o regexTool.o

all: server

$(EXE): $(LINK_OBJECT) utility/bool.h
	$(CC) $(CFLAG) -o server $(LINK_OBJECT)

logger.o: utility/logger.c utility/logger.h
	$(CC) $(CFLAG) -c utility/logger.c

server.o: server.c server.h
	$(CC) $(CFLAG) -c server.c
	
http.o: http/http.c http/http.h http/httpStructures.h
	$(CC) $(CFLAG) -c http/http.c 
	
httpStructures.o: http/httpStructures.c http/httpStructures.h
	$(CC) $(CFLAG) -c http/httpStructures.c
	
tcpSocketIo.o: utility/tcpSocketIo.c utility/tcpSocketIo.h
	$(CC) $(CFLAG) -c utility/tcpSocketIo.c
	
byteString.o: utility/byteString.c utility/byteString.h
	$(CC) $(CFLAG) -c utility/byteString.c
	
filesystem.o: utility/filesystem.c utility/filesystem.h
	$(CC) $(CFLAG) -c utility/filesystem.c
	
regexTool.o: utility/regexTool.c utility/regexTool.h
	$(CC) $(CFLAG) -c utility/regexTool.c
	
clean:
	rm server.o logger.o tcpSocketIo.o httpStructures.o \
	http.o byteString.o regexTool.o filesystem.o server
