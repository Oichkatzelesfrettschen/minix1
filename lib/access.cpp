#include "../include/lib.hpp" // C++23 header

// Wrapper around the ACCESS system call using the new interface.
//
// Parameters:
//   name - path to test
//   mode - access mode bits
//
// Returns 0 on success or -1 with errno set on failure.
int access(const char *name, int mode) {
    // Delegate to the file system server via callm3.
    return callm3(FS, ACCESS, mode, const_cast<char *>(name));
}
