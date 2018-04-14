/*
 * httpStructures.c
 *
 *  Created on: 11 Apr. 2018
 *      Author: ben
 */

#include "httpStructures.h"
#include <stdlib.h>

eHeader_t* _initEHeader();
gHeader_t* _initGHeader();
rsHeader_t* _initRsHeader();
rqHeader_t* _initRqHeader();
status_t* _initHttpStatus();

void _freeHttpStatus(status_t* s);
void _freeRqHeader(rqHeader_t *h);
void _freeRsHeader(rsHeader_t *h);
void _freeGHeader(gHeader_t *h);
void _freeEHeader(eHeader_t* h);

request_t*
initRequest(){
	request_t* r = malloc(sizeof(request_t));
	r->eHeader=_initEHeader();
	r->gHeader=_initGHeader();
	r->httpVersion=NULL;
	r->method=NULL;
	r->uri=NULL;
	r->rqHeader=_initRqHeader();
	return(r);
}

response_t *
initResponse() {
	response_t* r = malloc(sizeof(response_t));
	r->httpVersion=NULL;
	r->status=_initHttpStatus();
	r->gHeader=_initGHeader();
	r->rsHeader=_initRsHeader();
	r->eHeader=_initEHeader();
	r->entityPath=NULL;
	return(r);
}

status_t*
_initHttpStatus(){
	status_t* s=malloc(sizeof(status_t));
	s->code=NULL;
	s->phrase=NULL;
	return(s);
}

void _freeHttpStatus(status_t* s) {
	free(s->code);
	free(s->phrase);
}

rqHeader_t*
_initRqHeader() {
	rqHeader_t* h=malloc(sizeof(rqHeader_t));
	h->authorization=NULL;
	h->from=NULL;
	h->ifModifiedSince=NULL;
	h->referrer=NULL;
	h->userAgent=NULL;
	return(h);
}

void _freeRqHeader(rqHeader_t *h) {
	free(h->authorization);
	free(h->from);
	free(h->ifModifiedSince);
	free(h->referrer);
	free(h->userAgent);
}

rsHeader_t*
_initRsHeader() {
	rsHeader_t* h=malloc(sizeof(rsHeader_t));
	h->location=NULL;
	h->server=NULL;
	h->wWWAuthenticate=NULL;
	return(h);
}

void _freeRsHeader(rsHeader_t *h) {
	free(h->location);
	free(h->server);
	free(h->wWWAuthenticate);
}

gHeader_t*
_initGHeader() {
	gHeader_t* h=malloc(sizeof(gHeader_t));
	h->date=NULL;
	h->pragma=NULL;
	return(h);
}

void _freeGHeader(gHeader_t *h) {
	free(h->date);
	free(h->pragma);
}

eHeader_t*
_initEHeader() {
	eHeader_t* h=malloc(sizeof(eHeader_t));
	h->allow=NULL;
	h->contentEncoding=NULL;
	h->contentLength=NULL;
	h->contentType=NULL;
	h->expires=NULL;
	h->lastModified=NULL;
	return(h);
}

void _freeEHeader(eHeader_t* h) {
	free(h->allow);
	free(h->contentEncoding);
	free(h->contentLength);
	free(h->contentType);
	free(h->expires);
	free(h->lastModified);
}

void freeRequest(request_t* r) {
	_freeGHeader(r->gHeader);
	_freeEHeader(r->eHeader);
	_freeRqHeader(r->rqHeader);
	free(r->httpVersion);
	free(r->method);
	free(r->uri);
}

void freeResponse(response_t* r) {
	_freeGHeader(r->gHeader);
	_freeEHeader(r->eHeader);
	_freeRsHeader(r->rsHeader);
	_freeHttpStatus(r->status);
	free(r->httpVersion);
	free(r->entityPath);
}
