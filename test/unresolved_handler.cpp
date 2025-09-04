/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2017-2023 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <bustache/render/string.hpp>
#include <span>
#include "model.hpp"

using namespace bustache;
using namespace test;

value_ptr throw_on_unresolved(std::string const& key)
{
    throw std::runtime_error("unresolved key: " + key);
}

value_ptr banana_on_unresolved(std::string const& key)
{
    static constexpr std::string_view banana("banana");
    return &banana;
}

TEST_CASE("unresolved")
{
    format const fmt("before-{{unresolved}}-after");
    object const empty;
    std::string out;

    SECTION("throw")
    {
        CHECK_THROWS_WITH
        (
            render_string(out, fmt, empty, no_context, no_escape, throw_on_unresolved),
            "unresolved key: unresolved"
        );

        CHECK(out == "before-");
    }

    SECTION("default value")
    {
        render_string(out, fmt, empty, no_context, no_escape, banana_on_unresolved);

        CHECK(out == "before-banana-after");
    }
}

TEST_CASE("nested")
{
    format const fmt("{{a.b}}");
    constexpr auto void_sink = [](std::span<const char>) {};

    CHECK_THROWS_WITH
    (
        render(void_sink, fmt, object{}, no_context, no_escape, throw_on_unresolved),
        "unresolved key: a"
    );

    CHECK_THROWS_WITH
    (
        render(void_sink, fmt, object{{"a", object{}}}, no_context, no_escape, throw_on_unresolved),
        "unresolved key: b"
    );
}