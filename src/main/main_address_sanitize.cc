// Demo of Address Sanitizer.
// By Ari Saif
// Run with this command:
// bazel run --config=asan //src/main:main_address_sanitize -- --choice=[choice]
// where choice is one of the values from 0 to 6. See the main function below.

#include <iostream>
#include <string>
#include <vector>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"

ABSL_FLAG(uint32_t, choice, 0, "choice");
//-----------------------------------------------------------------------------
volatile char *global_ptr;

__attribute__((noinline)) void FunctionThatEscapesLocalObject() {
  char local[100];
  global_ptr = &local[0];
}

char global_array[10];

//-----------------------------------------------------------------------------
int main(int argc, char **argv) {
  absl::ParseCommandLine(argc, argv);

  int choice = absl::GetFlag(FLAGS_choice);

  switch (choice) {
    case 0: {
      // Usage after delete.
      int *p = new int;
      delete p;
      std::cout << "*p: " << *p << std::endl;
      break;
    }

    case 1: {
      // SEGV on unknown address
      std::vector<int> v;
      std::cout << "v[100]: " << v[100] << std::endl;
      break;
    }

    case 2: {
      // Stack buffer overflow.
      char array[16];
      array[20] = 1;  // BOOM!
      break;
    }

    case 3: {
      // Heap buffer overflow.
      char *array = new char[16];

      array[16] = 1;  // BOOM!
      break;
    }

    case 4: {
      // Leak detection (Doesn't work on Mac?)
      // Set this env variable before running:
      // export ASAN_OPTIONS=detect_leaks=1
      // Or with bazel:
      // bazel run --config=asan //src/main:main_address_sanitize \
      // --run_under='export ASAN_OPTIONS=detect_leaks=1 &&' -- --choice=4
      int *p = new int;
      *p = 10;
      std::cout << "*p: " << *p << std::endl;
      break;
    }

    case 5: {
      // stack-use-after-return
      // Set this env variable before running:
      // export ASAN_OPTIONS=detect_stack_use_after_return=1
      FunctionThatEscapesLocalObject();
      return global_ptr[0];
    }

    case 6: {
      // global-buffer-overflow
      global_array[11] = 1;  // Boom!
      break;
    }

    case 7: {
      // global-buffer-overflow
      static char array[10];
      array[11] = 1;  // Boom!
      break;
    }

    case 8: {
      // stack-use-after-scope
      volatile char *ptr = nullptr;
      {
        char x = 0;
        ptr = &x;
      }
      *ptr = 5;
      break;
    }

    case 9: {
      // stack-use-after-scope
      volatile char *ptr = nullptr;

      *ptr = 5;
      break;
    }

    default:
      std::cout << "Error: Invalid choice value: " << choice << std::endl;
  }

  return 0;
}