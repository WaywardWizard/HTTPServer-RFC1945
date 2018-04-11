/*
 * httpStructures.h
 *
 *  Created on: 11 Apr. 2018
 *      Author: ben
 */

typedef struct generalHeader gHeader_t;
typedef struct requestHeader rqHeader_t;
typedef struct entityHeader eHeader_t;
typedef struct responseHeader rsHeader_t;
typedef struct request request_t;
typedef struct response response_t;
typedef struct httpStatus status_t;

request_t* initRequest();
response_t* initResponse();
