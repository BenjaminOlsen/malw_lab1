// The _GNU_SOURCE feature test macro must be defined in order to obtain 
// the  definitions  of  RTLD_DEFAULT and RTLD_NEXT from <dlfcn.h>.
#define  _GNU_SOURCE

#include <stdio.h>
#include <dirent.h>
#include <dlfcn.h>

// 1.- Define a static global function pointer on your lib (initially set to null) of a 
// orig_readdir function with the same signature as readdir. Use man to discover
// the signature
static struct dirent *(*orig_readdir) (DIR *) = NULL;

// 2.- When the library is loaded, we should initialize the orig_readdir
// function pointer to the original readdir function
__attribute__ ((constructor)) void initialization()
{
      orig_readdir = dlsym (RTLD_NEXT, "readdir");
      fprintf (stderr, "Catched! %p\n\n", orig_readdir);
}

// 3.- Define a readdir function following the original specification
struct dirent *readdir (DIR * dirp)
{

// 4.- Now the easy part, invoke the orig_readdir function as it was legitimate
  struct dirent *tmp;
  tmp = orig_readdir (dirp);

// 5.- Check the given process name, if the process name is the process we want to hide, just
// iterate again to the next dir entry and return it effectively skipping the hidden process
//
// TO DO !!!!
//

  return (tmp);
}
