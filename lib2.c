/* MALW LAB1 PT2.

usage:

(1) build
$ make lib2.so

(2) intercept open using defaults (target = /tmp/aaa, source = /tmp/bbb)
$ LD_PRELOAD=./lib2.so cat aaa

(3) use ./aaa and ./bbb instead of /tmp/aaa & /tmp/bbb 
$ LD_PRELOAD=./lib2.so SOURCE=bbb TARGET=aaa cat aaa 

(4) intercept openat:
$ LD_PRELOAD=./lib2.so grep --color=always e aaa

(5) intercept fopen:
$ LD_PRELOAD=./lib2.so more aaa

(6) clean:
$ make clean
*/

#include <stdio.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

typedef int (*open_fn)(const char*, int, /*mode_t*/ ...);
typedef int (*openat_fn)(int dirfs, const char*, int, ...);
typedef FILE* (*fopen_fn)(const char *restrict, const char *restrict);

static open_fn orig_open = NULL;
static openat_fn orig_openat = NULL;
static fopen_fn orig_fopen = NULL;

/* to help resolve relative paths to absolute paths: */
static dev_t target_dev, source_dev;
static ino_t target_ino, source_ino;

static const char* target_filename = NULL;
static const char* source_filename = NULL;

static int getinodev(const char *filename, ino_t *ino, dev_t *dev) {
    if (!filename || !ino || !dev) return -1;

    struct stat info;
    if (stat(filename, &info) == -1) return -1;
    *ino = info.st_ino;
    *dev = info.st_dev;
    return 0;
}

static int getinodevat(int dirfd, const char *filename, ino_t *ino, dev_t *dev) {
    if (!filename || !ino || !dev) return -1;
    struct stat info;
    if (fstatat(dirfd, filename, &info, /* flags: follow symlinks: */ 0) == -1) return -1;
    *ino = info.st_ino;
    *dev = info.st_dev;
    return 0;
} 

__attribute__ ((constructor)) void init_open() {
    orig_open = dlsym(RTLD_NEXT, "open");
    orig_openat = dlsym(RTLD_NEXT, "openat");
    orig_fopen = dlsym(RTLD_NEXT, "fopen");

    const char *tgt = getenv("TARGET");
    const char *src = getenv("SOURCE");
    target_filename = strdup(tgt != NULL ? tgt : "/tmp/aaa");
    source_filename = strdup(src != NULL ? src : "/tmp/bbb");

    if (getinodev(target_filename, &target_ino, &target_dev) != 0) {
        fprintf(stderr, "can't stat %s\n", target_filename);
        return;
    }
    if (getinodev(source_filename, &source_ino, &source_dev) != 0) {
        fprintf(stderr, "cant stat %s\n", source_filename);
        return;
    }
    // fprintf(stdout, "init ok\n");
}


int open(const char* pathname, int flags, ... /* mode_t mode */) {
    
    ino_t ino;
    dev_t dev;

    if (getinodev(pathname, &ino, &dev) != 0) {
        return -1;
    }

    if (ino == target_ino && dev == target_dev) {
        // printf("doing the switcheroo %s <- %s\n", target_filename, source_filename);
        pathname = source_filename;
    }

    /* have to do this for var args */
    if (flags & O_CREAT) {  /* O_TMPFILE too? */
        /* a mode argument must exist, so reading it is valid */
        va_list args;
        va_start(args, flags);
        mode_t mode = va_arg(args, mode_t);
        va_end(args);
        return orig_open(pathname, flags, mode);
    } else {
        return orig_open(pathname, flags);
    }

    return 0;
}

int openat(int dirfd, const char *pathname, int flags, ... /* mode_t mode*/) {

    // printf("got openat\n");
    ino_t ino;
    dev_t dev; 
    if (getinodevat(dirfd, pathname, &ino, &dev) != 0) {
        //fprintf(stderr, "problem getting ino, dev for dir %d, %s\n", dirfd, pathname);
        return -1;
    }

    if (ino == target_ino && dev == target_dev) {
        // printf("doing the switcheroo %s <- %s\n", target_filename, source_filename);
        pathname = source_filename;
    }
    if (flags & O_CREAT) {
        va_list args;
        va_start(args, flags);
        mode_t mode = va_arg(args, mode_t);
        va_end(args);
        return orig_openat(dirfd, pathname, flags, mode);
    } else {
        return orig_openat(dirfd, pathname, flags);
    }
}

FILE* fopen(const char *restrict pathname, const char *restrict mode) {
    // printf("got fopen\n");
    ino_t ino;
    dev_t dev;
    if (getinodev(pathname, &ino, &dev) != 0) {
        // fprintf(stderr, "problem getting inode, devide for %s\n", pathname);
        return NULL;
    }
    
    if (ino == target_ino && dev == target_dev) {
        // printf("swithcing fopen target %s <- %s\n", target_filename, source_filename);
        pathname = source_filename;
    }
    return orig_fopen(pathname, mode);
}

