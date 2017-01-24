/**
 * @file 
 * @brief Usefull function for the system project.
 * @note Namespace : all functions are prefixed by SU_ (for system_utils)
 */

#ifndef SYSTEM_UTILS_H
#define SYSTEM_UTILS_H


#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <stdarg.h>
#include <err.h>
#include <string.h>
#include <sysexits.h>

#include <assert.h>

/**
 * @brief Get the number of line of a file.
 * @param[in] file Path to the file to process.
 * @return Number of line in the file.
 * @note The file is not fully loaded in memory.
 **/
unsigned long SU_getFileNbLine(const char * file);

/**
 * @brief Split a file in sub files. 
 * @param[in] source_file Path to the file to process.
 * @param[in] nb_line_per_file Number of data to put in each subfile (the last file will contains all the remaining data).
 * @param[in] nb_files Number of file to create.
 * @param[in] source_file1 Name of the first subfile.
 * @param[in] ... Name of the other subfiles.
 **/
void SU_splitFile( const char * source_file, 
		   unsigned long nb_line_per_file,
		   unsigned long nb_files,
		   const char * source_file1, 
		   ... );

/**
 * @brief Split a file in sub files. 
 * @param[in] source_file Path to the file to process.
 * @param[in] nb_line_per_file Number of data to put in each subfile (the last file will contains all the remaining data).
 * @param[in] nb_files Number of file to create.
 * @param[in] file_names Array containing the subfiles names.
 **/
void SU_splitFile2( const char * source_file,
		    const unsigned long nb_line_per_file,
		    const unsigned long nb_files,
		    const char ** file_names);

/**
 * @brief Load a file of integer (one per line).
 * @param[in] file File to load.
 * @param[out] data Pointer to the array that will received the datas.
 * @return Number of data loaded.
 * @note No allocation needs to be perform but data must be freed after.
 **/
unsigned long SU_loadFile(const char * file, int ** data);

/**
 * @brief Load a file of integer (one per line).
 * @param[in] file File that will received the datas.
 * @param[in] data_size Number of data to save.
 * @param[in] data Array of data to write.
 **/
void SU_saveFile(const char * file, unsigned long data_size, const int * data);

/**
 * @brief Remove a file from filesystem.
 * @param[in] file File that will be deleted.
 * @todo Must be change by students
 **/
void SU_removeFile(const char * file);

/**
 * @brief fill a file with nb_elems random integers
 * @param[in] file_target File that will received the datas.
 * @param[in] nb_elems Number of data to generate.
 **/
void SU_randomGeneration(const char * file_target, unsigned long nb_elems);

/**
 * @brief Merge two sorted files of integers
 * @param[in] source_file1 First sorted file.
 * @param[in] source_file2 Second sorted file.
 * @param[in] file_target File that will received the datas.
 * @note Do not load all data in memory. Work on streams.
 **/
void SU_mergeSortedFiles(const char * source_file1,
			 const char * source_file2, 
			 const char * file_target);

/**
 * @brief Sort of an array of integer with Quick Sort algorithm.
 * @param[in] data_size Number of element in the data array 
 * @param[in,out] data Array of integer to sort.
 * @note Use a recursive, in place, version of quick sort.
 **/
void SU_QSort(unsigned long data_size, int * data);

/**
 * @brief Sort of an array of integer with Insertion Sort algorithm.
 * @param[in] data_size Number of element in the data array 
 * @param[in,out] data Array of integer to sort.
 **/
void SU_ISort(unsigned long data_size,  int * data);

/**
 * @brief Sort of an array of integer with Heap Sort algorithm.
 * @param[in] data_size Number of element in the data array 
 * @param[in,out] data Array of integer to sort.
 **/
void SU_HSort(unsigned long data_size,  int * data);

/**
 * @brief Test function. Juste print a message on stderr.
 **/
void SU_test();

#endif
