#ifndef SERVER_H_
#define SERVER_H_

#define RECVBUFFER_SIZE  4096
#define MAX_READATTEMPT  5				// Max consecutive read failures allowed
#define MAXTHREAD 8

#define EUSAGE 		  5
#define EEOF	      13 // At end of file, cant read any line
#define EREGCOMP	  23
#define EHEADINVALID  27 // Header was invalid
#define EPATH_INVALID 29

#endif
