#include "tests.h"

#include "lib.h"



#include <stdio.h>

#include <stdlib.h>

#include <float.h>



bool example_test() {

    CC_Array* a;



    ASSERT_CC_OK(cc_array_new(&a))

    ASSERT_CC_OK(cc_array_add(a, (void*) 1))

    ASSERT_EQ(1, cc_array_size(a))



    void* get_result;

    ASSERT_CC_OK(cc_array_get_at(a, 0, &get_result))

    ASSERT_EQ(1, (int) get_result)



    cc_array_destroy(a);

    return true;

}







test_t TESTS[] = {

    &example_test,

    NULL

};