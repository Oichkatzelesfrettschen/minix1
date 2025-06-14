// Modernized for C++23

/* update - do sync periodically		Author: Andy Tanenbaum */

#include "signal.hpp"

// Entry point using modern C++ signature
int main() {
    int fd, buf[2];

    /* Disable SIGTERM */
    signal(SIGTERM, SIG_IGN);

    /* Open some files to hold their inodes in core. */
    close(0);
    close(1);
    close(2);

    open("/bin", 0);
    open("/lib", 0);
    open("/etc", 0);
    open("/tmp", 0);

    /* Flush the cache every 30 seconds. */
    while (1) {
        sync();
        sleep(30);
    }

    return 0; // Unreachable but keeps the compiler happy
}
