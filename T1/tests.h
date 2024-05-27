#include <stdbool.h>

#include <limits.h>



#define ASSERT_FAIL() { fprintf(stderr, "Assert at line %d failed\n", __LINE__); return false; }

#define ASSERT_TRUE(x) if (!(x)) { ASSERT_FAIL() }

#define ASSERT_EQ(a, b) ASSERT_TRUE((a) == (b))

#define ASSERT_NEQ(a, b) ASSERT_TRUE((a) != (b))

#define ASSERT_CC_OK(x) ASSERT_EQ(x, CC_OK)



typedef bool (*test_t)();

extern test_t TESTS[];

