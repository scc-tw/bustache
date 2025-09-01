/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2024

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#include <catch2/catch_test_macros.hpp>
#include <bustache/render/string.hpp>
#include "model.hpp"

using namespace bustache;
using namespace test;

TEST_CASE("context stack is restored after partial rendering")
{
    CHECK(
        to_string(
            "{{#section}}{{>partial}}{{/section}}{{>partial}}"_fmt(
                object{
                    {"section", object{{{"value", "section"}}}},
                    {"value", "root"}
                }
            ).context(context{{"partial", "{{value}}"_fmt}})
        )
        == "sectionroot"
    );
}
