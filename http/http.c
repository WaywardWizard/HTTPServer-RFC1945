#include "httpStructures.h"
#include "../utility/tcpSocketIo.h"
#include <stdlib.h>

request_t
*readRequest(int socketFd) {
	/**
	 * Read a request from a connected socket & return a request structure
	 */
	request_t* r=initRequest();
	char* requestLine = fdReadLine(socketFd);

	if(requestLine==NULL){
		notifyInvalidRequest();
	}

	/* Read request line */
	parseRequestLine(requestLine, r);
	free(requestLine);

	/* Read request header fields */
	while(strcmp((requestLine=fdReadLine(socketFd)), '')!=0) {

		/* Not implemented */
		parseRequestHeader(requestLine, r);
	}

	/* Not implemented */
	parseRequestEntity();

	return(r);
}


void
replyRequest(request_t *r, int socketFd, char* rootPath) {
	/**
	 * Reply to a parsed request, via socketFd
	 *
	 * ARGUMENT:
	 * 		socketFd - file descriptor for open socket
	 *
	 * NOTE:
	 * 		Only the GET method is supported for  the assignment
	 */
	char *resourcePath=NULL;
	switch(r->method) {
	case "GET":
		httpGet(r, rootPath);
		break;
	}
}


void
httpGet(request_t r, char* rootPath) {

	response_t response;
	char* status;
	switch(r->httpVersion) {
		case "HTTP/0.9":
			httpGet0p9(r, rootPath);
			break;
		case "HTTP/1.0":
			httpGet1p1(r, rootPath);
			break;
	}
	char* resourcePath=compilePathFromURI(r->uri, rootPath);

	/* Check the file is readable */
	if((testPath(resourcePath, R_OK))==FALSE) {
		/* Print error message and close up */
		handleInvalidRequest();
	}

	/* Check the file exists */
	if(testPath(resourcePath, F_OK)==FALSE) {
		status = "404"; 		//
	} else {
		status = "200";
	}
}


void
parseRequestHeader(char* headerLine, request_t *r) {}


char*
compilePathFromURI(char* uri, char* rootPath) {
	/**
	 * Given a URI and server root path, resolve to an absolute path
	 * Remove query and params part of the URI if any
	 *
	 * NOTE:
	 * 		Caller is responsible for freeing the compiled path
	 */
	int rPathLength=strlen(rootPath);
	char* uriPath;

	/* No match found */
	if(extractMatch(PATH, uri, &uriPath)==NULL) {
		handleInvalidPath();
	}

	char* path = malloc(strlen(rootPath)+strlen(uri));
	strcpy(path, rootPath);
	strcat(path, uri);
	return(path);
}


void
notifyInvalidRequest() {
	mylog("Request invalid.");
	exit(1);
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
	requestLine = extractMatch(URI_REGEX, requestLine, &(r->uri));

	/* Simple http request recieved => version is 0.9 */
	if(NULL==extractMatch(HTTP_VERSION_REGEX, requestLine, &(r->httpVersion))){
		char* version = "HTTP/0.9";
		r->httpVersion = malloc(strlen(version));
		strcpy(r->httpVersion, version);
	}

	return(0);
}

