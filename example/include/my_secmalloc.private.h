#ifndef MY_SECMALLOC_PRIVATE_H
#define MY_SECMALLOC_PRIVATE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

// Define a structure to store information about a freed pointer
typedef struct {
  void *ptr;
  bool freed;
} freed_pointer_t;

// Define a structure to represent a block descriptor
typedef struct {
  void *ptr;       // Pointer to the allocated memory block
  size_t size;     // Size of the memory block
  bool free;       // Flag indicating whether the block is free or not
  uint32_t canary; // Canary value to detect buffer overflows
  bool reused;
} block_descriptor_t;

// Function prototypes
void generate_report(const char *operation, size_t size, void *address);
void initialize_pools();
void initialize_freed_pointers();
void initialize_canary();
void initialize();
void *my_malloc(size_t size);
void *my_calloc(size_t nmemb, size_t size);
void *my_realloc(void *ptr, size_t size);
bool check_double_free(void *ptr);
void mark_as_freed(void *ptr);
void my_free(void *ptr);
void check_canaries();

#endif /* MY_SECMALLOC_PRIVATE_H */
