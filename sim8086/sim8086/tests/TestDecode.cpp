//
// Created by Joe Samyn on 8/21/26.
//

#include "Assert.h"
#include "TestFixture.h"

TEST(TestPass, {
    ASSERT_TRUE(true);
})

TEST(TestFail, {
    ASSERT_TRUE(false);
})

TESTS(DecodeTests) = {
    TestPass,
    TestFail
};
