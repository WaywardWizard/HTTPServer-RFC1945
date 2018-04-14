#include <errno.h>
#include "filesystem.h"
#include "logger.h"


int testFile(char* path, int tests) {
	/**
	 * Tests a path with unistd.h access()
	 *
	 * ARGUMENT:
	 * 	path - location of file or folder to test
	 * 	tests - bitwise or of F_OK, R_OK, W_OK as per 'access()' documentation
	 *
	 * RETURN:
	 * 	true if path passes all tests
	 *
	 * NOTE:
	 *	 Requires #include <unistd.h>
	 */
	int e = access(path, tests);
	if(e==0){
		return(true);
	} else {
		switch(errno) {
		case EACCES:
			mylog("No read permissions or list permissions on parent.");
			break;
		case ENOENT:
			mylog("The given file path does not exist on the file system");
			break;
		case EROFS:
			mylog("The given file is read only");
			break;
		default:
			mylog("There is a problem with the given path");
		}
		return(FALSE);
	}
	return(false);
}

void handleFileOpenError() {
	mylog("Failed to open file");
}

void handleFileReadError(){
	mylog("An error occured when reading a file");
}

