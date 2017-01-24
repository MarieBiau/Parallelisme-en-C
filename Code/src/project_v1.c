/**
 * @file 
 * @brief Implementation of the V1 of the system project.
 */

#include "project_v1.h"

/**
 * @brief Maximum length (in character) for a file name.
 **/
#define PROJECT_FILENAME_MAX_SIZE 1024

/**
 * @brief Type of the sort algorithm used in this version.
 **/
//#define SORTALGO(nb_elem, values) SU_ISort(nb_elem, values)
//#define SORTALGO(nb_elem, values) SU_HSort(nb_elem, values)
#define SORTALGO(nb_elem, values) SU_QSort(nb_elem, values)

/**********************************/

void projectV1(const char * i_file, const char * o_file, unsigned long nb_split){

  /* Get number of line to sort */
  int nb_print = 0;
  unsigned long nb_lines = SU_getFileNbLine(i_file);
  unsigned long nb_lines_per_files = nb_lines/ (unsigned long) nb_split;
  fprintf(stderr,
	  "Projet1 version with %lu split of %lu lines\n",
	  nb_split,
	  nb_lines_per_files);

  /* 0 - Deal with case nb_split = 1 */
  if(nb_split < 2){
    int * values = NULL;
    unsigned long nb_elem = SU_loadFile(i_file, &values);

    SORTALGO(nb_elem, values);

    SU_saveFile(o_file, nb_elem, values);
    
    free(values);
    return;
  }

  /* 1 - Spit the source file */

  /* 1.1 - Create a vector of target filenames for the split */
  char ** filenames = (char**) malloc(sizeof(char*) * (size_t) nb_split);
  char ** filenames_sort = (char**) malloc(sizeof(char*) * (size_t) nb_split);
  unsigned long cpt = 0;
  for(cpt = 0; cpt < nb_split; ++cpt){
    filenames[cpt] = (char *) malloc(sizeof(char) * PROJECT_FILENAME_MAX_SIZE);
    nb_print = snprintf(filenames[cpt],
			PROJECT_FILENAME_MAX_SIZE,
			"/tmp/tmp_split_%d_%lu.txt",getpid(), cpt);
    if(nb_print >= PROJECT_FILENAME_MAX_SIZE){
      err(1, "Out of buffer (%s:%d)", __FILE__, __LINE__ );
    }

    filenames_sort[cpt] = (char *) malloc(sizeof(char) * PROJECT_FILENAME_MAX_SIZE);
    nb_print = snprintf(filenames_sort[cpt],
			PROJECT_FILENAME_MAX_SIZE,
			"/tmp/tmp_split_%d_%lu.sort.txt",getpid(), cpt);
    if(nb_print >= PROJECT_FILENAME_MAX_SIZE){
      err(1, "Out of buffer (%s:%d)", __FILE__, __LINE__ );
    }
  }

  /* 1.2 - Split the source file */
  SU_splitFile2(i_file,
		nb_lines_per_files,
		nb_split,
		(const char **) filenames
		);

  /* 2 - Sort each file */
  projectV1_sortFiles(nb_split, (const char **) filenames, (const char **) filenames_sort);

  /* 3 - Merge (two by two) */
  projectV1_combMerge(nb_split, (const char **) filenames_sort, (const char *) o_file);

  /* 4 - Clear */
  for(cpt = 0; cpt < nb_split; ++cpt){
    free(filenames[cpt]); // not needed :  clear in sort
    free(filenames_sort[cpt]);
  }

  free(filenames);
  free(filenames_sort);

}

void projectV1_sortFiles(unsigned long nb_split, const char ** filenames, const char ** filenames_sort){

  unsigned long cpt=0;
  pid_t pid_fils;

  while(cpt<nb_split && pid_fils!=0){			//Pour le processus père
  	pid_fils=-1;
  	pid_fils=fork();											//On crée autant de fils qu'il y a de sous-fichiers
  	if (pid_fils==-1) {
      fprintf(stderr,"Erreur du fork");
      exit(EXIT_FAILURE);
    }
   	cpt++;
  }
  
  if (pid_fils==0){												//Pour chaque fils
  	cpt--;
    int * values = NULL;
    unsigned long nb_elem = SU_loadFile(filenames[cpt], &values);
    SU_removeFile(filenames[cpt]);				//Suppression du fichier utilisé
    fprintf(stderr, "Inner sort %lu: Array of %lu elem by %d\n", cpt, nb_elem, getpid());
    SORTALGO(nb_elem, values);            //Tri des valeurs
    SU_saveFile(filenames_sort[cpt], nb_elem, values);    //Sauvegarde du sous-fichier trié
    free(values);
    exit(EXIT_SUCCESS);
  }
  
  if (pid_fils!=0){                       //Pour le processus père
  	for(cpt=0; cpt<nb_split; cpt++){
      wait(NULL);                         //Il attend chacun des fils
  	}
  }

}

void projectV1_combMerge(unsigned long nb_split, const char ** filenames_sort, const char * o_file){

  int nb_print = 0;
  unsigned long cpt = 0;

  char previous_name [PROJECT_FILENAME_MAX_SIZE];		//previous_name prend le nom du premier sous-fichier trié
  nb_print = snprintf(previous_name,
		      PROJECT_FILENAME_MAX_SIZE,
		      "%s", filenames_sort[0]);
  if(nb_print >= PROJECT_FILENAME_MAX_SIZE){
    err(1, "Out of buffer (%s:%d)", __FILE__, __LINE__ );
  }

  char current_name [PROJECT_FILENAME_MAX_SIZE];		//current_name prend le nom du premier fichier merge
  nb_print = snprintf(current_name,
		      PROJECT_FILENAME_MAX_SIZE,
		      "/tmp/tmp_split_%d_merge_%d.txt", getpid(), 0);
  if(nb_print >= PROJECT_FILENAME_MAX_SIZE){
    err(1, "Out of buffer (%s:%d)", __FILE__, __LINE__ );
  }

  for(cpt = 1; cpt < nb_split - 1; ++cpt){					//Boucle qui parcourt les sous-fichiers triés

    fprintf(stderr, "Merge sort %lu : %s + %s -> %s \n",
	    cpt,
	    previous_name,
	    filenames_sort[cpt],
	    current_name);
    SU_mergeSortedFiles(previous_name,			//Fusion entre le fichier previous_name et le sous-fichier trié numéro cpt
			filenames_sort[cpt],
			current_name);												//Stockage dans le fichier current_name
    SU_removeFile(previous_name);
    SU_removeFile(filenames_sort[cpt]);			//Suppression des fichiers utilisés

    nb_print = snprintf(previous_name,			//previous_name prend le nom du précédent fichier current_name
			PROJECT_FILENAME_MAX_SIZE,
			"%s", current_name);
    if(nb_print >= PROJECT_FILENAME_MAX_SIZE){
      err(1, "Out of buffer (%s:%d)", __FILE__, __LINE__ );
    }

    nb_print = snprintf(current_name,				//current_name prend le nom du fichier merge numéro cpt
			PROJECT_FILENAME_MAX_SIZE,
			"/tmp/tmp_split_%d_merge_%lu.txt",getpid(), cpt);
    if(nb_print >= PROJECT_FILENAME_MAX_SIZE){
      err(1, "Out of buffer (%s:%d)", __FILE__, __LINE__ );
    }
  }

  /* Last merge */
  fprintf(stderr, "Last merge sort : %s + %s -> %s \n",
	  previous_name,
	  filenames_sort[nb_split - 1],
	  o_file);
  SU_mergeSortedFiles(previous_name,			//Fusion entre le dernier fichier merge qui porte le numéro nb_split-2 et
		      filenames_sort[nb_split - 1],		//le dernier sous-fichier trié qui porte le numéro nb_split-1
		      o_file);												//Stockage dans le fichier o_file
  SU_removeFile(previous_name);
  SU_removeFile(filenames_sort[nb_split - 1]);	//Suppression des fichiers utilisés

}
