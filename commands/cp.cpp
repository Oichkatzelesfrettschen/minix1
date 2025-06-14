/*<<< WORK-IN-PROGRESS MODERNIZATION HEADER
  This repository is a work in progress to reproduce the
  original MINIX simplicity on modern 32-bit and 64-bit
  ARM and x86/x86_64 hardware using C++23.
>>>*/

/* cp - copy files	Author: Andy Tanenbaum */

#include "stat.hpp"
#include <array>

#define TRANSFER_UNIT 16384
std::array<char, TRANSFER_UNIT> cpbuf{};
int isfloppy; /* set to 1 for cp x /dev/fd? */

int main(int argc, char *argv[]) {
    int fd1, fd2, m, s;
    struct stat sbuf, sbuf2;

    if (argc < 3)
        usage();

    /* Get the status of the last named file.  See if it is a directory. */
    s = stat(argv[argc - 1], &sbuf);
    m = sbuf.st_mode & S_IFMT;
    if (s >= 0 && m == S_IFDIR) {
        /* Last argument is a directory. */
        cp_to_dir(argc, argv);
    } else if (argc > 3) {
        /* More than 2 arguments and last one is not a directory. */
        usage();
    } else if (s < 0 || m == S_IFREG || m == S_IFCHR || m == S_IFBLK) {
        /* Exactly two arguments.  Check for cp f1 f1. */
        if (equal(argv[1], argv[2])) {
            std_err("Cannot copy a file to itself\n");
            exit(-1);
        }

        /* Command is of the form cp f1 f2. */
        fd1 = open(argv[1], 0);
        if (fd1 < 0) {
            stderr3("Cannot open ", argv[1], "\n");
            exit(1);
        }
        fstat(fd1, &sbuf);
        fd2 = creat(argv[2], sbuf.st_mode & 0777);
        if (fd2 < 0) {
            stderr3("Cannot create ", argv[2], "\n");
            exit(2);
        }
        fstat(fd2, &sbuf2);
        if ((sbuf2.st_mode & S_IFMT) == S_IFBLK)
            isfloppy = 1;
        copyfile(fd1, fd2);
    } else {
        stderr3("Cannot copy to ", argv[2], "\n");
        exit(3);
    }
}

static void cp_to_dir(int argc, char *argv[]) {
    int i, fd1, fd2;
    std::array<char, 256> dirname{};
    char *ptr, *dp;
    struct stat sbuf;

    for (i = 1; i < argc - 1; i++) {
        fd1 = open(argv[i], 0);
        if (fd1 < 0) {
            stderr3("Cannot open ", argv[i], "\n");
            continue;
        }

        ptr = argv[argc - 1];
        dp = dirname.data();
        while (*ptr != 0)
            *dp++ = *ptr++;

        *dp++ = '/';
        ptr = argv[i];

        /* Concatenate dir and file name in dirname buffer. */
        while (*ptr != 0)
            ptr++; /* go to end of file name */
        while (ptr > argv[i] && *ptr != '/')
            ptr--; /* get last component*/
        if (*ptr == '/')
            ptr++;
        while (*ptr != 0)
            *dp++ = *ptr++;
        *dp++ = 0;
        fstat(fd1, &sbuf);
        fd2 = creat(dirname.data(), sbuf.st_mode & 0777);
        if (fd2 < 0) {
            stderr3("Cannot create ", dirname.data(), "\n");
            continue;
        }
        copyfile(fd1, fd2);
    }
}

static void copyfile(int fd1, int fd2) {
    int n, m;

    do {
        n = read(fd1, cpbuf, TRANSFER_UNIT);
        if (n < 0) {
            std_err("Write error\n");
            break;
        }
        if (n > 0) {
            m = write(fd2, cpbuf, n);
            if (m != n) {
                perror("cp");
                exit(1);
            }
            if (isfloppy)
                sync(); /* purge the cache all at once */
        }
    } while (n == TRANSFER_UNIT);
    close(fd1);
    close(fd2);
}

static void usage(void) {
    std_err("Usage:  cp f1 f2;  or  cp f1 ... fn d2\n");
    exit(-1);
}

static int equal(char *s1, char *s2) {
    while (1) {
        if (*s1 == 0 && *s2 == 0)
            return (1);
        if (*s1 != *s2)
            return (0);
        if (*s1 == 0 || *s2 == 0)
            return (0);
        s1++;
        s2++;
    }
}

static int match(char *s1, char *s2, int n) {
    while (n--) {
        if (*s1++ != *s2++)
            return (0);
    }
    return (1);
}

static void stderr3(char *s1, char *s2, char *s3) {
    std_err(s1);
    std_err(s2);
    std_err(s3);
}
