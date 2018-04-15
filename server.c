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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <pthread.h> /* -l pthread when compiling */
#include <semaphore.h>


#include "utility/bool.h"
#include "server.h"
#include "utility/tcpSocketIo.h"
#include "http/http.h"
#include "utility/logger.h"
#include "./utility/filesystem.h"


#define SEMAPHORE_SHARE_THREADS 0 // As per man sem_init
sem_t threadQuota; // This indicates how many more threads we can create


typedef struct docrootSocketPair {
	int socket;    // socket fd connected to client
	char* docroot; // null term string path to server root dir
} dsPair_t;


void deployConcierge(int port, char* serverRoot);
void printUsage();
void validatePort(int port);
void validateServerRoot(char* serverRoot);
void stripTrailingSlash(char** path);
void stripTrailingChar(char** path,char c);
dsPair_t* initDsPair(int socket, char* dRoot);
void freeDsPair(dsPair_t* d);
void waitForThreadAvailable();
void* threadProcessRequest(void* dsPair);

dsPair_t* initDsPair(int socket, char* dRoot) {
	dsPair_t *dsPair=malloc(sizeof(dsPair_t));
	dsPair->socket=socket;
	dsPair->docroot=malloc(strlen(dRoot)+1);
	strcpy(dsPair->docroot, dRoot);
	return(dsPair);
}

void freeDsPair(dsPair_t* d){
	free(d->docroot);
}

int
main(int argc, char* argv[]){

	if (argc!=3) {
		printUsage();
	}

	sem_init(&threadQuota, SEMAPHORE_SHARE_THREADS, MAXTHREAD);

	/* Check server root valid, remove any trailing slash */
	char* serverRoot = strdup(argv[2]);
	stripTrailingSlash(&serverRoot);
	validateServerRoot(serverRoot);

	int port = atoi(argv[1]);
	validatePort(port);

	mylog("Deploying concierge");
	deployConcierge(port, serverRoot);
}

void stripTrailingSlash(char** path){stripTrailingChar(path, '/');}
void
stripTrailingChar(char** path,char c) {
	/**
	 * Remove a trailing slash from a string if it exists.
	 *
	 * ASSUMPTION:
	 * 	*path is null terminated
	 */
	int l = strlen(*path);
	if ((*path)[l-1]==c){
		(*path)[l-1]='\0';
		*path = realloc(*path, l-1);
	}
}

void
deployConcierge(int port, char* serverRoot){
	/**
	 * Deploy concierge to hand off all incoming connections.
	 *
	 * RETURN:
	 * 		integer file descriptor for soc
	 */

	int socketFd=getListeningSocket(port);
	int workSocket;
	pthread_t thread;

	/* Recieve requests and hand them off to worker threads */
	while(true) {

		/* Accept connection */
		workSocket = accept(socketFd, NULL, NULL);
		dsPair_t* d=initDsPair(workSocket, serverRoot);

		/* Block until there are we are below the thread limit*/
		waitForThreadAvailable();

		/* The thread cleanup handler will close the socket & free the dsPair*/
		pthread_create(&thread, NULL, threadProcessRequest, (void*)d);
	}

}

void waitForThreadAvailable() {

	/* Wait untill there is a thread available */
	sem_wait(&threadQuota);
	return;
}

void* threadProcessRequest(void* dsPair) {
	/**
	 * Handle a single http request as a separate thread.
	 *
	 * ARGUMENT:
	 *  dsPair_t dsPair - Socket and path to root directory. The socket shall
	 *  be closed before the thread exits, and all of the memory allocated to
	 *  dsPair freed
	 *
	 * RETURN:
	 * 	Process request
	 */

	/* Unpack arguments, to be freed when cleaning up */
	dsPair_t* pathSocket=(dsPair_t*)dsPair;
	int socketFd=pathSocket->socket;
	char* docRoot=pathSocket->docroot;

	/* Process & reply to http request */
	processRequest(socketFd, docRoot);

	/* Close up the socket and free argument structure */
	freeDsPair((dsPair_t*)dsPair);
	free(dsPair);
	closeSocket(socketFd);

	/* Increment the available number of threads*/
	sem_post(&threadQuota);
	pthread_exit(NULL);
}

void validatePort(int port) {
	/**
	 * Check the port given is within the valid range of addresses [1024, 65535]
	 *
	 * Print usage and exit if invalid
	 */
	if (!(port>1023) || !(port<=65535)) {
		mylog("Given port outside valid range [1024, 65535]");
		printUsage();
	}
}

void
validateServerRoot(char* serverRoot){
	/*Check the server root exists and the server process has read and
	 * write permissions on it. */
	if(testFile(serverRoot, (F_OK|R_OK))!=true) {
		printUsage();
	}
}

void printUsage(){
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
