#include "my_secmalloc.private.h"
#include <criterion/criterion.h>
#include <criterion/logging.h>
#include <stdio.h>

#define LITTLE_SIZE 1048576   // 1 MB
#define LARGE_SIZE 1073741824 // 1 GB

Test(secmalloc, malloc) {
  printf("[TEST] malloc: Testing malloc with various sizes and multiple "
         "allocations\n");

  // Test with zero-size allocation
  void *ptr_zero = my_malloc(0);
  cr_assert_not_null(ptr_zero, "malloc with zero size returned NULL");
  my_free(ptr_zero);

  // Test with large allocation size
  size_t large_size = LARGE_SIZE; // Allocate 1GB
  void *ptr_large = my_malloc(large_size);
  cr_assert_not_null(ptr_large, "malloc with large size returned NULL");
  my_free(ptr_large);

  // Test with multiple allocations and deallocations
  void *ptr_multiple[100];
  for (int i = 0; i < 100; i++) {
    ptr_multiple[i] = my_malloc(10);
    cr_assert_not_null(ptr_multiple[i], "malloc iteration %d returned NULL", i);
  }
  for (int i = 0; i < 100; i++) {
    my_free(ptr_multiple[i]);
  }
}

Test(secmalloc, calloc) {
  printf("[TEST] calloc: Testing calloc with various sizes and multiple "
         "allocations\n");

  // Test with zero-size allocation
  void *ptr_zero = my_calloc(0, 10);
  cr_assert_not_null(ptr_zero, "calloc with zero nmemb returned NULL");
  my_free(ptr_zero);

  // Test with little allocation size
  size_t little_size = LITTLE_SIZE; // Allocate 1GB
  void *ptr_little = my_calloc(1, little_size);
  cr_assert_not_null(ptr_little, "calloc with large size returned NULL");
  for (size_t i = 0; i < little_size; i++) {
    cr_assert(((char *)ptr_little)[i] == 0, "calloc did not zero out memory");
  }
  my_free(ptr_little);

  // Test with multiple allocations and deallocations
  printf(
      "[TEST] calloc: Testing multiple calloc allocations and deallocations\n");
  void *ptr_multiple[100];
  for (int i = 0; i < 100; i++) {
    ptr_multiple[i] = my_calloc(1, 10);
    cr_assert_not_null(ptr_multiple[i], "calloc iteration %d returned NULL", i);
  }
  for (int i = 0; i < 100; i++) {
    my_free(ptr_multiple[i]);
  }
}

Test(secmalloc, realloc) {
  printf("[TEST] realloc: Testing realloc with various sizes and multiple "
         "allocations\n");

  // Test with NULL pointer and large allocation size
  size_t large_size = LARGE_SIZE; // Allocate 1GB
  void *ptr_large = my_realloc(NULL, large_size);
  cr_assert_not_null(ptr_large,
                     "realloc with NULL pointer and large size returned NULL");
  my_free(ptr_large);

  // Test with multiple allocations and deallocations
  void *ptr_multiple[100];
  for (int i = 0; i < 100; i++) {
    ptr_multiple[i] = my_realloc(NULL, 10);
    cr_assert_not_null(ptr_multiple[i], "realloc iteration %d returned NULL",
                       i);
  }
  for (int i = 0; i < 100; i++) {
    my_free(ptr_multiple[i]);
  }
}

Test(secmalloc, free) {
  printf("[TEST] free: Testing freeing of allocated memory\n");

  void *ptr = my_malloc(100);
  cr_assert_not_null(ptr, "malloc returned NULL");
  my_free(ptr);
}

Test(secmalloc, double_free) {
  printf("[TEST] double_free: Testing detection of double free\n");
  cr_log_warn(
      "Double Free test should fail if detection is implemented correctly");
  void *ptr = my_malloc(100);
  my_free(ptr);
  my_free(ptr); // This should trigger the double-free detection
}

Test(secmalloc, canary_check) {
  printf("[TEST] canary_check: Testing detection of buffer overflow using "
         "canary value\n");
  cr_log_warn(
      "Buffer overflow test should fail if canary is implemented correctly");
  void *ptr = my_malloc(100);
  cr_assert_not_null(ptr, "malloc returned NULL");

  // Intentionally corrupt the canary value
  char *buffer = (char *)ptr;
  buffer[100] = 'A'; // Write beyond allocated memory

  check_canaries();
  my_free(ptr); // This should detect the corrupted canary and handle it (e.g.
                // log or terminate)
}

// Comment out this test if you don't have `check_memory_leaks` implemented
Test(secmalloc, memory_leak) {
  printf("[TEST] memory_leak: Testing detection of memory leaks\n");
  void *ptr1 = my_malloc(100);
  void *ptr2 = my_malloc(200);
  void *ptr3 = my_malloc(300);
  (void)ptr3; // Cast to avoid "unused variable" warning

  my_free(ptr1);
  my_free(ptr2);
  // Intentionally do not free ptr3 to simulate memory leak
}
