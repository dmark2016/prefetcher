#include <stdio.h>
#include <stdlib.h>
#include "prefetcher.cc"

typedef bool (*test_ptr)();

int prefetch_index;
Addr * expected_prefetches;

/**
 * Mock required functions
 */
int in_cache(Addr addr) {
    return 0;
}

void issue_prefetch(Addr addr) {
    if (expected_prefetches[prefetch_index++] != addr) {
        printf(
            "\nUnexpected prefetch for addr %lu. Expected %lu.\n\n",
            addr,
            expected_prefetches[prefetch_index - 1]
        );
        exit(EXIT_FAILURE);
    } else {
        printf("Prefetching %lu\n", addr);
    }
}

/**
 * Actual tests
 */

bool test1() {

    // Set up prefetches
    Addr prefetches[] = {
        12, 30, 31, 35, 40
    };
    expected_prefetches = &prefetches[0];

    // Addresses
    //                 1   4   5   1   4  5   1    4
    Addr addrs[] = {10, 11, 15, 20, 21, 25, 30, 31, 35};
    int n_addr = sizeof(addrs)/sizeof(Addr);

    for (int i = 0; i < n_addr; i++) {
        printf("\n\n-- Access to %lu --\n", addrs[i]);
        prefetch_access((AccessStat) {
            10, addrs[i], i, 1
        });
    }

    return prefetch_index == sizeof(prefetches)/sizeof(Addr);
}

bool test2() {

    // Set up prefetches
    Addr prefetches[] = {3};
    expected_prefetches = &prefetches[0];

    // Addresses
    Addr addrs[] = {1, 2, 4, 7, 11, 16, 22, 29, 37, 46, 56};
    int n_addr = sizeof(addrs)/sizeof(Addr);

    for (int i = 0; i < n_addr; i++) {
        prefetch_access((AccessStat) {
            10, addrs[i], i, 1
        });
    }

    return prefetch_index == sizeof(prefetches)/sizeof(Addr);
}


bool test3() {

    // Set up prefetches
    Addr prefetches[] = {1, 1};
    expected_prefetches = &prefetches[0];

    // Addresses
    Addr addrs[] = {1, 1, 3, 1, 1, 3};
    int n_addr = sizeof(addrs)/sizeof(Addr);

    for (int i = 0; i < n_addr; i++) {
        prefetch_access((AccessStat) {
            10, addrs[i], i, 1
        });
    }

    return prefetch_index == 0;
}


/**
 * Test runner
 */
test_ptr tests[] = {test1, test2};
int main(int argc, char ** argv) {

    for (int i = 0; i < sizeof(tests)/sizeof(test_ptr); i++) {
        // Initialize prefetcher
        prefetch_index = 0;
        prefetch_init();

        // Run test
        bool success = tests[i]();

        // Error printing
        if (success) {
            printf("SUCCESS: Test %i\n", i);
        } else {
            printf("FAILURE: Test %i\n", i);
            exit(EXIT_FAILURE);
        }
    }

}
