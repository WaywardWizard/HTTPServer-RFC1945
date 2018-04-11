/*
 * httpStructures.c
 *
 *  Created on: 11 Apr. 2018
 *      Author: ben
 */

#include "httpStructures.h"
#include <stdlib.h>

gHeader_t* initEHeader();
gHeader_t* initGHeader();
rsHeader_t* initRsHeader();
rqHeader_t* initRqHeader();
status_t* initHttpStatus();

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

struct httpStatus {
	char* statusCode;
	char* statusPhrase;
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
	httpStatus_t *status;
	gHeader_t *gHeader;
	rHeader_t *rHeader;
	eHeader_t *eHeader;
};

request_t*
initRequest(){
	request_t* r = malloc(sizeof(request_t));
	r->eHeader=initEHeader();
	r->gHeader=initGHeader();
	r->httpVersion=NULL;
	r->method=NULL;
	r->uri=NULL;
	r->rqHeader=initRqHeader();
	return(r);
}

response_t *
initResponse() {
	response_t* r = malloc(sizeof(response_t));
	r->httpVersion=NULL;
	r->status=initHttpStatus();
	r->gHeader=initGHeader();
	r->rHeader=initRqHeader();
	r->eHeader=initEHeader();
}

status_t*
initHttpStatus(){
	status_t* s=malloc(sizeof(status_t));
	s->statusCode=NULL;
	s->statusPhrase=NULL;
	return(s);
}

rqHeader_t*
initRqHeader() {
	rqHeader_t* h=malloc(sizeof(rqHeader_t));
	h->authorization=NULL;
	h->from=NULL;
	h->ifModifiedSince=NULL;
	h->referrer=NULL;
	h->userAgent=NULL;
}

rsHeader_t*
initRsHeader() {
	rsHeader_t* h=malloc(sizeof(rsHeader_t));
	h->location=NULL;
	h->server=NULL;
	h->wWWAuthenticate=NULL;
}

gHeader_t*
initGHeader() {
	gHeader_t* h=malloc(sizeof(gHeader_t));
	h->date=NULL;
	h->pragma=NULL;
}

eHeader_t*
initEHeader() {
	eHeader_t* h=malloc(sizeof(eHeader_t));
	h->allow=NULL;
	h->contentEncoding=NULL;
	h->contentLength=NULL;
	h->contentType=NULL;
	h->expires=NULL;
	h->lastModified=NULL;
}
