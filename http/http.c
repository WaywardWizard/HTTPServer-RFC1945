#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "httpStructures.h"
#include "../utility/tcpSocketIo.h"
#include "../utility/filesystem.h"
#include "../utility/logger.h"
#include "../utility/regexTool.h"
#include "http.h"

#define MIME_JS "javascript"
#define MIME_HTML "html"
#define MIME_CSS "css"
#define MIME_JPEG "jpeg"
#define MIME_DEFAULT "application/octet-stream$"
#define MATCH_MIME_JS "\\.js$"
#define MATCH_MIME_HTML "\\.html$"
#define MATCH_MIME_CSS "\\.css$"
#define MATCH_MIME_JPEG "\\.jpg$"

#define EINVALID_REQUEST 19 // Request was malformed

void _handleInvalidPath();

int _parseRequestLine(char* requestLine, request_t *r);
response_t* _getResponse(request_t *r, int socketFd, char* rootPath);
request_t *_getRequest(int socketFd);
response_t *_httpGet(request_t *r, char* rootPath);
char* _getMimeType(char* fPath);
char* _compilePathFromURI(char* uri, char* rootPath);

void _notifyInvalidRequest();
void _parseRequestHeader(char* headerLine, request_t *r);
void _parseRequestEntity(request_t* r, int socketFd);
void _sendResponse(response_t* r, int socketFd);


void processRequest(int socketFd, char* rootPath) {

	/* Read request, assemble response */
	request_t* r=_getRequest(socketFd);
	response_t* rs=_getResponse(r, socketFd, rootPath);
	freeRequest(r);free(r);

	/* Send the response and close up */
	_sendResponse(rs, socketFd);
	freeResponse(rs);free(rs);
}

request_t *_getRequest(int socketFd) {
	/**
	 * Read a request from a connected socket & return a request structure
	 */

	request_t* r=initRequest();
	char* requestLine = fdReadLine(socketFd);

	if(requestLine==NULL){
		handleInvalidRequest();
	}

	/* Read request line */
	_parseRequestLine(requestLine, r);
	free(requestLine);

	/* Read request header fields */
	while((requestLine=fdReadLine(socketFd))!=NULL) {

		/* Not implemented */
		_parseRequestHeader(requestLine, r);
	}

	/* Not implemented. Reads content-length bytes from fd */
	_parseRequestEntity(r, socketFd);

	return(r);
}


response_t*
_getResponse(request_t *r, int socketFd, char* rootPath) {
	/**
	 * Reply to a parsed request, via socketFd
	 *
	 * ARGUMENT:
	 * 		request_t r - The request. Should be parsed/populated
	 * 		socketFd - file descriptor for open socket
	 *
	 * NOTE:
	 * 		Only the GET method is supported (for the assignment)
	 */
	char *resourcePath=NULL;
	response_t *rs;
	if(strcmp(r->method,"GET")==0) {
		rs = _httpGet(r, rootPath);
	}
	return(rs);
}


response_t*
_httpGet(request_t *r, char* rootPath) {

	response_t* response;
	char* statusCode;
	char* statusPhrase;
	char* resourcePath=_compilePathFromURI((r->uri), rootPath);
	char* contentType;

	/* Check the file exists */
	if(testFile(resourcePath, F_OK|R_OK)==FALSE) {
		statusCode = "404";
		statusPhrase="Not Found";

	} else {
		statusCode = "200";
		statusPhrase="OK";
	}

	/* Set status and phrase */
	response->status->code=malloc(strlen(statusCode)+1);
	response->status->phrase=malloc(strlen(statusPhrase)+1);
	strcpy(response->status->code, statusCode);
	strcpy(response->status->phrase, statusPhrase);

	/* TODO Check if this needs to bee free'd*/
	contentType=_getMimeType(resourcePath);
	response->eHeader->contentType=contentType;

	response->entityPath=resourcePath;

	return(response);
}


char* _getMimeType(char* fPath) {
	/**
	 * Given a filepath, return it's mime type based on its file extension
	 */
	if (isMatch(""MATCH_MIME_JS, fPath)) {
		return(MIME_JS);
	} else if (isMatch(""MATCH_MIME_HTML, fPath)) {
		return(MIME_HTML);
	} else if (isMatch(""MATCH_MIME_JPEG, fPath)) {
		return(MIME_JPEG);
	} else if (isMatch(""MATCH_MIME_CSS, fPath)) {
		return(MIME_CSS);
	}
	return(""MIME_DEFAULT);
}


void
_parseRequestHeader(char* headerLine, request_t *r) {return;}


char*
_compilePathFromURI(char* uri, char* rootPath) {
	/**
	 * Given a URI and server root path, resolve to an absolute path
	 * Remove query and params part of the URI if any
	 *
	 * NOTE:
	 * 		Caller is responsible for freeing the compiled path that is returned
	 */
	int rPathLength=strlen(rootPath);
	char* uriPath;

	/* No match found */
	if(extractMatch(PATH, uri, &uriPath)==NULL) {
		_handleInvalidPath();
	}

	char* path = malloc(strlen(rootPath)+strlen(uri));
	strcpy(path, rootPath);
	strcat(path, uri);
	return(path);
}


void
_handleInvalidPath() {
	mylog("Could not extract a path from the given uri");
	exit(1);
}

void
_notifyInvalidRequest() {
	mylog("Request invalid.");
	exit(1);
}


int
_parseRequestLine(char* requestLine, request_t *r) {
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
	requestLine = extractMatch(URI_REGEX, requestLine, &(r->uri));

	/* Simple http request recieved => version is 0.9 */
	if(NULL==extractMatch(HTTP_VERSION_REGEX, requestLine, &(r->httpVersion))){
		char* version = "HTTP/0.9";
		r->httpVersion = malloc(strlen(version));
		strcpy(r->httpVersion, version);
	}

	return(0);
}


void _parseRequestEntity(request_t* r, int socketFd){}


void _sendResponse(response_t* r, int socketFd) {
	/* Serialize response structure and pipe it into socketFd */

	/* Status line */
	sendString(socketFd, r->httpVersion, ' ');
	sendString(socketFd, r->status->code, ' ');
	sendString(socketFd, r->status->phrase, '\n');
	sendChar(socketFd, '\n');

	/* Response header */
	sendString(socketFd, "Content-Type", ':');
	sendString(socketFd, r->eHeader->contentType, '\n');

	/* Send Entity */
	FILE *f = fopen(r->entityPath, "r");
	if(f==NULL){
		handleFileOpenError();
	}
	sendFile(socketFd, f);
	fclose(f);

}
