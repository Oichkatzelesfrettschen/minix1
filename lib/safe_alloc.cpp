#include "../include/lib.hpp" // C++17 header
#include <stdio.h>
#include <stdlib.h>

/* Allocate memory and abort on failure. */
void *safe_malloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr == NULL) {
        fprintf(stderr, "out of memory\n");
        exit(1);
    }
    return ptr;
}

/* Free memory if pointer not NULL. */
void safe_free(void *ptr) {
    if (ptr != NULL) {
        free(ptr);
    }
}
