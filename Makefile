CC			= gcc
CFLAG		= -l pthread -g
EXE			= server
LINK_OBJECT = server.o logger.o http.o httpStructures.o \
				tcpSocketIo.o byteString.o

$(EXE): $(LINK_OBJECT)
	$(CC) $(CFLAG) -o server $(LINK_OBJECT)
	
logger.o: utility/logger.c utility/logger.h
	$(CC) $(CFLAG) -c utility/logger.c

server.o: server.c server.h
	$(CC) $(CFLAG) -c server.c
	
http.o: http/http.c http/http.h http/httpStructures.h
	$(CC) $(CFLAG) -c http/http.c 
	
httpStructures.o: http/httpStructures.c http/httpStructures.o
	$(CC) $(CFLAG) -c http/httpStructures.c
	
tcpSocketIo.o: utility/tcpSocketIo.c utility/tcpSocketIo.h
	$(CC) $(CFLAG) -c utility/tcpSocketIo.c
	
byteString.o: utility/byteString.c utility/byteString.o
	$(CC) $(CFLAG) -c utility/byteString.c
	
clean:
	rm server.o logger.o tcpSocketIo.c httpStructures.o http.o byteString.o \
