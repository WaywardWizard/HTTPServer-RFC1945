#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "httpStructures.h"
#include "./../utility/tcpSocketIo.h"
#include "./../utility/filesystem.h"
#include "./../utility/logger.h"
#include "./../utility/regexTool.h"
#include "http.h"

#define MIME_JS "application/javascript"
#define MIME_HTML "text/html"
#define MIME_CSS "text/css"
#define MIME_JPEG "image/jpeg"
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
void _httpGet(request_t *r, response_t *response, char* rootPath);
char* _getMimeType(char* fPath);
char* _compilePathFromURI(char* uri, char* rootPath);

void _notifyInvalidRequest();
void _parseRequestHeader(char* headerLine, request_t *r);
void _parseRequestEntity(request_t* r, int socketFd);
void _sendResponse(response_t* r, int socketFd);


void processRequest(int socketFd, char* rootPath) {
	/**
	 * Process an http request on socketFd. Send response back through socket.
	 *
	 * Respond only to valid GET requests, only with 404 or 200 status, only
	 * for mime types as specified in the assignment
	 */

	/* Read request, assemble response */
	request_t* r=_getRequest(socketFd);
	if(r==NULL) {
		mylog("Malformed request");

		/* No handling required for malformed requests in assignment */
		return;
	}

	response_t* rs=_getResponse(r, socketFd, rootPath);
	freeRequest(r);free(r);

	/* Send the response.*/
	_sendResponse(rs, socketFd);
	freeResponse(rs);free(rs);
}


request_t *_getRequest(int socketFd) {
	/**
	 * Read a request from <socketFd> & return a request structure
	 *
	 * Return null if no valid request could be read
	 */

	request_t* r=initRequest();

	/* Read request line from socket */
	char* requestLine = fdReadLine(socketFd);
	if(requestLine==NULL){
		freeRequest(r);free(r);
		free(requestLine);
		return(NULL);
	}

	/* Parse request line to request structure*/
	if(_parseRequestLine(requestLine, r)==EINVALID_REQUEST) {
		freeRequest(r);free(r);
		free(requestLine);
		return(NULL);
	}

	/* Not implemented*/
	_parseRequestHeader(requestLine, r);

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
	 * 		request_t r - The request. Should be parsed/populated with ateast
	 * 		a request line
	 *
	 * 		socketFd - file descriptor for open socket
	 *
	 * RETURN:
	 * 		response_t *r - response structure representing server response.
	 * 		The caller is responsible for freeing this.
	 *
	 * NOTE:
	 * 		Only the GET method is supported (for the assignment)
	 */

	response_t *rs=initResponse();
	rs->httpVersion=strdup(r->httpVersion);

	/* Write verions into response here */
	if(strcmp(r->method,"GET")==0) {
		_httpGet(r, rs, rootPath);
	}

	return(rs);
}


void _httpGet(request_t *request, response_t *response, char* rootPath) {
	/**
	 * Handle a get request. Populate response structure for that request
	 *
	 * ARGUMENT:
	 * 	request - request structure representing reqeust
	 * 	response - response structure to populate, representing the server
	 * 	response.
	 * 	rootPath - path to server document root
	 *
	 * RETURN:
	 * 	Mutate response structure
	 */

	request_t*r=request;
	char* statusCode;
	char* statusPhrase;
	char* resourcePath=_compilePathFromURI((r->uri), rootPath);
	char* contentType;

	/* Check the file exists */
	if(resourcePath==NULL||testFile(resourcePath, F_OK|R_OK)==FALSE) {
		statusCode = "404";
		statusPhrase="Not Found";

	} else {
		statusCode = "200";
		statusPhrase="OK";
		response->eHeader->contentType=strdup(_getMimeType(resourcePath));
		response->entityPath=resourcePath;
	}

	/* Set status and phrase */
	response->status->code=malloc(strlen(statusCode)+1);
	response->status->phrase=malloc(strlen(statusPhrase)+1);
	strcpy(response->status->code, statusCode);
	strcpy(response->status->phrase, statusPhrase);
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
	 * 	Caller is responsible for freeing the compiled path that is returned
	 *
	 * RETURN:
	 * 	allocated filepath of the uri. Null if cannot extract a valid filepath.
	 *
	 */
	int rPathLength=strlen(rootPath);
	char* uriPath;

	/* No match found */
	if(extractMatch(PATH, uri, &uriPath)==NULL) {
		_handleInvalidPath();
		return(NULL);
	}

	char* path = malloc(strlen(rootPath)+1+strlen(uriPath));
	strcpy(path, rootPath);
	strcat(path, "/");
	strcat(path, uriPath);
	free(uriPath);

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
	 * Load http request line into request structure. If the request is
	 * malformed, return EINVALID_REQUEST and write null bytes into request struct
	 */

	/* Extract method */
	requestLine = extractMatch(METHOD_REGEX, requestLine, &(r->method));
	if (requestLine==NULL){
		return(EINVALID_REQUEST);
	}

	/* Extract request URI */
	requestLine = extractMatch(URI_REGEX, requestLine, &(r->uri));
	if (requestLine==NULL){
		return(EINVALID_REQUEST);
	}

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

	/* Send a simple request (only the entity) if http0.9 */
	if(strcmp(r->httpVersion, "HTTP/0.9")!=0) {
		/* Status line */
		sendString(socketFd, r->httpVersion, " ");
		sendString(socketFd, r->status->code, " ");
		sendString(socketFd, r->status->phrase, "\n");

		/* Send headers for entity if entity exists */
		if (strcmp(r->status->code, "404")!=0) {
			sendString(socketFd, "Content-Type:", " ");
			sendString(socketFd, r->eHeader->contentType, "\n");
			sendChar(socketFd, "\n");
		}
	}

	/* Send Entity if exists*/
	if (r->entityPath!=NULL) {
		FILE *f = fopen(r->entityPath, "rb");
		if(f==NULL){
			handleFileOpenError();
		}
		sendFile(socketFd, f);
		fclose(f);
	}
}
