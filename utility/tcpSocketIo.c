/*
 * tcpSocketIo.c
 *
 *  Created on: 11 Apr. 2018
 *      Author: ben
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#include "tcpSocketIo.h"
#include "bool.h"
#include "logger.h"
#include "byteString.h"
#include "filesystem.h"


/* Setting up listening sockets */
void _bindSocket(int socketFd, const struct sockaddr_in *socketAddr);
struct sockaddr_in *_getSocketAddress(in_addr_t ipaddress, int port);
void _listenSocket(int socketFd, int backlog);

/* Per thread buffers & fd locking to prevent lefover cross contamination */
void _moduleInit();
int *_getFdPointer();
int _setFd(int fd);
int _isLocked(int fd);
byteString_t* _getFdBuffer();
int _setFdBuffer(char* string, int len);
void _unsetFdBuffer();
int _sendByte(int socketFd, char* bytes, int length);
void _sliceByteStringCacheLeftover(byteString_t *b, int sliceIndex);

void _handleSendError();

static pthread_key_t tFd;
static pthread_key_t tBuf;


void _moduleInit() {
	/* Initiliaze thread storage keys. Run once only */
	static int initialized=false;
	if(!initialized) {
		initialized=true;
		pthread_key_create(&tFd, free);
		pthread_key_create(&tBuf, bsDestruct);
	}
}


int* _getFdPointer() {
	/**
	 * Returns the file descriptor the thread is currently locked to. If
	 * this is null, then the thread is not locked.
	 *
	 * If fd is not null, then _getFdBuffer() will return a non empty string.
	 */
	return(pthread_getspecific(tFd));
}


int _setFd(int fd) {
	/**
	 * Locks the executing thread to the given fd
	 *
	 * Return a non zero value if setting failed
	 */
	free(_getFdPointer());
	int* threadValue = malloc(sizeof(int));
	*threadValue=fd;
	return(pthread_setspecific(tFd, (void*)threadValue));
}


void _unsetFd() {
	free(_getFdPointer());
	pthread_setspecific(tFd, NULL);
}


int _isLocked(int fd) {
	/**
	 * Determines if the thread is locked to a different fd than the one
	 * given.
	 *
	 * RETURN:
	 * 		true - the module is locked for the calling thread and cant be used
	 * 		with the given fd
	 *
	 * 		false - the module is not locked and can be used
	 */

	/* Initialize thread storage used for locking if required */
	_moduleInit();

	int *currentFd = (int*)pthread_getspecific(tFd);
	if ((currentFd!=NULL)&&(*currentFd!=fd)) {
		return(true);
	}
	return(false);
}


void _unlock(){
	/**
	 * Discard fd buffer for calling thread and unlock it for use with
	 * other buffers */
	_unsetFd();
	_unsetFdBuffer();
}


void flushFdBuffer() {
	_unlock();
}


byteString_t *_getFdBuffer() {
	/**
	 * Retrieves the string located in the threads buffer leftover
	 */
	return(pthread_getspecific(tBuf));
}


int _setFdBuffer(char*  byteString, int len) {
	/**
	 * Replace existing buffer with new <bytestring> of <len> Free prior buffer.
	 *
	 * ARGUMENT:
	 * 		byteString - byte string to copy into buffer
	 * 		len - number of bytes to copy
	 *
	 * RETURN:
	 * 		Set bytestring to NULL if zero length. Else creare bytestring_t
	 *
	 */
	byteString_t *fdBuffer=bsInit();
	if (len>0) {
		bsWrite(fdBuffer, byteString, len);
	} else {
		fdBuffer=NULL;
	}

	/* Free the old fdBuffer & set the new one */
	bsFree(_getFdBuffer());
	return(pthread_setspecific(tBuf, (void*)fdBuffer));
}


void _unsetFdBuffer() {
	/**
	 * Set the fdBuffer to null (& implicitly its length to zero)
	 */
	bsFree(_getFdBuffer());
	pthread_setspecific(tBuf, NULL);
}


int _getFdBufferLength() {
	/** Get the length of the fdBuffer. defined as zero if there is no fdBuffer*/
	byteString_t *b = _getFdBuffer();
	if (b==NULL) {return 0;} else {return b->length;}
}


void _bindSocket(int socketFd, const struct sockaddr_in *socketAddr) {
	/**
	 * Given an address and socket fd - bind the two. Print error messages on
	 * failure and exit the program
	 */
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


void _listenSocket(int socketFd, int backlog) {
	/**
	 * Given a socket and backlog, execute primitive to listen for connections
	 * on error - print error and exit.
	 */
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


struct sockaddr_in *_getSocketAddress(in_addr_t ipaddress, int port){
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


int getListeningSocket(int port) {
	/**
	 * return a fd for a tcp socket listening on any ip address, and the
	 * given port
	 */

	/* TCP socket atop IP network layer */
	int socketFd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in *socketAddr = _getSocketAddress(INADDR_ANY, port);
	_bindSocket(socketFd, socketAddr);
	_listenSocket(socketFd, MAX_BACKLOG);
	return(socketFd);
}


byteString_t *fdReadBytes(int fd, int byteCount){
	/* Get <byteCount> bytes from the file descriptor and return the resulting
	 * byte string
	 */

	/* Do nothing if locked */
	if (_isLocked(fd)||byteCount<=0) {return NULL;}

	byteString_t *line;
	int leftoverSize=_getFdBufferLength();
	int fdBytesToRead = byteCount;


	/* Use up leftover from prior calls for the fd (if there is any) */
	if(leftoverSize>0) {

		line = bsCopy(_getFdBuffer());

		/* Leftover has enough bytes to satisfy the request*/
		if(byteCount<=leftoverSize){

			/* Get byteCount bytes from leftover into line */
			_sliceByteStringCacheLeftover(line, byteCount);

			return(line);

		/* Leftover partially satisfies the request */
		} else {
			fdBytesToRead-=leftoverSize;
			bsFree(_getFdBuffer());
		}
	}

	/* Read remaining bytes required to satisfy the request from the file desc*/
	char* readBuffer=malloc(fdBytesToRead);
	if(read(fd, readBuffer, fdBytesToRead)!=fdBytesToRead) {
		/* This should not happen for regular files as per the man page */
		mylog("Could not read enough bytes from file descriptor");
		return(NULL);
	}

	bsAppend(line, readBuffer, fdBytesToRead);
	free(readBuffer);
	return(line);
}


void _sliceByteStringCacheLeftover(byteString_t *b, int sliceIndex) {
	/**
	 * Given a byteString & a slice index slice the string from the beginning to
	 * the slice index. Dont include the slice indexth element of the byte string.
	 *
	 * Shrink the byte string to its new size and save any remainder in the
	 * leftover buffer for the thread
	 *
	 * ARGUMENT:
	 * 		byteString_t b - string to slice
	 * 		int sliceIndex
	 *
	 * 	RETURN:
	 * 		Shrink byteString. Save leftover into TLS (thread local storage)
	 *
	 * 	NOTE:
	 * 		Null bytes have no special significance in this context
	 */

	/* Check the byte string is big enough to slice */
	int byteStringLength=b->length;
	if (sliceIndex>byteStringLength) {
		return;
	}

	/* Save the slice remainder*/
	int sliceLen = sliceIndex;
	char* byteString=b->string;
	int leftoverLen=byteStringLength-sliceLen;
	if (leftoverLen!=0) {
		_setFdBuffer(byteString+sliceIndex, leftoverLen);
	}

	/* Shrink byteString to slice size */
	bsShrink(b, sliceIndex);
}


char* fdReadLine(int fd) {
	/* Given a file descriptor <fd>, read and return a null terminated line from
	 * it.
	 *
	 * Save any excess read bytes into a buffer for subsequent calls
	 *
	 * ARGUMENT:
	 * 		fd - file descriptor to read from (assumed non seekable)
	 *
	 * RETURN:
	 * 		char* line -
	 * 					Newline demarcated Line read from file descriptor.
	 *
	 * 					If no bytes can be read, the program will exit with an
	 * 					ESOCKETREAD error.
	 *
	 * 					If there are no lines remaining, return null,
	 *
	 * 					!!This return string should be FREEd by the caller
	 *
	 * NOTE:
	 * 		Once called, this function will be locked to the given file
	 * 		descriptor untill it closes. That is, if it is subsequently called
	 * 		with another file descriptor it will always return null. The reason
	 * 		for this is to prevent buffer leftover cross contaminating streams.
	 *
	 * 		This function will be unlocked (callable with different fd's) once
	 * 		the leftover is empty or the unlock() function is called for the
	 * 		module by a thread. This unlocks the module for that thread
	 *
	 * 		Since each thread will have its own TLS for this function -
	 * 		each thread can work with one file descriptor at once
	 *
	 */

	if (_isLocked(fd)) {return NULL;}

	ssize_t bytesRead;
	byteString_t* line=bsInit();
	byteString_t* leftover=NULL;
	char* returnLine;
	int lineLength;
	int newLineIx;
	char* newLineLocation;
	int leftoverSize;
	int readFailCount=0;
	char buffer[BUFFER];


	/* Helper function */
	char* __convertBsToString(byteString_t *b) {
		char* returnLine=malloc(b->length+1);
		memcpy(returnLine, b->string, b->length);
		returnLine[b->length]='\0';
		return(returnLine);
	}


	/* Use leftover buffer before reading from the fd */
	if (_getFdBufferLength()>0){

		leftover = bsCopy(_getFdBuffer());
		newLineLocation=memchr(leftover->string, '\n', leftover->length);

		/* Line found */
		if (newLineLocation!=NULL) {

			/* Extract line */
			newLineIx=newLineLocation-leftover->string;
			lineLength=newLineIx+1;
			_sliceByteStringCacheLeftover(leftover, lineLength);

			returnLine=__convertBsToString(leftover);
			bsFree(leftover);
			return(returnLine);

		/* Line not found. Empty leftover buffer and unlock (since we have used it)*/
		} else {
			_unlock();
		}

		bsFree(line);
		line=bsCopy(leftover);
		bsFree(leftover);
	}

	/* Read from fd untill a newline is found (or the socket is emptied) */
	do{
		bytesRead = read(fd, buffer, BUFFER);

		// Retry if an error occured on reading, shortcircuit if EOF
		if(bytesRead<=0){
			(bytesRead<0)&&readFailCount++;
			continue;
		}

		/* Scan bufer for newline, ignoring null bytes */
		newLineLocation = memchr(buffer, '\n', bytesRead);

		bsAppend(line, buffer, bytesRead);

	} while (newLineLocation== NULL && ((bytesRead==BUFFER)
											||readFailCount<=READ_REATTEMPT
											||bytesRead==0));


	// The fd had repeated read errors.
	if(bytesRead<0){

		mylog("The socket could not be read from");
		exit(ESOCKETREAD);
	}

	/* Line found. Assemble and return */
	if (newLineLocation!=NULL) {

		newLineIx=newLineLocation-buffer;
		_sliceByteStringCacheLeftover(line, newLineIx+1);
		returnLine=__convertBsToString(line);
		bsFree(line);
		return(returnLine);

	/* Nothing found, so just cache the leftover */
	} else {
		if(line->length>0){
			_sliceByteStringCacheLeftover(line,0);
			bsFree(line);
		} else {
			/* Nothing in buffer */
			_unlock();
		}
		return(NULL);
	}
}


void closeSocket(int s) {
	close(s);
}


int sendString(int socketFd, char* s, char* c) {
	/**
	 * Send a string into the socket.
	 *
	 * ARGUMENT:
	 * 	socketFd - socket to send string through
	 * 	s - string to send (null byte terminated)
	 * 	c - a character to append to the transmission. If null, dont append anything.
	 *
	 * RETURN:
	 * 	SENDOK if sending was sucessful. ESEND if sending failed or partially
	 * 	failed
	 *
	 * ASSUMPTION:
	 * 	That <s> is null byte terminated.
	 */

	int length = strlen(s);
	/* Send string */
	if(_sendByte(socketFd, s, length)==ESEND) {
		return(ESEND);
	}

	/* Trailing char if specified */
	if(c!=NULL) {
		if(sendChar(socketFd,c)==ESEND) {
			return(ESEND);
		}
	}

	return(SENDOK);
}


int _sendByte(int socketFd, char* bytes, int length) {
	/**
	 * Send <length> bytes of bytestream <bytes> through <socketFd>
	 *
	 * If sending fails,
	 */
	int sentCount=0;
	int sent;
	int sendLength=length;
	while (sentCount!=length) {
		sent = send(socketFd, bytes+sentCount, sendLength, 0);
		if (sent==-1) {
			_handleSendError();
			return(ESEND);
		}
		sentCount+=sent;
		sendLength-=sent;
	}
	return(SENDOK);
}


void _handleSendError(){
	/**
	 * Handle a send error with a message and program termination
	 */
	switch(errno){
		case(EBADF):
			mylog("Send attempted with invalid fd");
			break;
		case(ECONNRESET):
			mylog("Peer reset connection during send");
			break;
		case(ENOTCONN):
			mylog("Send attempted with non-connected socket");
			break;
		default:
			mylog("An error occured while sending data");
	}
}


int sendChar(int socketFd, char* s) {
	/**
	 * Send a single character <s> throught <socketFd>
	 */
	if(s!=NULL){
		return(_sendByte(socketFd, s, 1));
	}
	return(SENDOK);
}


int sendFile(int socketFd, FILE *f, long fSize) {
	/**
	 * Send a binary file through the network.
	 *
	 * ARGUMENT
	 * 	socketFd - socket to send via
	 * 	f - file to send.
	 * 	fSize - size of file in bytes
	 *
	 * ASSUMPTION
	 * 	f is @ zero offset, or else only part of the file will be sent
	 */
	char buffer[SENDBUFFER];
	size_t nRead;

	/* Send groups of bytes until the EOF is reached */
	while(feof(f)==0){

		/* Handle file read errors if present */
		nRead=fread(buffer, SENDBUFFER, 1, f);
		if (ferror(f)!=0) {
			handleFileReadError();
			return(ESEND);
		}

		if(_sendByte(socketFd, buffer, nRead*SENDBUFFER)==ESEND) {
			return(ESEND);
		}
		fSize-=(nRead*SENDBUFFER);
	}

	/* Send remainder of file */
	return(_sendByte(socketFd, buffer, fSize));
}

