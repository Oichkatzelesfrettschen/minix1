/*<<< WORK-IN-PROGRESS MODERNIZATION HEADER
  This repository is a work in progress to reproduce the
  original MINIX simplicity on modern 32-bit and 64-bit
  ARM and x86/x86_64 hardware using C++23.
>>>*/

/* mknod - build a special file		Author: Andy Tanenbaum */

// Entry point using modern signature
int main(int argc, char *argv[]) {
    /* mknod name b/c major minor makes a node. */

    int mode, major, minor;

    if (argc != 5)
        badcomm();
    if (*argv[2] != 'b' && *argv[2] != 'c')
        badcomm();
    mode = (*argv[2] == 'b' ? 060666 : 020666);
    major = atoi(argv[3]);
    minor = atoi(argv[4]);
    if (major < 0 || minor < 0)
        badcomm();
    if (mknod(argv[1], mode, (major << 8) | minor) < 0)
        perror("mknod");
    return 0;
}

// Convert ASCII digits to an integer value
static int atoi(const char *p) {
    /* Ascii to integer conversion. */
    int c, n;

    n = 0;
    while (c = *p++) {
        if (c < '0' || c > '9')
            return (-1);
        n = 10 * n + (c - '0');
    }
    return (n);
}

// Print usage and terminate the program
[[noreturn]] static void badcomm() {
    std_err("Usage: mknod name b/c major minor\n");
    exit(1);
}
