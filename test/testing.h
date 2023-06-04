#ifndef LIBWFC_TESTS_TESTING_H_
#define LIBWFC_TESTS_TESTING_H_

#define PRINT_TEST_FAIL() \
    fprintf(stderr, "Test failed: %s:%d %s\n", __FILE__, __LINE__, __func__)

#endif
