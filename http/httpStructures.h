/*
 * httpStructures.h
 *
 *  Created on: 11 Apr. 2018
 *      Author: ben
 */
#ifndef HTTP_HTTPSTRUCTURES_H_
#define HTTP_HTTPSTRUCTURES_H_

typedef struct generalHeader gHeader_t;
typedef struct requestHeader rqHeader_t;
typedef struct entityHeader eHeader_t;
typedef struct responseHeader rsHeader_t;
typedef struct request request_t;
typedef struct response response_t;
typedef struct httpStatus status_t;

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
	long contentLength;
	char* contentType;
	char* expires;
	char* lastModified;
};

struct responseHeader { // Response header fields
	char* location;
	char* server;
	char* wWWAuthenticate;
};

struct httpStatus {
	char* code;
	char* phrase;
};

struct request {  // Request fields
	/* Request Line */
	char* method;
	char* uri;
	char* httpVersion;

	/* Request header fields */
	rqHeader_t *rqHeader;

	/* General header fields */
	gHeader_t *gHeader;

	/* Entity header fields */
	eHeader_t *eHeader;

};

struct response {
	char* httpVersion;
	char* entityPath;
	status_t *status;
	gHeader_t *gHeader;
	rsHeader_t *rsHeader;
	eHeader_t *eHeader;
};

request_t* initRequest();
response_t* initResponse();
void freeRequest(request_t* r);
void freeResponse(response_t* r);

#endif
