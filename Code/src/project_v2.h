/**
 * @file
 * @brief V2 of the system project.
 */

#ifndef PROJECT_V2_H
#define PROJECT_V2_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>	/* time */
#include <unistd.h>	/* Unix std */
#include <sys/types.h>
#include <sys/wait.h>

#include "system_utils.h"

/**
 * @brief Function to sort a file using an external sort and nb_split subfiles.
 * @param[in] i_file Name of the file to process.
 * @param[in] o_file Name of the output sorted file.
 * @param[in] nb_split Number of subfiles to use for the external sort.
 * @note Parallelism.
 **/
void projectV2(const char * i_file, const char * o_file, unsigned long nb_split);

/**
 * @brief Function to sort a temporary subfile and remove it using parallelism.
 * @param[in] nb_split Index of the subfile in the array of files.
 * @param[in] filenames Array of file to sort names.
 * @param[in] filenames_sort Array of sorted file names.
 **/
void projectV2_sortFiles(unsigned long nb_split,
			 const char ** filenames,
			 const char ** filenames_sort);

/**
 * @brief Function to sort-merge a list of sorted subfiles using parallelism.
 * @param[in] nb_split Index of the subfile in the array of files.
 * @param[in] filenames_sort Array of sorted file names.
 * @param[in] o_file Name of the output file where sorted data are written.
 * @note It works in stream. Files are not fully loaded in memory.
 **/
void projectV2_combMerge(unsigned long nb_split,
			 const char ** filenames_sort,
			 const char * o_file);

#endif
