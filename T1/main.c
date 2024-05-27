#include "lib.h"

#include "tests.h"



#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)

#define WINDOWS

#include <crtdbg.h>

#endif



#include <stdio.h>

#include <stdlib.h>

#include <float.h>



static size_t test_count;



void calc_test_count() {

    test_count = 0;

    while (TESTS[test_count] != NULL) {

        test_count++;

    }

}



bool run_test_case(int index) {

    if (index < 0 || index >= test_count) {

        fprintf(stderr, "Invalid test index\n");

        return false;

    }



    test_t test = TESTS[index];



    bool result;

#ifdef WINDOWS

    __try {

#endif

    result = test();

#ifdef WINDOWS

    }

    __except (true) {

        fprintf(stderr, "Exception caught\n");

        result = false;

    }

#endif



    if (result) {

        fprintf(stderr, "Test %d passed\n", index);

    } else {

        fprintf(stderr, "Test %d failed\n", index);

    }



    return result;

}



int main(int argc, char** argv) {



#ifdef WINDOWS

    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);

    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);



    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);

    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);



    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);

    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);

#endif



    calc_test_count();



    if (argc == 1) {

        bool all_passed = true;



        for (int i = 0; i < test_count; i++) {

            all_passed = all_passed & run_test_case(i);

        }



        return all_passed ? EXIT_SUCCESS : EXIT_FAILURE;

    }



    if (argc != 2) {

        fprintf(stderr, "Expected exactly one argument\n");

        return EXIT_FAILURE;

    }



    int requested_test;

    if (sscanf(argv[1], "%d", &requested_test) != 1) {

        fprintf(stderr, "First argument must be the test index to execute\n");

        return EXIT_FAILURE;

    }



    return run_test_case(requested_test) ? EXIT_SUCCESS : EXIT_FAILURE;

}

