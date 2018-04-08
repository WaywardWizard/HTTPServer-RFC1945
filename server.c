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
#include <regex.h>
#include <errno.h>

#include "server.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h> /* -l pthread when compiling */

typedef enum rMethod {GET, POST, HEAD} rMethod_e;

struct generalHeader {
	char* date;
	char* pragma;
};

struct requestHeader { // Request header fields
	char* authorization;
	char* from;
	char* ifModifiedSince;
	char* referrer;
	char* userAgent;
};

struct entityHeader { // Entity header fields
	char* allow;
	char* contentEncoding;
	char* contentLength;
	char* contentType;
	char* expires;
	char* lastModified;
};

struct responseHeader { // Response header fields
	char* location;
	char* server;
	char* wWWAuthenticate;
};

struct request {  // Request fields
	/* Request Line */
	char* method;
	char* uri;
	char* httpVersion;

	/* Request header fields */
	rqHeader_t *rHeader;

	/* General header fields */
	gHeader_t *gHeader;

	/* Entity header fields */
	eHeader_t *eHeader;

};

void bindSocket(int socketFd, const struct sockaddr_in *socketAddr);
int deployConcierge(int port);
char* fdReadLine(int fd);
void flushFd(int fd);
struct sockaddr_in *getSocketAddress(in_addr_t ipaddress, int port);
void mylog(char* m);
void notifyInvalidRequest();
int parseRequestLine(char* requestLine, request_t *r);
void printUsage();
request_t* readRequest(int socketFd);
void validatePort(int port);
void validateServerRoot(char* serverRoot);
void listenSocket(int socketFd, int backlog);
char* extractMatch(char* regex, char* searchString, char** destination);
request_t* initRequest();
void handleInvalidHeader();

int
main(int argc, char* argv[]){
	if (argc!=3) {
		printUsage();
	}

	char* serverRoot = argv[2];
	validateServerRoot(serverRoot);

	int port = atoi(argv[1]);
	validatePort(port);

	deployConcierge(port);
}

request_t*
initRequest(){
	request_t* r = malloc(sizeof(request_t));
	r->eHeader=NULL;
	r->gHeader=NULL;
	r->httpVersion=NULL;
	r->method=NULL;
	r->uri=NULL;
	r->rHeader=NULL;
	return(r);
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
	listenSocket(socketFd, MAX_BACKLOG);

	/* Recieve requests and hand them off to worker threads */
	int workSocket = accept(socketFd,NULL, NULL);
	while(true) {
		request_t request = *readRequest(workSocket);
		mylog("Method:\n\t");
		mylog(request.method);
		mylog("URI:\n\t");
		mylog(request.uri);
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
		mylog("Given port outside valid range [1024, 65535]");
		printUsage();
	}
}

void
validateServerRoot(char* serverRoot){
	/*Check the server root exists and the server process has read and
	 * write permissions on it. */
	int e = access(serverRoot, F_OK|R_OK|W_OK);

	if(e==0){
		return;
	} else {
		switch(errno) {
		case EACCES:
			mylog("No read permissions on server root or list permissions on parent of server root.");
			break;
		case ENOENT:
			mylog("The given server root path does not exist on the file system");
			break;
		case EROFS:
			mylog("The given server root path is read only");
			break;
		default:
			mylog("There is a problem with the given server root");
		}
		printUsage();
	}
}

void
printUsage(){
	/**
	 * Print usage instructions and terminate with a usage error code.
	 */
	fprintf(stdout, "\nUSAGE:\n");
	fprintf(stdout, "./serverExecutable serverPort documentRoot\n");
	fprintf(stdout, "\n");
	fprintf(stdout, "serverPort: Port to listen on. int in [1024, 65535] \n");
	fprintf(stdout, "documentRoot: Path so server's document root. Must");
	fprintf(stdout, " exist and be writable\n\n");
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
	struct sockaddr_in *address =malloc(sizeof(struct sockaddr_in));
	bzero(address, sizeof(*address));
	address->sin_family=AF_INET;			/* State IP Socket */
	address->sin_addr.s_addr=htonl(ipaddress);
	address->sin_port=htons(port);		/* Port in network compatible form */
	return(address);
}

void
listenSocket(int socketFd, int backlog) {
	int e = listen(socketFd, backlog);
	if(e==0){
		return;
	} else if (e==-1) {
		switch(e) {
		default:
			mylog("Could not listen on socket.");
		}
		exit(ELISTEN);
	}
}

void
bindSocket(int socketFd, const struct sockaddr_in *socketAddr) {
	int e = bind(socketFd, (struct sockaddr *) socketAddr, sizeof(struct sockaddr_in));;
	if(e==0){
		return;
	} else if (e==-1) {

		switch(e) {
		case EADDRINUSE:
			mylog("Could not bind socket. Address given already in use");
			break;
		default:
			mylog("Could not bind socket.");
		}
		exit(EBINDFAILED);
	}
}

void
flushFd(int fd){
	/**
	 * Read all reminaing content in file descriptor untill EOF hit.
	 */
	char* line;
	while((line=fdReadLine(fd))!=NULL){
		free(line);
	}
}

char*
fdReadLine(int fd) {
	/* Given a file descriptor <fd>, read a line from it in chunks. Save leftover
	 * from readline into static buffer to circumvent seeking back to endline.
	 *
	 * RETURN:
	 * 		char* line - Line read from file descriptor.
	 *
	 * 					A pseudo line is a string that was preceeded by a newline,
	 * 					but followed with an EOF instead of a newline.
	 *
	 * 					If no bytes can be read, the program will exit with an
	 * 					ESOCKETREAD error.
	 *
	 * 					If there are no lines remaining, this will return null,
	 *
	 *
	 * 					This return string should be free'd by the caller
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


	/* Buffer leftover tracking */
	static char *leftover; 					// Buffer lefover, not '\0' terminated
	static int leftoverSize;
	static int lastFd=-1;
	static int fdHasClosed=0;


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
		 * 		Mutate line, leftover, leftoverSize
		 *
		 * 		Line is always null terminated, and includes the '\n' char
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

		/* Shrink line, leave space for '\n\0'*/
		*line = realloc(*line, (nLine+1+1));
		(*line)[nLine+1]='\0';
	}

	int nLine,leftoverLength, newlineBufferOffset=0, usedLeftover=0;
	int lineSizeGrowth, splitIndex;
	int lineSize=0;
	int readFailed=0, readAttempted=0;

	/* Holds the line, includes the final '\n' & appended with '\0' on return */
	char *line=NULL;
	char* newLineLocation=NULL;				// Location of newline in buffer
	char buffer[RECVBUFFER_SIZE];
	ssize_t bytesRead;

	/* Lock function to fd */
	if((lastFd!=-1) && (fd!=lastFd)){return NULL;}
	if(lastFd==-1){lastFd=fd;}

	void unlock() {
		fdHasClosed=0;
		lastFd=-1;
		free(leftover);
		leftoverSize=0;
	}

	if(fdHasClosed){
		char* newLineLocation=memchr(leftover, '\n',leftoverSize);

		/* line is initially what is in leftover */
		line=malloc(leftoverSize+1);

		/* Line just a null byte if leftover has nothing inside it */
		if(lineSize>0){
			strncpy(line, leftover, leftoverSize);
		}

		/* Return non newline terminated leftover if fd has closed */
		if (newLineLocation==NULL){
			line[leftoverSize]='\0'; // Null terminate the line
			unlock();

		/* Leftover has newline in it - split and return line */
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

			// Retry if an error occured on reading, shortcircuit if EOF
			if(bytesRead<=0){
				(bytesRead<0)&&readFailed++;
				continue;
			}
		}

		/* Scan bufer for newline, ignoring null bytes */
		newLineLocation = memchr(buffer, '\n', bytesRead);

		/* Increse line size, and copy buffered data into line*/
		line = realloc(line, (lineSize+bytesRead));
		strncpy(line+lineSize, buffer, bytesRead); /* Ignore null bytes, whole buffer*/
		lineSize+=bytesRead;

	} while (newLineLocation == NULL && ((bytesRead==RECVBUFFER_SIZE)
											||(readFailed&&(readFailed<=MAX_READATTEMPT))
											||(usedLeftover)));

	// The fd had repeated read errors.
	if(bytesRead<0){

		mylog("The socket could not be read from");
		exit(ESOCKETREAD);

	}

	/* Assemble the line and write any buffer leftover into storage */
	if (newLineLocation!=NULL) {

		newlineBufferOffset=newLineLocation-buffer;
		leftoverSize = bytesRead-(newlineBufferOffset+1);
		extractLineCacheLeftover(&line, lineSize-leftoverSize-1, lineSize);

		/* Bytes were read (>0 since nl found) and EOF encountered */
		if ((bytesRead<RECVBUFFER_SIZE) && !(usedLeftover)){
			fdHasClosed=1;
		}

		return(line);
	}

	/* INVARIANT: A newline was not found, but a read was attempted & returned
	 * less than the number of bytes in our buffer and hence has reached EOF.
	 * This is an invariant because the reading loop will not exit until such*/

	// Put any remainder into the leftover (leave off the null byte)
	fdHasClosed=1;
	if(lineSize>0){
		// Put the line in leftover
		leftover=malloc(lineSize);
		strncpy(leftover, line, lineSize);

		// Yield a line from whatever is left over
		return(fdReadLine(fd));
	}

	/* The read was empty, and there was no leftover. The fd has been cleared
	 * of it's lines, and its final pseudo line. Now return null */
	unlock();
	return(NULL);
}

request_t
*readRequest(int socketFd) {
	/**
	 * Read a request from a connected socket & return a request structure
	 */
	request_t* r=initRequest();
	char* requestLine = fdReadLine(socketFd);
	fdReadLine(socketFd);

	if(requestLine==NULL){
		notifyInvalidRequest();
	}

	parseRequestLine(requestLine, r);
	return(r);
}


void
notifyInvalidRequest() {
	mylog("Request invalid.");
	exit(1);
}

void
mylog(char* m) {
	printf("%s\n",m);
}

int
parseRequestLine(char* requestLine, request_t *r) {
	/**
	 * Load http1.0 request line into request structure. If the request is
	 * malformed, return EINVALID_REQUEST and write null bytes into request struct
	 */

	/* Extract method */
	requestLine = extractMatch(METHOD_REGEX, requestLine, &(r->method));
	if (requestLine==NULL){
		return(EINVALID_REQUEST);
	}

	/* Extract request URI */
	extractMatch(URI_REGEX, requestLine, &(r->uri));
	return(0);
}

char*
extractMatch(char* regex, char* searchString, char** destination) {
	/**
	 * Search <string> for match with <regex> Write match into match
	 * destination.
	 *
	 * Terminates if an error occured
	 *
	 * RETURN:
	 * 		NULL if no match.
	 * 		Else, pointer to remainder of search string.
	 */
	regex_t rx;
	regmatch_t match;
	int errSize=100; // Default error message size
	int error = regcomp(&rx, regex, REG_EXTENDED);
	int matchSize;

	/* Check compilation sucessful */
	if(error!=0){
		char errorMessage[errSize];
		regerror(error,&rx,errorMessage,errSize);
		mylog("Regex compilation error: \n");
		mylog("\t");
		mylog(errorMessage);
		exit(EREGCOMP);
	}

	/* Match */
	error = regexec(&rx,searchString, 1, &match, 0);
	regfree(&rx);

	/* Allocate destination memory & write match into destination */
	if (error!=REG_NOMATCH) {
		matchSize = match.rm_eo-match.rm_so;
		*destination = malloc(matchSize+1);
		strncpy(*destination, searchString+match.rm_so, matchSize);
		(*destination)[matchSize]='\0';
		return(searchString+match.rm_eo);
	}

	/* No match found */
	return(NULL);
}

void
handleInvalidHeader(){
	mylog("Header was invalid. Worker thread closing\n");
}
