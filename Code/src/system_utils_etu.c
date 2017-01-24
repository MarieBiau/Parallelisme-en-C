/**
 * @file 
 * @brief Implementation by students of usefull function for the system project.
 * @todo Change the SU_removeFile to use exec instead of system.
 */


#include "system_utils.h"

/**
 * @brief Maximum length (in character) for a command line.
 **/
#define SU_MAXLINESIZE (1024*8) 

/********************** File managment ************/

void SU_removeFile(const char * file){
  
	pid_t pid_fils = -1;
	pid_fils = fork();		//Création d'un fils
  
	if (pid_fils==-1) {
		fprintf(stderr,"Erreur du fork");
	}
  
	else if (pid_fils==0) {		//Exécuté par le fils
		char buffer[SU_MAXLINESIZE-3];
		snprintf(buffer, SU_MAXLINESIZE-3, "%s",file);	//On met dans buffer les paramètres de la commande "rm"
		fprintf(stderr, "%s\n", buffer);
		if (execl("/bin/rm","rm",buffer,NULL)==-1) {	//Suppression du fichier grâce à l'utilisation de execl
			fprintf(stderr,"Erreur de execl");
			exit(EXIT_FAILURE);
		}
	}
 
	else { wait(NULL); }		//Le père attend son fils
	
}
