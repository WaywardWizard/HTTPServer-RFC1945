#ifndef SERVER_H_
#define SERVER_H_

#define RECVBUFFER_SIZE  4096
#define MAX_READATTEMPT  5				// Max consecutive read failures allowed

#define TRUE  1
#define FALSE 0

#define EUSAGE 		  5
#define EEOF	      13 // At end of file, cant read any line
#define EINVALID_REQUEST 19 // Request was malformed
#define EREGCOMP	  23
#define EHEADINVALID  27 // Header was invalid
#define EPATH_INVALID 29

#define	METHOD_REGEX "^GET"
#define	URI_REGEX    "^([[:space:]]){1}"RELURI
#define HTTP_VERSION_REGEX "HTTP/[[:digit:]]+\.[[:digit:]]+"

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

#endif
