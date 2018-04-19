/*
 * Author: 			Ben Tomlin
 * Student Id:		btomlin
 * Student Nbr:		834198
 * Date:			Apr 2018
 */

#ifndef HTTP_HTTP_H_
#define HTTP_HTTP_H_

#define METHOD_REGEX "^GET"
#define URI_REGEX    "^([[:space:]]){1}"RELURI

/* \\, escape once for c to get a literal \ */
#define HTTP_VERSION_REGEX "HTTP/[[:digit:]]+\\.[[:digit:]]+"

/* Regex building blocks as per RFC1945 */
#define CTL			 "[:cntrl:]"
#define ALPHANUM      "[:alpha:][:digit:]"
#define SP			 "[:space:]"
#define HEX			 "A-Fa-f[:digit:]"
#define SAFE			 "$_.-"
#define UNSAFE		 CTL SP "#%<>\""
#define EXTRA		 "!*'(),"
#define NATIONAL      "[^"RESERVED EXTRA UNSAFE ALPHANUM SAFE "]"
#define ESCAPE		 "(%["HEX"]{2})"
#define RESERVED		 ";/@?:&=+"
#define UNRESERVED	 "["ALPHANUM EXTRA SAFE"]|"NATIONAL
#define UCHAR 		 "("UNRESERVED"|"ESCAPE")"
#define PCHAR		 "([:@+=&]|"UCHAR")"
#define SEGMENT 		 PCHAR "*"
#define FSEGMENT		 PCHAR "+"
#define PATH			 "("FSEGMENT "(/" SEGMENT ")*)"
#define PARAM		 "("PCHAR"|/)*"
#define PARAMS		 PARAM"(;"PARAM")*"
#define QUERY		 "(" UCHAR "|[" RESERVED "])*"
#define RELPATH		 "("PATH"?(;"PARAMS")?(([\?])"QUERY")?)"
#define ABSPATH		 "(/"RELPATH")"
#define RELURI	     "("ABSPATH"|"RELPATH")"

void processRequest(int socketFd, char* rootPath);


#endif /* HTTP_HTTP_H_ */
