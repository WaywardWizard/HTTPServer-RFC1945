/* HTTP1.0 Server
 * Author: 			Ben Tomlin
 * Student Nbr:		834198
 * Date:			Apr 2018
 *
 * This program aims to implement an HTTP1.0 server as defined by RFC1945
 *
 * Requirements
 * 	GET request response
 * 	Valid response
 * 	.html, .jpg, .css, .js
 * 	Multiple requests with pthread
 *
 * 	args:
 * 		./server port rootpath
 * 		path to root web
 * 		port
 *
 * 	response:
 *	 	200, 404 response
 *	 	http status
 *	 	content type
 *
 * 	preliminary understanding:
 *
 * 	shuttle
 * 		make socket.
 * 		bind to a port.
 * 		listen on socket.
 * 		recieve request
 * 		negotiate new socket.
 * 		hand off to resource retrieval
 *
 * 	fetcher
 * 		negotiate socket
 * 		recieve http request
 * 		fetch resource
 * 		send response
 *
 *
 *
 */

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h> /* -l pthread when compiling */

#define MAX_BACKLOG  1024				// Max listen backlog
#define   RECVBUFFER_SIZE  4096
#define	 MAX_READATTEMPT  5				// Max consecutive read failures allowed

#define true  1
#define false 0

#define  EBINDFAILED	  7
#define 	EUSAGE 		  5
#define  ELISTEN		  11
#define  EEOF	      13 // At end of file, cant read any line
#define  ESOCKETREAD	  17 // Could not read from socket after repeated attempts

typedef enum rMethod {GET, POST, head} rMethod_e;

typedef struct generalHeader {
	char* date;
	char* pragma;
} gHeader_t;

typedef struct requestHeader { // Request header fields
	char* authorization;
	char* from;
	char* ifModifiedSince;
	char* referrer;
	char* userAgent;
} rqHeader_t;

typedef struct entityHeader { // Entity header fields
	char* allow;
	char* contentEncoding;
	char* contentLength;
	char* contentType;
	char* expires;
	char* lastModified;
} eHeader_t;

typedef struct responseHeader { // Response header fields
	char* location;
	char* server;
	char* wWWAuthenticate;
} rsHeader_t;

typedef struct request {  // Request fields
	/* Request Line */
	rMethod_e method;
	char* uri;
	char* httpVersion;

	/* Request header fields */
	rqHeader_t *rHeader;

	/* General header fields */
	gHeader_t *gHeader;

	/* Entity header fields */
	eHeader_t *eHeader;

} request_t;

int
main(int argc, char* argv[]){
	if (argc!=2)
		printUsage();

	char* serverRoot = argv[1];
	validateServerRoot(serverRoot);

	int port = atoi(argv[2]);
	validatePort(port);

	deployConcierge(port);
}

int
deployConcierge(int port){
	/**
	 * Deploy concierge to hand off all incoming connections.
	 *
	 * RETURN:
	 * 		integer file descriptor for soc
	 */

	/* TCP socket atop IP network layer */
	int socketFd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in *socketAddr = getSocketAddress(INADDR_ANY, port);
	bindSocket(socketFd, socketAddr);
	int e = listen(socketFd, MAX_BACKLOG);

	/* Assert listen primitive was sucessful */
	if(e!=0) {
		log("Listen primitive failed");
		exit(ELISTEN);
	}

	/* Recieve requests and hand them off to worker threads */
	while(true) {
		workSocket = accept(socketFd,NULL, NULL);
		request_t request = readRequest(workSocket);
	}


}

void
validatePort(int port) {
	/**
	 * Check the port given is within the valid range of addresses [1024, 65535]
	 *
	 * Print usage and exit if invalid
	 */
	if (!(port>1023) || !(port<=65535)){
		printUsage();
	}
	log("Given port outside valid range [1024, 65535]");
	printUsage();
}

void
validateServerRoot(char* serverRoot){
	/*Check the server root exists and the server process has read and
	 * write permissions on it. */
	int e = access(serverRoot, F_OK|R_OK|W_OK);

	if(e==0){
		return;
	}

	if(e==EACCES){
		log("No read permissions on server root or list permissions on parent of server root.");
	} else if (e==ENOENT) {
		log("The given server root path does not exist on the file system");
	} else if (e==EROFS) {
		log("The given server root path is read only");
	} else {
		log("There is a problem with the given server root");
	}
	printUsage();
}

void
printUsage(){
	/**
	 * Print usage instructions and terminate with a usage error code.
	 */
	fprintf(stdout, "USAGE:\n");
	fprintf(stdout, "./serverExecutable serverPort documentRoot\n");
	fprintf(stdout, "\n");
	fprintf(stdout, "serverPort: Port to listen on. int in [1024, 65535] \n");
	fprintf(stdout, "documentRoot: Path so server's document root. Must \n");
	fprintf(stdout, "			   exist and be writable");
	exit(EUSAGE);
}

struct sockaddr_in
*getSocketAddress(in_addr_t ipaddress, int port){
	/***
	 * Return a TCP/IP socket address at <ipaddress>:<port>
	 *
	 * ARGUMENT:
	 * 		ipaddress - a single ip addr or range of addr which the address could
	 * 		be bound to.
	 *
	 * 		port - port to bind to
	 */
	struct sockaddr_in address;
	bzero(address, sizeof(address));
	address.sin_family=AF_INET;			/* State IP Socket */
	address.sin_addr.s_addr=htonl(ipaddress);
	address.sin_port=htons(port);		/* Port in network compatible form */
	return(&address);
}

bindSocket(int socketFd, const struct sockaddr_in *socketAddr) {
	int e = bind(socketFd, (struct sockaddr *) socketAddr, sizeof(struct sockaddr_in));;
	if(e==0){return;}

	switch(e) {
	case EADDRINUSE:
		log("Could not bind socket. Address given already in use");
		break;
	}
	exit(EBINDFAILED);
}

char*
fdReadLine(int fd) {
	/* Given a file descriptor <fd>, read a line from it in chunks. Save leftover
	 * from readline into static buffer to circumvent seeking back to endline.
	 *
	 * RETURN:
	 * 		char* line - Line read from file descriptor.
	 *
	 * 					If no bytes can be read, the program will exit.
	 *
	 * 					If there are no lines remaining, this will return null,
	 * 					and on a second call, return the pseudo-line and unlock
	 * 					for another file descriptor.
	 *
	 * 					A pseudo line is a string that was preceeded by a newline,
	 * 					but followed with an EOF instead of a newline.
	 *
	 * NOTE:
	 * 		Once called, this function will be locked to the given file
	 * 		descriptor untill it closes. That is, if it is subsequently called
	 * 		with another file descriptor it will always return null. The reason
	 * 		for this is to prevent buffer leftover cross contaminating streams.
	 *
	 * 		This function will be unlocked (callable with different fd's) once
	 * 		it seeks it's locked fd to it's EOF AND if there are any lines
	 * 		not followed by a new line, it must be called twice to return its
	 * 		final pseudo-line.
	 */

	int nLine,leftoverLength, newlineBufferOffset=0, usedLeftover=0;
	int lineSizeGrowth, splitIndex;
	int lineSize=1; 						// Always space for a null byte
	int readFailed=0, readAttempted=0;

	/* Buffer leftover tracking */
	static char *leftover; 					// Buffer lefover
	static int leftoverSize;
	static int lastFd=NULL;
	static int fdHasClosed=0;

	char* newLineLocation=NULL;				// Location of newline in buffer
	char *line=NULL; 						// This holds our line
	char buffer[1+RECVBUFFER_SIZE];
	ssize_t bytesRead;

	/* Lock function to fd */
	if((lastFd!=NULL) && (fd!=lastFd)){return NULL;}
	if(lastFd==NULL){lastFd=fd;}

	void unlock() {
		fdHasClosed=0;
		lastFd=NULL;
		free(leftover);
		leftoverSize=0;
	}

	/* Return non newline terminated leftover if fd has closed */
	if(fdHasClosed){
		char* newLineLocation=strnchr(leftover, '\n',leftoverSize);

		/* line is initially what is in leftover */
		line=malloc(leftoverSize+1);
		strncpy(line, leftover,leftoverSize);
		line[leftoverSize]='\0';

		/* Leftover has no newline in it - return the leftover */
		if (newLineLocation==NULL){
			unlock();
		} else {
			splitIndex=newLineLocation-leftover;
			extractLineCacheLeftover(&line, splitIndex, leftoverSize);
		}
		return(line);
	}

	/* Repeated read buffer untill a newline is found or run out of data */
	do {

		/* Use the cached leftover as inital read */
		usedLeftover=0;
		if (leftover!=NULL) {

			/* Leftover has a null byte */
			strncpy(buffer, leftover, leftoverSize);
			bytesRead = leftoverSize;
			usedLeftover = 1;

			/* Free leftover */
			free(leftover);
			leftoverSize=0;
			leftover=NULL;

		/* Read from fd */
		} else {

			bytesRead = read(fd, buffer, (RECVBUFFER_SIZE));
			readAttempted=1;

			// Retry if an error occured on reading, shortcircuit if EOF
			if(bytesRead<=0){
				(bytesRead<0)&&readFailed++;
				continue;
			}
		}
		buffer[bytesRead]='\0';
		newLineLocation = strchr(buffer, '\n');

		/* Increse line size, and copy buffered data into line*/
		line = realloc(line, (lineSize+bytesRead));
		strcpy(line[lineSize-1], buffer); // Overwrite old '\0'
		lineSize+=bytesRead;

	} while (newLineLocation == NULL && ((bytesRead==RECVBUFFER_SIZE)
											||(readFailed&&(readFailed<=MAX_READATTEMPT))
											||(usedLeftover)));

	// The fd had repeated read errors.
	if(bytesRead<0){

		log("The socket could not be read from");
		exit(ESOCKETREAD);

	}

	/* Assemble the line and write any buffer leftover into storage */
	if (newLineLocation!=NULL) {

		newlineBufferOffset=newLineLocation-buffer;
		leftoverSize = bytesRead-(newlineBufferOffset+1);
		extractLineCacheLeftover(line, lineSize-leftoverSize-1, lineSize);

		/* Bytes were read (>0 since nl found) and EOF encountered */
		if (bytesRead<RECVBUFFER_SIZE) {
			fdHasClosed=1;
		}

		return(line);
	}

	/* Hereon, a newline was not found, but a read was attempted */

	// Put line into the leftover - leave off the null byte
	fdHasClosed=1;
	realloc(leftover, lineSize-1); // free leftover if nothing leftover
	strncpy(leftover, line, lineSize-1);
	return(NULL); // If the function is called again, the leftover will be returned.

	// Read suceeded. EOF found y/n
	void extractLineCacheLeftover(char** line, int nLine, int length) {
		/**
		 * Given a string & a split index trim the string to the size of the
		 * split index (inclusive of split index), overwrite the character at
		 * split index with a null byte and save the remainder in leftover.
		 *
		 * ARGUMENT:
		 * 		char** line - address of string to split
		 * 		int nLine   - split index
		 * 		int length	- the string length
		 *
		 * 	RETURN:
		 * 		Mutate string, leftover, leftoverSize
		 *
		 * 	NOTE:
		 * 		Leftover saved as is, with no null byte.
		 *
		 */

		/* Save leftover */
		leftoverSize=length-(nLine+1);
		if(leftoverSize>0){
			strncpy(leftover, line[nLine+1], leftoverSize);
		}

		/* Shrink line */
		*line = realloc(*line, (nLine+1));
		(*line)[nLine]='\0'; // Overwrite with null byte
	}
}

request_t readRequest(int socketFd) {
	/**
	 * Read a request from a connected socket & return a request structure
	 */
	char recieveBuffer[RECVBUFFER_SIZE];



}







