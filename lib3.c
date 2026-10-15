
#define _GNU_SOURCE

#include <stdio.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <string.h>
#include <stdarg.h>
#include <sys/mman.h>
#include <unistd.h>

typedef int (*open_fn)(const char*, int, ...);
typedef int (*openat_fn)(int, const char*, int, ...);
typedef FILE* (*fopen_fn)(const char*, const char*);

static open_fn orig_open = NULL;
static openat_fn orig_openat = NULL;
static fopen_fn orig_fopen = NULL;

static dev_t target_dev;
static ino_t target_ino;
static int target_ready = 0;

static const char fake_content[] = "This is the fake content of the file\n";

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
    if (fstatat(dirfd, filename, &info, 0) == -1) return -1;
    *ino = info.st_ino;
    *dev = info.st_dev;
    return 0;
}

static int create_fake(void) {
    int fd = memfd_create("lab3", 0);
    if (fd == -1) return -1;

    ssize_t written = write(fd, fake_content, sizeof(fake_content) - 1);

    if (written != (ssize_t)(sizeof(fake_content) - 1) || lseek(fd, 0, SEEK_SET) == -1) {
        close(fd);
        return -1;
    }

    return fd;
}

__attribute__((constructor)) void init_open(void) {
    orig_open = dlsym(RTLD_NEXT, "open");
    orig_openat = dlsym(RTLD_NEXT, "openat");
    orig_fopen = dlsym(RTLD_NEXT, "fopen");

    target_ready = (getinodev("/tmp/aaa", &target_ino, &target_dev) == 0);
}

static int is_target(ino_t ino, dev_t dev) {
    return target_ready && ino == target_ino && dev == target_dev;
}

static int read_only(int flags) {
    return (flags & O_ACCMODE) == O_RDONLY && !(flags & (O_CREAT | O_TRUNC | O_PATH));
}

static int needs_mode(int flags) {
    return (flags & O_CREAT) || ((flags & O_TMPFILE) == O_TMPFILE);
}

int open(const char* pathname, int flags, ...) {
    ino_t ino;
    dev_t dev;

    if (read_only(flags) && getinodev(pathname, &ino, &dev) == 0 && is_target(ino, dev)) {
        return create_fake();
    }

    if (needs_mode(flags)) {
        va_list args;
        va_start(args, flags);
        mode_t mode = va_arg(args, mode_t);
        va_end(args);
        return orig_open(pathname, flags, mode);
    }

    return orig_open(pathname, flags);
}

int openat(int dirfd, const char *pathname, int flags, ...) {
    ino_t ino;
    dev_t dev;

    if (read_only(flags) && getinodevat(dirfd, pathname, &ino, &dev) == 0 && is_target(ino, dev)) {
        return create_fake();
    }

    if (needs_mode(flags)) {
        va_list args;
        va_start(args, flags);
        mode_t mode = va_arg(args, mode_t);
        va_end(args);
        return orig_openat(dirfd, pathname, flags, mode);
    }

    return orig_openat(dirfd, pathname, flags);
}

FILE* fopen(const char *restrict pathname, const char *restrict mode) {
    ino_t ino;
    dev_t dev;

    if (mode && mode[0] == 'r' && !strchr(mode, '+') && getinodev(pathname, &ino, &dev) == 0 && is_target(ino, dev)) {
        int fd = create_fake();
        if (fd == -1) return NULL;

        FILE *file = fdopen(fd, mode);
        if (file == NULL) close(fd);

        return file;
    }

    return orig_fopen(pathname, mode);
}
