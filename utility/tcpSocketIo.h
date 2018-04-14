/*
 * tcpSocketIo.h
 *
 *  Created on: 11 Apr. 2018
 *      Author: ben
 */
#ifndef UTILITY_TCPSOCKETIO_H_
#define UTILITY_TCPSOCKETIO_H_

#include "byteString.h" // fdReadBytes returns a bytestring


#define BUFFER		  256			// Read buffer [bytes]
#define SENDBUFFER	  1024			// Send buffer [bytes]
#define READ_REATTEMPT 3
#define MAX_BACKLOG  64				// Max listen backlog


int getListeningSocket(int port);			// Get a TCP/IP listening socket
char* fdReadLine(int fd);			// Read a line
byteString_t *fdReadBytes(int fd, int byteCount); // read set amt of bytes
void flushFdBuffer();				// Unlock module for use with other fd's
void closeSocket(int s);

void sendString(int socketFd, char* s, char* c);
void sendChar(int socketFd, char* s);
void sendFile(int socketFd, FILE *f);


#define EBINDFAILED	  7
#define ESOCKETREAD	  17 // Could not read from socket
#define ELISTEN		  11
#define EREADDEFICIT	  99 // fd did not have as many bytes as requested
#define ESEND 		  101

#endif
