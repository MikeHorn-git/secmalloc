#include "my_secmalloc.private.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#define INITIAL_POOL_SIZE (1024 * 1024) // 1MB
#define MAX_FREED_POINTERS                                                     \
  100 // Define the maximum number of freed pointers to track
void __attribute__((destructor)) clean();

static block_descriptor_t *meta_pool = NULL;
static void *data_pool = NULL;
static size_t meta_pool_size = 0;
static size_t data_pool_size = 0;
static freed_pointer_t
    freed_pointers[MAX_FREED_POINTERS]; // Array to store information
                                        // about freed pointers

static unsigned int CANARY_VALUE;

// Log function executed if MSM_OUTPUT env is set
void generate_report(const char *operation, size_t size, void *address) {
  char const *msm_output = getenv("MSM_OUTPUT");

  if (msm_output) {
    FILE *log_file = fopen(msm_output, "a");
    if (log_file) {
      fprintf(log_file, "[LOG] %s: Size=%zu, Address=%p\n", operation, size,
              address);
      fclose(log_file);
    } else {
      fprintf(stderr, "Error : Failed to open log file%s\n", msm_output);
    }
  }
}

// Map pages of memory
void initialize_pools(void) {
  if (!meta_pool) {
    meta_pool_size = INITIAL_POOL_SIZE / sizeof(block_descriptor_t);
    meta_pool =
        mmap(NULL, meta_pool_size * sizeof(block_descriptor_t),
             PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (meta_pool == MAP_FAILED) {
      perror("mmap meta_pool");
      exit(EXIT_FAILURE);
    }
    memset(meta_pool, 0, meta_pool_size * sizeof(block_descriptor_t));
  }

  if (!data_pool) {
    data_pool_size = INITIAL_POOL_SIZE;
    data_pool = mmap(NULL, data_pool_size, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (data_pool == MAP_FAILED) {
      perror("mmap data_pool");
      exit(EXIT_FAILURE);
    }
  }
}

void initialize_freed_pointers(void) {
  for (size_t i = 0; i < MAX_FREED_POINTERS; i++) {
    freed_pointers[i].ptr = NULL;
    freed_pointers[i].freed = false;
  }
}

void initialize_canary(void) {
  srand((unsigned int)time(NULL)); // Seed the random number generator
  CANARY_VALUE = rand();           // Generate a random canary value
}

void initialize(void) {
  initialize_pools();          // Initialize pools
  initialize_freed_pointers(); // Initialize the array of freed pointers
  initialize_canary();         // Initialize the canary value
}

void *my_malloc(size_t size) {
  initialize();

  if (size == 0)
    size = 1; // Allocate a minimum of 1 byte for zero-size requests

  // Search for a free block of sufficient size
  for (size_t i = 0; i < meta_pool_size; i++) {
    if (meta_pool[i].free && meta_pool[i].size >= size &&
        !meta_pool[i].reused) {
      meta_pool[i].free = 0;
      meta_pool[i].reused = true; // Mark the block as reused
      meta_pool[i].canary = CANARY_VALUE;
      generate_report("malloc", size, meta_pool[i].ptr);
      return meta_pool[i].ptr;
    }
  }

  // Allocate a new block if no free block of sufficient size is found
  for (size_t i = 0; i < meta_pool_size; i++) {
    if (meta_pool[i].size == 0) {
      void *ptr = (char *)data_pool + (i * size);
      meta_pool[i].ptr = ptr;
      meta_pool[i].size = size;
      meta_pool[i].free = 0;
      meta_pool[i].reused = true; // Mark the block as reused
      meta_pool[i].canary = CANARY_VALUE;
      generate_report("malloc", size, meta_pool[i].ptr);
      return ptr;
    }
  }

  // If no free descriptor is found, enlarge the pools with mremap
  size_t new_size = meta_pool_size * 2; // Double the size

  void *new_data_pool =
      mremap(data_pool, data_pool_size, data_pool_size * 2, 0);

  block_descriptor_t *new_meta_pool =
      mremap(meta_pool, meta_pool_size * sizeof(block_descriptor_t),
             new_size * sizeof(block_descriptor_t), 0);

  if (new_data_pool == MAP_FAILED || new_meta_pool == MAP_FAILED) {
    perror("Failed to resize memory pools");
    return NULL;
  }

  // Update pointers and sizes
  data_pool = new_data_pool;
  meta_pool = new_meta_pool;
  data_pool_size *= 2;
  meta_pool_size = new_size;

  // Initialize newly added meta pool blocks
  for (size_t i = meta_pool_size / 2; i < meta_pool_size; i++) {
    meta_pool[i].ptr = NULL;
    meta_pool[i].size = 0;
    meta_pool[i].free = 1;
    meta_pool[i].reused = false;
    meta_pool[i].canary = 0;
  }

  // Retry allocation after resizing
  return my_malloc(size);
}

void *my_calloc(size_t nmemb, size_t size) {
  size_t total_size = nmemb * size;
  if (total_size == 0)
    total_size = 1; // Allocate a minimum of 1 byte for zero-size requests

  void *ptr = my_malloc(total_size);
  generate_report("calloc", total_size, ptr);
  if (ptr) {
    // Initialize the allocated memory block to zero
    memset(ptr, 0, total_size);
  } else {
    fprintf(stderr, "Failed to allocate memory.\n");
  }
  return ptr;
}

void *my_realloc(void *ptr, size_t size) {
  if (ptr == NULL)
    return my_malloc(size);

  if (size == 0) {
    my_free(ptr);
    return NULL;
  }

  for (size_t i = 0; i < meta_pool_size; i++) {
    if (size > meta_pool[i].size) {
      if (meta_pool[i].size >= size + sizeof(unsigned int)) {
        generate_report("realloc", size, ptr);
        return ptr;
      } else {
        size_t delta = size - meta_pool[i].size;
        if (i + 1 < meta_pool_size && meta_pool[i + 1].free &&
            (meta_pool[i].size + meta_pool[i + 1].size) >=
                meta_pool[i].size + delta) {
          meta_pool[i].size += delta;
          meta_pool[i].free = 0;
          meta_pool[i + 1].ptr += delta;
          meta_pool[i + 1].size -= delta;
          meta_pool[i + 1].free = 1;
          *(unsigned int *)((char *)meta_pool[i].ptr + size) = CANARY_VALUE;
          generate_report("realloc", size, ptr);
          return ptr;
        }
      }
      void *new_ptr = my_malloc(size);
      if (new_ptr) {
        memcpy(new_ptr, ptr, meta_pool[i].size - sizeof(unsigned int));
        my_free(ptr);
        generate_report("realloc", size, new_ptr);
        return new_ptr;
      }
    }
  }
  return NULL;
}

// Check if a pointer has already been freed
bool check_double_free(void *ptr) {
  for (size_t i = 0; i < MAX_FREED_POINTERS; i++) {
    if (freed_pointers[i].ptr == ptr && freed_pointers[i].freed) {
      return true; // Pointer has already been freed
    }
  }
  return false; // Pointer has not been freed
}

// Function to mark a pointer as freed
void mark_as_freed(void *ptr) {
  for (size_t i = 0; i < MAX_FREED_POINTERS; i++) {
    if (freed_pointers[i].ptr == NULL) {
      freed_pointers[i].ptr = ptr;
      freed_pointers[i].freed = true;
    }
  }
}

void my_free(void *ptr) {
  if (check_double_free(ptr)) {
    fprintf(stderr, "Double free detected at %p\n", ptr);
    exit(EXIT_FAILURE);
  }

  mark_as_freed(ptr);

  // Free logic
  for (size_t i = 0; i < meta_pool_size; i++) {
    if (meta_pool[i].ptr == ptr && !meta_pool[i].free) {
      if (meta_pool[i].canary != CANARY_VALUE) {
        fprintf(stderr, "Heap overflow detected!\n");
        exit(EXIT_FAILURE);
      }
      size_t block_size = meta_pool[i].size;
      meta_pool[i].free = 1;
      meta_pool[i].reused = false;
      meta_pool[i].ptr = NULL;
      meta_pool[i].size = 0;
      generate_report("free", block_size, ptr);
      return;
    }
  }
}

void check_canaries(void) {
  for (size_t i = 0; i < meta_pool_size; i++) {
    if (!meta_pool[i].free && meta_pool[i].size > 0) {
      if (meta_pool[i].canary != CANARY_VALUE) {
        fprintf(stderr, "Canary value corrupted at %p\n", meta_pool[i].ptr);
        exit(EXIT_FAILURE);
      }
    }
  }
}

void check_memory_leaks(void) {
  for (size_t i = 0; i < meta_pool_size; i++) {
    if (!meta_pool[i].free && meta_pool[i].size > 0) {
      fprintf(stderr, "Memory leak detected at %p, size: %zu\n",
              meta_pool[i].ptr, meta_pool[i].size);
      exit(EXIT_FAILURE);
    }
  }
}

// Call at exit
void clean(void) {
  check_memory_leaks();

  // Unmap pages of memory
  if (meta_pool)
    munmap(meta_pool, meta_pool_size * sizeof(block_descriptor_t));
  if (data_pool)
    munmap(data_pool, data_pool_size);
}
