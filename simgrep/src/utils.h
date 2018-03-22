#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#ifndef UTILS_H_
#define UTILS_H_

/**
 *	@brief Checks for invalid command inputs
 *	@param argc Argument counter
 *	@param argv Argument vector
 *	@return Returns 0 on success, 1 otherwise
 */
int check_inputs(int argc, char** argv);

/**
 *	@brief Opens log file
 *	Makes use of an environment variable, LOGFILENAME,
 *	that must have been created and exported by the user
 *	@return Returns 0 on success, 1 otherwise
 */
int create_logFile();

/**
 *	@brief Writes on the open log file
 *	@param str String to be written to the file
 *	@return Returns 0 on success, 1 otherwise
 */
int write_logFile(char* str);

/**
 *	@brief Closes log file
 *	@return Returns 0 on success, 1 otherwise
 */
int close_logFile();

/**
 *	@brief Opens file passed as a parameter
 *	@param filename File's name
 *	@return Returns 0 on success, 1 otherwise
 */
int open_file(const char* filename);

/**
 *	@brief Searches for pattern (case-sensitive!)
 *	@param pattern Character pattern to be searched
 *	@param file Name of the file to be searched
 */
void search_pattern(char* pattern, char* file);

#endif /* UTILS_H_ */
