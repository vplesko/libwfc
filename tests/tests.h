#ifndef LIBWFC_TESTS_TESTS_H_
#define LIBWFC_TESTS_TESTS_H_

#define ARR_LEN(a) (sizeof(a) / sizeof((a)[0]))

#define PRINT_TEST_FAIL() \
    fprintf(stderr, "Test failed: %s:%d %s\n", __FILE__, __LINE__, __func__)

#endif
