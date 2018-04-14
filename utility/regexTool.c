/*
 * regexTool.c
 *
 *  Created on: 13 Apr. 2018
 *      Author: ben
 */

char*
extractMatch(char* regex, char* searchString, char** destination) {
	/**
	 * Search <string> for match with <regex> Write match into match
	 * destination.
	 *
	 * Terminates if an error occured
	 *
	 * RETURN:
	 * 		NULL if no match.
	 * 		Else, pointer to remainder of search string.
	 */
	regex_t rx;
	regmatch_t match;
	int errSize=100; // Default error message size
	int error = regcomp(&rx, regex, REG_EXTENDED);
	int matchSize;

	/* Check compilation sucessful */
	if(error!=0){
		char errorMessage[errSize];
		regerror(error,&rx,errorMessage,errSize);
		mylog("Regex compilation error: \n");
		mylog("\t");
		mylog(errorMessage);
		exit(EREGCOMP);
	}

	/* Match */
	error = regexec(&rx,searchString, 1, &match, 0);
	regfree(&rx);

	/* Allocate destination memory & write match into destination */
	if (error!=REG_NOMATCH) {
		matchSize = match.rm_eo-match.rm_so;
		*destination = malloc(matchSize+1);
		strncpy(*destination, searchString+match.rm_so, matchSize);
		(*destination)[matchSize]='\0';
		return(searchString+match.rm_eo);
	}

	/* No match found */
	return(NULL);
}
