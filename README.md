# About this project
This code partially implements an HTTP1.0 server as per RFC1945 and supports 0.9 & 1.0 GET requests, 404|200 responses and mime types of {js, html, css, jpeg}

Demonstrates threading, thread local storage, socket io, makefiles, data structures, c competency. 

## Of Note
- Implementation of a multithreaded socketio readline abstraction. A socket is locked to a thread once it has been read from, and is read in buffered chunks untill a newline occurs. The leftover is then used in the next read. This allowed for reading blocks of bytes from a socket (rather than one by one). Once leftover for a socket is used it will become readable by another thread (through the readline() abstraction). 
