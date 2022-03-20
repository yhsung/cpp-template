// Demo of Address Sanitizer.
// By Ari Saif
//-----------------------------------------------------------------------------
// How to build and run?
//-----------------------------------------------------------------------------
// Using bazel:
/*
bazel run --config=asan //src/main:main_address_sanitize -- [choice]
where choice is one of the choices below. See the main function below.

Example:

bazel run --config=asan //src/main:main_address_sanitize -- 1
*/
//-----------------------------------------------------------------------------
// Using GCC:
/*
* Run with this command:

g++ \
-O0 \
-g \
-fno-omit-frame-pointer \
-fsanitize=address \
src/main/main_address_sanitize.cc

* And then:
./a.out <choice>

* where choice is one of of the values below See the main function below.
*/
//-----------------------------------------------------------------------------
#include <iostream>
#include <string>
#include <vector>

//-----------------------------------------------------------------------------
volatile char *global_ptr;

__attribute__((noinline)) void FunctionThatEscapesLocalObject() {
  char local[100];
  global_ptr = &local[0];
}

char global_array[10];

//-----------------------------------------------------------------------------
int main(int argc, char **argv) {
  if (argc != 2) {
    std::cout << "Usage: main_address_sanitize <choice>" << std::endl;
    return -1;
  }

  int choice = std::stoi(argv[1]);
  std::cout << "choice: " << choice << std::endl;

  switch (choice) {
    case 0: {
      //  SEGV on unknown address
      char *ptr = nullptr;
      *ptr = 5;
      break;
    }

    case 1: {
      // SEGV on unknown address
      std::vector<char> v;
      std::cout << "v[100]: " << v[100] << std::endl;
      break;
    }

    case 2: {
      // BUS on unknown address
      char *ptr;
      std::cout << "ptr: " << (int *)ptr << std::endl;
      *ptr = 5;
      break;
    }

    case 3: {
      // Usage after delete.
      char *p = new char;
      delete p;
      std::cout << "*p: " << *p << std::endl;
      break;
    }

    case 4: {
      // Double-free.
      char *p = new char;
      delete p;
      delete p;
      break;
    }

    case 5: {
      // Stack buffer overflow.
      char array[16];
      array[20] = 1;  // BOOM!
      break;
    }

    case 6: {
      // Heap buffer overflow.
      char *array = new char[16];
      array[16] = 1;  // BOOM!
      break;
    }

    case 7: {
      // Heap buffer overflow (example 2).
      char *ptr = new char;
      *ptr = 'a';
      std::cout << *(ptr + 1) << std::endl;
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
      // stack-use-after-return
      // Set this env variable before running:
      // export ASAN_OPTIONS=detect_stack_use_after_return=1
      FunctionThatEscapesLocalObject();
      return global_ptr[0];
    }

    case 10: {
      // global-buffer-overflow
      // char global_array[10]; // global variable.
      global_array[11] = 1;  // Boom!
      break;
    }

    case 11: {
      // global-buffer-overflow
      static char array[10];
      array[11] = 1;  // Boom!
      break;
    }

    case 12: {
      // Leak detection (Doesn't work on Mac?)
      // Set this env variable before running:
      // export ASAN_OPTIONS=detect_leaks=1
      // Or with bazel:
      /*
        bazel run --config=asan //src/main:main_address_sanitize \
        --run_under='export ASAN_OPTIONS=detect_leaks=1 &&' -- 12
      */
      char *p = new char;
      *p = 10;
      std::cout << "*p: " << *p << std::endl;
      break;
    }

    default:
      std::cout << "Error: Invalid choice value: " << choice << std::endl;
  }

  return 0;
}