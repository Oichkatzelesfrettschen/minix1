#include "../include/lib.hpp" // C++23 header
#include "../include/stdio.hpp"
#include <fcntl.h>

#define PMODE 0644

/*
 * Open a file as a stream.
 *
 * Supported modes:
 *      "w" - create/truncate for writing
 *      "a" - open for appending (create if needed)
 *      "r" - open for reading
 */
FILE *fopen(const char *name, const char *mode) {
    int i;         /* index into _io_table */
    FILE *fp;      /* resulting stream */
    int fd;        /* file descriptor from open/creat */
    int flags = 0; /* stream flags */

    /* Locate a free slot in the open file table. */
    for (i = 0; _io_table[i] != 0; i++) {
        if (i >= NFILES)
            return NULL;
    }

    /* Decide how to open or create the file. */
    switch (*mode) {
    case 'w':
        /* Create or truncate the file for writing. */
        flags |= WRITEMODE;
        fd = creat(name, PMODE);
        if (fd < 0)
            return NULL;
        break;

    case 'a':
        /* Open for appending; create if necessary. */
        flags |= WRITEMODE;
        fd = open(name, 1);
        if (fd < 0)
            return NULL;
        lseek(fd, 0L, 2);
        break;

    case 'r':
        /* Open an existing file for reading. */
        flags |= READMODE;
        fd = open(name, 0);
        if (fd < 0)
            return NULL;
        break;

    default:
        /* Unrecognized mode. */
        return NULL;
    }

    /* Allocate the FILE structure. */
    fp = (FILE *)safe_malloc(sizeof(FILE));

    /* Initialize the stream structure. */
    fp->_count = 0;
    fp->_fd = fd;
    fp->_flags = flags;
    fp->_buf = static_cast<char *>(safe_malloc(BUFSIZ));
    fp->_flags |= IOMYBUF;

    fp->_ptr = fp->_buf;
    _io_table[i] = fp;
    return fp;
}
