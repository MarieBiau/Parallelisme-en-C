/**
 * @file 
 * @brief Implementation of the V3 of the system project.
 */

#include "project_v3.h"

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

void projectV3(const char * i_file, const char * o_file, unsigned long nb_split){

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
  char ** filenames_s = (char**) malloc(sizeof(char*) * (size_t) nb_split);
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
    
    filenames_s[cpt] = (char *) malloc(sizeof(char) * PROJECT_FILENAME_MAX_SIZE);
  }

  /* 1.2 - Split the source file */
  SU_splitFile2(i_file,
		nb_lines_per_files,
		nb_split,
		(const char **) filenames
		);

  /* 2 - Sort each file */
  projectV3_sortFiles(nb_split, (const char **) filenames, (const char **) filenames_sort);

  /* 3 - Merge (two by two) */
  projectV3_combMerge(nb_split, (const char **) filenames_sort, (const char **) filenames_s, (const char *) o_file);

  /* 4 - Clear */
  for(cpt = 0; cpt < nb_split; ++cpt){
    free(filenames[cpt]); // not needed :  clear in sort
    free(filenames_sort[cpt]);
    free(filenames_s[cpt]);
  }

  free(filenames);
  free(filenames_sort);
  free(filenames_s);

}

void projectV3_sortFiles(unsigned long nb_split, const char ** filenames, const char ** filenames_sort){

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

void projectV3_combMerge(unsigned long nb_split, const char ** filenames_sort, const char ** filenames_s, const char * o_file){

	int nb_print=0;
	pid_t pid_fils1=-1;
	pid_t pid_fils2=-1;	
	char name [PROJECT_FILENAME_MAX_SIZE];
	char name_fils1 [PROJECT_FILENAME_MAX_SIZE];
	char name_fils2 [PROJECT_FILENAME_MAX_SIZE];

	/* Cas de base */

	if (nb_split==1) {			//Si le nombre de splits vaut 1
		rename(filenames_sort[0],o_file);			//Alors on renomme le fichier
	}
	
	else if (nb_split==2 || nb_split==3) {		//Si le nombre de splits vaut 2 ou 3 alors on effectue la fusion

		fprintf(stderr, "Merge sort : %s + %s -> %s \n",
			filenames_sort[0],
			filenames_sort[1],
			o_file);
		SU_mergeSortedFiles(filenames_sort[0],	//Fusion entre le premier et le deuxième sous-fichier trié
			filenames_sort[1],
			o_file);															//Stockage dans le fichier o_file
		SU_removeFile(filenames_sort[0]);
		SU_removeFile(filenames_sort[1]);				//Suppression des fichiers utilisés
	
	  if (nb_split==3) {		//Si le nombre de splits vaut 3, il y a une fusion supplémentaire
	  	fprintf(stderr, "Merge sort : %s + %s -> %s \n",
				filenames_sort[2],
				o_file,
				o_file);
			SU_mergeSortedFiles(filenames_sort[2],		//Fusion entre le troisième sous-fichier trié et la fusion des deux premiers
				o_file,
				o_file);																//Stockage dans le fichier o_file
			SU_removeFile(filenames_sort[2]);					//Suppression des fichiers utilisés
		}
 
  }

	/* Cas général */
	
	else {		//Si le nombre de splits est supérieur à 3
	
		pid_fils1=-1;
		pid_fils2=-1;
		unsigned long cpt;
		pid_fils1=fork();		//On créé un premier fils
  	
  	if (pid_fils1==-1) {
			fprintf(stderr,"Erreur du fork");
			exit(EXIT_FAILURE);
		}

		if (pid_fils1==0) {			//Effectué par le fils numéro 1
			nb_print = snprintf(name,			//name prend le nom du fichier merge portant le pid du fils numéro 1
				      PROJECT_FILENAME_MAX_SIZE,
				      "/tmp/tmp_split_%d_merge.txt", getpid());
	  	if(nb_print >= PROJECT_FILENAME_MAX_SIZE){
	    	err(1, "Out of buffer (%s:%d)", __FILE__, __LINE__ );
	  	}
	  	for (cpt=0;cpt<nb_split/2;cpt++) {
	  		filenames_s[cpt]=filenames_sort[cpt];
	  	}
	  	projectV3_combMerge(nb_split/2, (const char **) filenames_s, (const char **) filenames_s, (const char *) name);		//On effectue la récursivité sur la première moitié des fichiers
	  	exit(EXIT_SUCCESS);
	  }
		
		if (pid_fils1!=0) {			//Effectué par le père
			pid_fils2=fork();		//On créé un second fils
			if (pid_fils2==-1){
				fprintf(stderr,"Erreur du fork");
    		exit(EXIT_FAILURE);
    	}
		}
		
		if (pid_fils2==0) {			//Effectué par le fils numéro 2
			nb_print = snprintf(name,			//name prend le nom du fichier merge portant le pid du fils numéro 2
				      PROJECT_FILENAME_MAX_SIZE,
				      "/tmp/tmp_split_%d_merge.txt", getpid());
	  	if(nb_print >= PROJECT_FILENAME_MAX_SIZE){
	    	err(1, "Out of buffer (%s:%d)", __FILE__, __LINE__ );
	  	}
	  	for (cpt=nb_split/2;cpt<nb_split;cpt++) {
	  		filenames_s[cpt-(nb_split/2)]=filenames_sort[cpt];
	  	}
	  	projectV3_combMerge((nb_split/2+nb_split%2), (const char **) filenames_s, (const char **) filenames_s, (const char *) name);		//On effectue la récursivité sur la deuxième moitié des fichiers
	  	exit(EXIT_SUCCESS);
	  }

	  if (pid_fils1!=0 && pid_fils2!=0) {		//Le processus père  	
	  	wait(NULL);
	  	wait(NULL);		//Doit attendre ses deux fils	  	  	
	  	nb_print = snprintf(name_fils1,			//name_fils1 prend le nom du fichier merge portant le pid du fils numéro 1
				      PROJECT_FILENAME_MAX_SIZE,
				      "/tmp/tmp_split_%d_merge.txt", pid_fils1);
	  	if(nb_print >= PROJECT_FILENAME_MAX_SIZE){
	    	err(1, "Out of buffer (%s:%d)", __FILE__, __LINE__ );
	  	}
	  	nb_print = snprintf(name_fils2,			//name_fils2 prend le nom du fichier merge portant le pid du fils numéro 2
				      PROJECT_FILENAME_MAX_SIZE,
				      "/tmp/tmp_split_%d_merge.txt", pid_fils2);
	  	if(nb_print >= PROJECT_FILENAME_MAX_SIZE){
	    	err(1, "Out of buffer (%s:%d)", __FILE__, __LINE__ );
	  	}
	  	
	  	fprintf(stderr, "Merge sort : %s + %s -> %s \n",		
				name_fils1,
				name_fils2,
				o_file);
			SU_mergeSortedFiles(name_fils1,			//Fusion entre le fichier de fusion du fils numéro 1 et le fichier de fusion du fils numéro 2
				name_fils2,
				o_file);													//Stockage dans le fichier o_file
			SU_removeFile(name_fils1);
			SU_removeFile(name_fils2);					//Suppression des fichiers utilisés
			
	  }
	  
	}

}
