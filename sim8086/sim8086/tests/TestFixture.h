//
// Created by Joe Samyn on 8/21/26.
//

#pragma once

#include <vector>
struct TestResult {
    const char* name;
    int result;
};

typedef TestResult (*TestFunc)();
typedef std::vector<TestFunc> TestArray;

#define TEST(name, test) static struct TestResult name() { test return {#name, 0}; }
#define TESTS(name) TestArray name
#define TEST_COUNT(tests) sizeof(tests) / sizeof(TestFunc)
