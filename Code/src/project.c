#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>	/* time */
#include <unistd.h>	/* Unix std */
#include <getopt.h>	/* Getopt */

#include "system_utils.h"
#include "project_v0.h"
#include "project_v1.h"
#include "project_v2.h"
#include "project_v3.h"

/**
 * @brief Maximum length (in character) for a file name.
 **/
#define PROJECT_FILENAME_MAX_SIZE 1024

/**
 * @brief Type of the sort algorithm used in the demo.
 **/
//#define SORTALGO(nb_elem, values) SU_ISort(nb_elem, values)
//define SORTALGO(nb_elem, values) SU_HSort(nb_elem, values)
#define SORTALGO(nb_elem, values) SU_QSort(nb_elem, values)

/**
 * @brief Display the project commande usage.
 **/
void usage(){
  fprintf(stderr, "Usage: %s [-hmn] [-i arg] [-o arg]	       \n", __FILE__);
  fprintf(stderr, "\n");
  fprintf(stderr, "Options: \n");
  fprintf(stderr, "\t -i arg \t Input file (if needed)	\n");
  fprintf(stderr, "\t -o arg \t Output file (if needed) \n");
  fprintf(stderr, "\t -m arg \t Program mode		\n");
  fprintf(stderr, "\t -n arg \t Number of value to set	\n");
  fprintf(stderr, "\t -k arg \t Number of split		\n");
  fprintf(stderr, "\t -h     \t Display this message	\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Modes:\n");
  fprintf(stderr, "\t test	    \t Display a test message		  \n");
  fprintf(stderr, "\t generation    \t Fill a file with random numbers	  \n");
  fprintf(stderr, "\t lineCount	    \t Count the number of line of a file \n");
  fprintf(stderr, "\t demoSortSplit \t Sort a file with a 3 files split	  \n");
  fprintf(stderr, "\t demoSort	    \t Sort a file without spliting	  \n");
  fprintf(stderr, "\t projectV0	    \t V0 of the project         	  \n");
  fprintf(stderr, "\t projectV1	    \t V1 of the project         	  \n");
  fprintf(stderr, "\t projectV2	    \t V2 of the project         	  \n");
  fprintf(stderr, "\t projectV3	    \t V3 of the project         	  \n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Examples:								    \n");
  fprintf(stderr, "./bin/project -m test						    \n");
  fprintf(stderr, "./bin/project -m generation -o /tmp/test.txt -n 50000000		    \n");
  fprintf(stderr, "./bin/project -m lineCount -i /tmp/test.txt				    \n");
  fprintf(stderr, "./bin/project -m demoSort -i /tmp/test.txt -o /tmp/test.sort.txt	    \n");
  fprintf(stderr, "./bin/project -m demoSortSplit -i /tmp/test.txt -o /tmp/test.sort.txt    \n");
  fprintf(stderr, "./bin/project -m projectV0 -i /tmp/test.txt -o /tmp/test.sort.txt -k 5   \n");
}

/* Mode functions declaration */
void test();
void generation(const char * file_target, unsigned long nb_elems);
void lineCount(const char * i_file);
void demoSort(const char * i_file, const char * o_file);
void demoSortSplit(const char * i_file, const char * o_file);

/* Main function */
int main(int argc, char ** argv){

  /* Initialize random seed: */
  srand ((unsigned int)time(NULL)); // Good enought for this application

  /* Options */
  char * i_file = "";
  char * o_file = "";
  char * mode	= "";

  int mode_test          = 0;
  int mode_demoSort      = 0;
  int mode_projectV0     = 0;
  int mode_projectV1     = 0;
  int mode_projectV2     = 0;
  int mode_projectV3     = 0;
  int mode_demoSortSplit = 0;
  int mode_lineCount     = 0;
  int mode_generation    = 0;

  unsigned long nb_values = 100;
  unsigned long nb_split  = 4;

  /* Parameters parsing */
  int c = 0;
  int errflg = 0;;

  while ((c = getopt(argc, argv, "hm:i:o:n:k:")) != -1) {
    switch(c) {
    case 'n':
      nb_values = strtoul(optarg, NULL, 0);
      break;
    case 'k':
      nb_split = strtoul(optarg, NULL, 0);
      break;
    case 'i':
      i_file = optarg;
      break;
    case 'o':
      o_file = optarg;
      break;
    case 'm':
      mode = optarg;
      break;
    case 'h':
      usage();
      exit(0);
      break;
    case ':':	    /* -i or -o without operand */
      fprintf(stderr, "Option -%c requires an operand\n", optopt);
      errflg++;
      break;
    case '?':
      fprintf(stderr, "Unrecognized option: -%c\n", optopt);
      errflg++;
    }
  }

  /* Check mode */
  if(strcmp(mode, "demoSort") == 0){
    mode_demoSort = 1;
    /* need at least in and out file */
    if(strcmp(i_file, "") == 0 || strcmp(o_file, "") == 0){
      fprintf(stderr, "Missing parameters: need an output and an input file.\n");
      errflg++;
    }
  } else if(strcmp(mode, "demoSortSplit") == 0){
    mode_demoSortSplit = 1;
    /* need at least in and out file */
    if(strcmp(i_file, "") == 0 || strcmp(o_file, "") == 0){
      fprintf(stderr, "Missing parameters: need an output and an input file.\n");
      errflg++;
    }
  } else if(strcmp(mode, "projectV0") == 0){
    mode_projectV0 = 1;
    /* need at least in and out file */
    if(strcmp(i_file, "") == 0 || strcmp(o_file, "") == 0){
      fprintf(stderr, "Missing parameters: need an output and an input file.\n");
      errflg++;
    }
  } else if(strcmp(mode, "projectV1") == 0){
    mode_projectV1 = 1;
    /* need at least in and out file */
    if(strcmp(i_file, "") == 0 || strcmp(o_file, "") == 0){
      fprintf(stderr, "Missing parameters: need an output and an input file.\n");
      errflg++;
    }
  } else if(strcmp(mode, "projectV2") == 0){
    mode_projectV2 = 1;
    /* need at least in and out file */
    if(strcmp(i_file, "") == 0 || strcmp(o_file, "") == 0){
      fprintf(stderr, "Missing parameters: need an output and an input file.\n");
      errflg++;
    }
  } else if(strcmp(mode, "projectV3") == 0){
    mode_projectV3 = 1;
    /* need at least in and out file */
    if(strcmp(i_file, "") == 0 || strcmp(o_file, "") == 0){
      fprintf(stderr, "Missing parameters: need an output and an input file.\n");
      errflg++;
    }
  } else if(strcmp(mode, "test") == 0) {
    mode_test = 1;
  } else if(strcmp(mode, "generation") == 0) {
    mode_generation = 1;
    /* need at least out file */
    if(strcmp(o_file, "") == 0){
      fprintf(stderr, "Missing parameters: need an output file.\n");
      errflg++;
    }
  } else if(strcmp(mode, "lineCount") == 0) {
    mode_lineCount = 1;
    /* need at least out file */
    if(strcmp(i_file, "") == 0){
      fprintf(stderr, "Missing parameters: need an intput file.\n");
      errflg++;
    }
  } else {
    errflg ++;
  }


  if (errflg) {
    usage();
    exit(1);
  }

  /* Main */
  /* Mode test */
  if(mode_test){
    test();
  }

  /* Call the right function for a mode */
  if(mode_generation){
    /* Mode generation */
    generation(o_file, nb_values);
  } else if(mode_lineCount){
    /* Mode line count */
    lineCount(i_file);
  } else if(mode_demoSortSplit){
    /* Mode demo sort split */
    demoSortSplit(i_file, o_file);
  } else if(mode_demoSort){
    /* Mode demo sort */
    demoSort(i_file, o_file);
  } else if(mode_projectV0){
    /* Mode projet V0 */
    projectV0(i_file, o_file, nb_split);
  } else if(mode_projectV1){
    /* Mode projet V1 */
    projectV1(i_file, o_file, nb_split);
  } else if(mode_projectV2){
    /* Mode projet V2 */
    projectV2(i_file, o_file, nb_split);
  } else if(mode_projectV3){
    /* Mode projet V3 */
    projectV3(i_file, o_file, nb_split);
  }

  /* End */
  return 0;
}

/* Modes functions  */

void test(){
  SU_test();
}

void generation(const char * file_target, unsigned long nb_elems){
  SU_randomGeneration(file_target, nb_elems);
}

void lineCount(const char * i_file){
  unsigned long nb_lines = SU_getFileNbLine(i_file);
  fprintf(stderr, "The source file has %lu lines\n", nb_lines);
}

void demoSortSplit(const char * i_file, const char * o_file){

  unsigned long nb_lines = SU_getFileNbLine(i_file);
  fprintf(stderr, "Demo version with 3 split of %lu lines\n", nb_lines/3);

  SU_splitFile(i_file,
	       nb_lines/3,
	       3,
	       "/tmp/test-split-a.txt",
	       "/tmp/test-split-b.txt",
	       "/tmp/test-split-c.txt"
	       );


  int * values1 = NULL;
  unsigned long nb_elem1 = SU_loadFile("/tmp/test-split-a.txt", &values1);
  SU_removeFile("/tmp/test-split-a.txt");
  fprintf(stderr, "F1: Array of %lu elem\n", nb_elem1);
  SORTALGO(nb_elem1, values1);
  SU_saveFile("/tmp/test-split-a-sort.txt", nb_elem1, values1);
  free(values1);

  int * values2 = NULL;
  unsigned long nb_elem2 = SU_loadFile("/tmp/test-split-b.txt", &values2);
  SU_removeFile("/tmp/test-split-b.txt");
  fprintf(stderr, "F2: Array of %lu elem\n", nb_elem2);
  SORTALGO(nb_elem2, values2);
  SU_saveFile("/tmp/test-split-b-sort.txt", nb_elem2, values2);
  free(values2);

  fprintf(stderr, "Start merge sort\n");
  SU_mergeSortedFiles("/tmp/test-split-a-sort.txt",
		      "/tmp/test-split-b-sort.txt",
		      "/tmp/test-res-tmp.txt");

  SU_removeFile("/tmp/test-split-a-sort.txt");
  SU_removeFile("/tmp/test-split-b-sort.txt");

  int * values3 = NULL;
  unsigned long nb_elem3 = SU_loadFile("/tmp/test-split-c.txt", &values3);
  SU_removeFile("/tmp/test-split-c.txt");
  fprintf(stderr, "F3: Array of %lu elem\n", nb_elem3);
  SORTALGO(nb_elem3, values3);
  SU_saveFile("/tmp/test-split-c-sort.txt", nb_elem3, values3);
  free(values3);

  fprintf(stderr, "Start merge sort 2\n");
  SU_mergeSortedFiles("/tmp/test-res-tmp.txt",
		      "/tmp/test-split-c-sort.txt",
		      o_file);

  SU_removeFile("/tmp/test-split-c-sort.txt");
  SU_removeFile("/tmp/test-res-tmp.txt");
}

void demoSort(const char * i_file, const char * o_file){

  fprintf(stderr, "Demo version sort\n");

  int * values = NULL;
  unsigned long nb_elem = SU_loadFile(i_file, &values);

  SORTALGO(nb_elem, values);
  
  SU_saveFile(o_file, nb_elem, values);

  free(values);
  fprintf(stderr, "End\n");
}
