/**
 * @file 
 * @brief Implementation of usefull function for the system project.
 * @warning You should not modifie the current file.
 */


#include "system_utils.h"

/**
 * @brief Maximum length (in character) for a file name.
 **/
#define SU_MAXLINESIZE (1024*8) 

/********************** File managment ************/

unsigned long SU_getFileNbLine(const char * file){

  unsigned long line_count = 0;

  int fd;
  ssize_t len;
  char *p;
  static char small_buffer[SU_MAXLINESIZE];
  static char *buffer = small_buffer;
  static size_t buffer_size = SU_MAXLINESIZE;

  /* Open the file */
  if (file == NULL) {
    warn("%s: read", file);
    return (0);
  } else {
    if ((fd = open(file, O_RDONLY, 0)) < 0) {
      warn("%s - (%s:%d ): open", file, __FILE__, __LINE__ );
      return (0);
    }
  }

  /* Count line number */
  while ((len = read(fd, buffer, buffer_size))) {
    if (len == -1) {
      warn("%s - (%s:%d): read", file, __FILE__, __LINE__ );
      (void)close(fd);
      return 0;
    }

    for (p = buffer; len--; ++p){
      if (*p == '\n'){
	++line_count;
      }
    }
  }

  (void)close(fd);
  return line_count;
}

void SU_splitFile2( const char * source_file,
		    const unsigned long nb_line_per_file,
		    const unsigned long nb_files,
		    const char ** file_names){
  
  unsigned long nb_line_in_cf = 0;
  char * p = NULL;

  /* Nothing to do if no files */
  if(nb_files <= 0){
    return ;
  }

  int ifd;
  int ofd = -1;
  unsigned long cfile_id = 0;
  static char small_buffer[SU_MAXLINESIZE];
  static char *buffer = small_buffer;
  static size_t buffer_size = SU_MAXLINESIZE;

  /* open in stream */
  if ((ifd = open(source_file, O_RDONLY, 0)) < 0) {
    free(file_names);
    err(EX_NOINPUT, "open (%s:%d)", __FILE__, __LINE__ );
  }

  /* Open first output file */
  if ((ofd = open(file_names[cfile_id] , O_WRONLY | O_CREAT | O_TRUNC,
		  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH )) < 0){
    free(file_names);
    (void)close(ifd);
    err(EX_NOINPUT, "open (%s:%d)", __FILE__, __LINE__ );
  }

  /* Read from in put */
  char * begin_ob = buffer;
  ssize_t len = -1;
  while ((len = read(ifd, buffer, buffer_size))) {
    if (len == -1) {
      free(file_names);
      (void)close(ifd);
      (void)close(ofd);
      err(EX_IOERR, "read (%s:%d)", __FILE__, __LINE__ );
    }

    begin_ob = buffer;
    unsigned long bufferLines = 0;

    /* look for \n */
    for (p = buffer; len--; ++p){
      if (*p == '\n'){

	++bufferLines;

	/* Change file ? */
	if((nb_line_in_cf + bufferLines >= nb_line_per_file) &&
	   (cfile_id < nb_files - 1)
	   ){
	  /* Write data before changeing file */
	  ssize_t sb = p - begin_ob + 1;
	  if (write(ofd, begin_ob, (size_t) sb) != sb) {
	    (void)close(ifd);
	    (void)close(ofd);
	    free(file_names);
	    err(EX_IOERR, "write (%s:%d)", __FILE__, __LINE__ );
	  }
	  /* Close previous file */
	  (void)close(ofd);

	  /* reset buffer boundaries */
	  begin_ob = p + 1;
	  bufferLines = 0;
	  nb_line_in_cf = 0;
	  ++cfile_id;

	  if ((ofd = open(file_names[cfile_id] , O_WRONLY | O_CREAT | O_TRUNC ,
			  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH )) < 0){
	    (void)close(ifd);
	    err(EX_NOINPUT, "open (%s:%d)", __FILE__, __LINE__ );
	  }
	}
      }
    }

    /* Write data befor changeing buffer */
    ssize_t sb = p - begin_ob;
    if(sb > 0){
      if (write(ofd, begin_ob, (size_t) sb) != sb) {
	err(EX_IOERR, "write (%s:%d)", __FILE__, __LINE__ );
      }
      nb_line_in_cf += bufferLines;
    }
  }


  /* Close last file */
  (void)close(ifd);
  (void)close(ofd);
}


void SU_splitFile( const char * source_file,
		   const unsigned long nb_line_per_file,
		   const unsigned long nb_files,
		   const char * source_file1, 
		   ...){

  /* Nothing to do if no files */
  if(nb_files <= 0){
    return ;
  }

  /* Get filenames */
  const char ** file_names =
    (const char **) malloc(nb_files * sizeof(const char *));

  file_names[0] = source_file1;

  va_list va;
  va_start(va, source_file1);
  unsigned long i = 0;
  for (i = 1; i < nb_files; ++i){
    file_names[i] = va_arg (va, const char *);
  }
  va_end (va);

  SU_splitFile2(source_file, nb_line_per_file, nb_files, file_names);

  /* Cleaning */
  free(file_names);
}

/********************** File loading ************/

/* Structure for fileLoadR */
typedef struct SU_fileLoad_structure SU_fileLoad_structure;
struct SU_fileLoad_structure {
  /* IO buffer */
  char small_buffer[SU_MAXLINESIZE];
  char *buffer;
  size_t buffer_size;
  /* data buffer */
  char small_buffer_data[SU_MAXLINESIZE];
  char *buffer_data;
  size_t buffer_data_size;
  /* file info */
  int fd;
  int open;
  /* iteration variables */
  ssize_t len; /* number of signifiant data in buffer */
  char * p; /* current char looked in buffer */
  char * val_begin; /* where the currrent value start */

  char * data_end; /* where the current data end */
};


void SU_fileLoad_structureInit(const char * file, SU_fileLoad_structure * fls) {

  /* Open the file */
  if ((file == NULL) || (fls == NULL)) {
    err(1, "SU_fileLoad_structureInit (%s:%d)", __FILE__, __LINE__ );
  }

  if ((fls->fd = open(file, O_RDONLY, 0)) < 0) {
    err(EX_IOERR, "read (%s:%d)", __FILE__, __LINE__ );
  }

  fls->buffer = fls->small_buffer;
  fls->buffer_size = SU_MAXLINESIZE;

  fls->buffer_data = fls->small_buffer_data;
  fls->buffer_data_size = SU_MAXLINESIZE;

  fls->open = 1;

  fls->len = 0;
  fls->p = fls->buffer;
  fls->val_begin = fls->buffer;

  fls->data_end = fls->buffer_data;
}

void SU_fileLoad_structureEnd(SU_fileLoad_structure * fls) {

  if(fls->open){
    (void) close (fls->fd);
    fls->open = 0;
  }
}

int SU_loadFile_R (SU_fileLoad_structure * fls, int * end_loading ) {


  if((end_loading == NULL) || (fls == NULL)){
    err(1, "SU_loadFile_R (%s:%d)", __FILE__, __LINE__ );
  }

  /* Check the file is open */
  if (!fls->open){
    *end_loading = 1;
    return -1;
  }

  while(1){

    /* Check need to read the file */
    if(fls->len == 0){

      fls->len = read(fls->fd, fls->buffer, fls->buffer_size);
      if (fls->len == -1) {
	SU_fileLoad_structureEnd(fls);
	err(EX_IOERR, "read (%s:%d)", __FILE__, __LINE__ );
      }

      if(fls->len == 0){
	/* No more things to read */
	/* Next call will return end_loading anyway */
	SU_fileLoad_structureEnd(fls);

	/* may have a remaining value in data buffer */
	*(fls->data_end) = '\0';
	*end_loading = (fls->data_end == fls->buffer_data);
	fls->data_end = fls->buffer_data;

	/* return last data value if any else end */
	return atoi(fls->buffer_data);
      } else {
	/* reinit buffer processing */
	fls->val_begin = fls->buffer;
	fls->p = fls->buffer;
      }
    }

    /* Else need to work on buffer */
    while(fls->len) {

      /* process the buffer */
      /* len != 0 */
      if(*(fls->p) == '\n'){
	size_t sb = (size_t) (fls->p - fls->val_begin +1);
	if(sb > 0) {
	  /* Extract the data */
	  memcpy(fls->data_end, fls->val_begin, sb*sizeof(char));
	  fls->data_end = fls->data_end + sb;
	}

	/* prepare next iteration */
	++ fls->p;
	-- fls->len;
	fls->val_begin = fls->p;
	*(fls->data_end+1)  = '\0';
	fls->data_end = fls->buffer_data;


	/* return value value */
	*end_loading = 0;
	return atoi(fls->buffer_data);
       }

      /* Go to next char in buffe */
      ++ fls->p ;
      -- fls->len;
    }

    /* save buffer to data */
    size_t sb = (size_t) (fls->p - fls->val_begin);
    if(sb > 0) {
      memcpy(fls->data_end, fls->val_begin, sb*sizeof(char));
      fls->data_end = fls->data_end + sb;
      // *(fls->data_end) = '\0'; /* Tmp value, wille be override */
    }

  }

  /* Should not be possible to end here ! */
  *end_loading = 1;
  return -1;
}

unsigned long SU_loadFile(const char * file, int ** data) {

  SU_fileLoad_structure fls;
  SU_fileLoad_structureInit(file, &fls);

  /* Set result array */
  ssize_t data_size = -1;
  size_t data_size_reserve = 10;
  *data = (int *) malloc(data_size_reserve * sizeof(int));
  assert(*data != NULL);

  unsigned long nb_value = 0;
  int end_load = 0;
  while(!end_load) {

    int val = SU_loadFile_R(&fls, &end_load);
    if(!end_load) {

      /* Add in result array (and realloc if needed) */
      if(data_size + 1 >= (ssize_t) data_size_reserve) {
	data_size_reserve *= 2;
	int * tmp_data = realloc(*data, data_size_reserve * sizeof(int));
	if(tmp_data == NULL){
	  SU_fileLoad_structureEnd(&fls);
	  err(1, "realloc (%s:%d)", __FILE__, __LINE__ );
	}
	*data = tmp_data;
      }

      /* Take new value into account */
      (*data)[++data_size] = val;
      ++ nb_value;
    }
  }

  SU_fileLoad_structureEnd(&fls);
  return nb_value;
}


/********************** File saving ************/
/* Structure for fileLoadR */
typedef struct SU_fileSave_structure SU_fileSave_structure;
struct SU_fileSave_structure {
  /* IO buffer */
  char small_buffer[SU_MAXLINESIZE];
  char *buffer;
  off_t buffer_size;
  /* file info */
  int fd;
  int open;
  /* iteration variables */
  char * buffer_end; /* where the current data end */
};


void SU_fileSave_structureInit(const char * file, SU_fileSave_structure * fss) {

  /* Open the file */
  if ((file == NULL) || (fss == NULL)) {
    err(1, "SU_fileSave_structureInit (%s:%d)", __FILE__, __LINE__ );
  }

  if ((fss->fd = open(file , O_WRONLY | O_CREAT | O_TRUNC ,
		      S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH )) < 0){
    err(EX_NOINPUT, "open (%s:%d)", __FILE__, __LINE__ );
  }

  fss->buffer = fss->small_buffer;
  fss->buffer_size = SU_MAXLINESIZE;

  fss->open = 1;

  fss->buffer_end = fss->buffer;
}

void SU_fileSave_structureEnd(SU_fileSave_structure * fss) {

  if(fss->open){
    /* Write data in buffer if any */
    if(fss->buffer_end != fss->buffer) {
      *(fss->buffer_end++)  = '\0';
      ssize_t sb = fss->buffer_end - fss->buffer - 1;
      if (write(fss->fd, fss->buffer, (size_t) sb) != sb) {
	err(EX_IOERR, "write SU_fileSave_structureEnd (%s:%d)", __FILE__, __LINE__ );
      }
    }

    (void) close (fss->fd);
    fss->open = 0;
  }
}

void SU_saveFile_R (int value, SU_fileSave_structure * fss) {

  if(fss == NULL){
    err(1, "SU_saveFile_R (%s:%d)", __FILE__, __LINE__ );
  }

  /* Check the file is open */
  if (!fss->open){
    return;
  }

  /* If no space in buffer, write it */
  if(fss->buffer + fss->buffer_size - fss->buffer_end < 50){ /* Keep margin */

    ssize_t sb = fss->buffer_end - fss->buffer;
    if (write(fss->fd, fss->buffer, (size_t) sb) != sb) {
      err(EX_IOERR, "write (%s:%d)", __FILE__, __LINE__ );
    }

    fss->buffer_end = fss->buffer;
  }

  /* Add value in buffer */
  fss->buffer_end += sprintf(fss->buffer_end, "%d", value);
  *(fss->buffer_end++) = '\n';
}


void SU_saveFile(const char * file, unsigned long data_size, const int * data) {

  SU_fileSave_structure fss;
  SU_fileSave_structureInit(file, &fss);

  unsigned long i = 0;
  for (i =0; i < data_size; ++i) {
    SU_saveFile_R(data[i], &fss);
  }

  SU_fileSave_structureEnd(&fss);
}


/********************** Merge file ************/
void SU_randomGeneration(const char * file_target, unsigned long nb_elems){

  SU_fileSave_structure fss;
  SU_fileSave_structureInit(file_target, &fss);

  unsigned long i = 0;
  for(i = 0; i < nb_elems; ++i){
    int val = rand();
    SU_saveFile_R(val, &fss);
  }

  SU_fileSave_structureEnd(&fss);
}

/********************** Merge file ************/

void SU_mergeSortedFiles(const char * source_file1,
			 const char * source_file2,
			 const char * file_target){

  SU_fileSave_structure fss;
  SU_fileSave_structureInit(file_target, &fss);

  SU_fileLoad_structure fls1;
  SU_fileLoad_structureInit(source_file1, &fls1);
  int end_file_1 = 0;

  SU_fileLoad_structure fls2;
  SU_fileLoad_structureInit(source_file2, &fls2);
  int end_file_2 = 0;

  int val1 = SU_loadFile_R(&fls1, &end_file_1);
  int val2 = SU_loadFile_R(&fls2, &end_file_2);

  while(!(end_file_1 && end_file_2)) {
    if(!end_file_1 && !end_file_2) {
      if(val1 < val2){
	SU_saveFile_R(val1, &fss);
	val1 = SU_loadFile_R(&fls1, &end_file_1);
      } else {
	SU_saveFile_R(val2, &fss);
	val2 = SU_loadFile_R(&fls2, &end_file_2);
      }
    } else if(!end_file_1) {
      SU_saveFile_R(val1, &fss);
      val1 = SU_loadFile_R(&fls1, &end_file_1);
    } else { /*!end_file_2*/
      SU_saveFile_R(val2, &fss);
      val2 = SU_loadFile_R(&fls2, &end_file_2);
    }
  }

  SU_fileSave_structureEnd(&fss);
}

/********************** Data sort algorithms ************/

void SU_QSort(unsigned long data_size, int * data) {
  if (data_size < 2)
    return;

  /* pick random pivot */
  int pivot = data[((unsigned long)rand())%(data_size -1)];

  int *left = data;
  int *right = data + data_size - 1;

  /* Partition around pivot */
  while (left <= right) {
    /* Search too big element */
    if (*left < pivot) {
      left++;
    }
    /* Search too small element */
    else if (*right > pivot) {
      right--;
    }
    else {
      /* swap value */
      int tmp = *left;
      *left = *right;
      *right = tmp;
      left++;
      right--;
    }
  }

  /* Recursive both side of the pivot */
  SU_QSort((unsigned long)(right - data + 1), data);
  SU_QSort((unsigned long)(data + data_size - left), left);

}

void SU_ISort(unsigned long data_size, int * data) {

  unsigned long i;
  unsigned long j;
  int value;

  for(i = 1; i < data_size; ++i) {
    value = data[i];
    for (j = i-1; j > 0 && data[j] > value; j--){
      data[j+1] = data[j];      
    }
    if(j == 0 && data[j] > value){
      data[j] = value;
    } 
    else{
      data[j+1] = value;
    }
  }
}

void SU_HSort(unsigned long data_size, int * data) {

  int t; /* the temporary value */
  unsigned long n = data_size;
  unsigned long parent = data_size/2;
  unsigned long index, child; /* heap indexes */

  /* loop until array is sorted */
  while (1) {

    if (parent > 0) {
      /* first stage - Sorting the heap */
      t = data[--parent];  /* save old value to t */
    } else {
      /* second stage - Extracting elements in-place */
      n--;		  /* make the heap smaller */
      if (n == 0) {
	return; /* When the heap is empty, we are done */
      }
      t = data[n];	  /* save lost heap entry to temporary */
      data[n] = data[0];	  /* save root entry beyond heap */
    }

    /* insert operation - pushing t down the heap to replace the parent */
    index = parent; /* start at the parent index */
    child = index * 2 + 1; /* get its left child index */
    while (child < n) {
      /* choose the largest child */
      if (child + 1 < n	 &&  data[child + 1] > data[child]) {
	child++; /* right child exists and is bigger */
      }
      /* is the largest child larger than the entry? */
      if (data[child] > t) {
	data[index] = data[child]; /* overwrite entry with child */
	index = child; /* move index to the child */
	child = index * 2 + 1; /* get the left child and go around again */
      } else {
	break; /* t's place is found */
      }
    }
    /* store the temporary value at its new location */
    data[index] = t;
  }
}

/********************** Mis ************/

void SU_test(){

  fprintf(stderr, "Test system_utils \t [OK].\n");
}
