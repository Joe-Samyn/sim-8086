//
// Created by Joe Samyn on 8/23/26.
//

#pragma once

#define ASSERT_TRUE(condition) { \
    if (condition != true) { \
        return { __func__, -1 }; \
    } \
}