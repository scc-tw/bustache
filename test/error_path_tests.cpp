/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2024 Test Suite Contributors

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#include <catch2/catch_test_macros.hpp>
#include <bustache/render/string.hpp>
#include <bustache/format.hpp>
#include "model.hpp"

using namespace bustache;
using namespace test;

TEST_CASE("error_path_malformed_templates", "[error-path]")
{
    SECTION("missing closing tag should throw")
    {
        REQUIRE_THROWS_AS(format("{{unclosed"), format_error);
    }

    SECTION("unmatched section tags behavior")
    {
        // The library is lenient - unclosed sections don't throw
        CHECK_NOTHROW(format("{{#section}}content"));
        
        // Closing tag without opening should throw
        REQUIRE_THROWS_AS(format("{{/section}}"), format_error);
    }

    SECTION("mismatched section names should throw")
    {
        REQUIRE_THROWS_AS(format("{{#foo}}{{/bar}}"), format_error);
    }

    SECTION("invalid delimiter syntax should throw")
    {
        REQUIRE_THROWS_AS(format("{{=<% %>}}"), format_error);
    }

    SECTION("empty delimiter should throw")
    {
        REQUIRE_THROWS_AS(format("{{= =}}"), format_error);
    }
}

TEST_CASE("error_path_null_handling", "[error-path]")
{
    SECTION("null values should render as empty")
    {
        object data{{"null_val", nullptr}};
        CHECK(to_string("Value: [{{null_val}}]"_fmt(data)) == "Value: []");
    }

    SECTION("accessing properties of null should be safe")
    {
        object data{{"obj", nullptr}};
        CHECK(to_string("{{obj.property}}"_fmt(data)) == "");
    }

    SECTION("null in sections should be falsy")
    {
        object data{{"null_section", nullptr}};
        CHECK(to_string("{{#null_section}}Should not appear{{/null_section}}"_fmt(data)) == "");
        CHECK(to_string("{{^null_section}}Should appear{{/null_section}}"_fmt(data)) == "Should appear");
    }
}

TEST_CASE("error_path_invalid_variable_names", "[error-path]")
{
    SECTION("variables with special characters")
    {
        // These should either work or fail gracefully
        object data{{"var-with-dash", "value1"}, {"var.with.dot", "value2"}};
        
        // Dashes might work depending on implementation
        auto result1 = to_string("{{var-with-dash}}"_fmt(data));
        CHECK((result1 == "value1" || result1 == ""));
        
        // Dots have special meaning for nested access
        CHECK(to_string("{{var.with.dot}}"_fmt(data)) == "");
    }

    SECTION("empty variable name should throw")
    {
        object data{{"", "empty_key"}};
        // Empty variable names are invalid and throw
        CHECK_THROWS(to_string("{{}}"_fmt(data)));
    }

    SECTION("variable names with spaces are surprisingly valid")
    {
        object data{{"has space", "value"}};
        // The library actually allows spaces in variable names
        CHECK(to_string("{{has space}}"_fmt(data)) == "value");
    }
}

TEST_CASE("error_path_extreme_inputs", "[error-path]")
{
    SECTION("extremely long template should handle gracefully")
    {
        std::string huge_template(10000, 'a');
        huge_template += "{{var}}";
        huge_template += std::string(10000, 'b');
        
        object data{{"var", "X"}};
        auto result = to_string(format(huge_template)(data));
        CHECK(result.length() == 20001); // 10000 + 1 + 10000
        CHECK(result[10000] == 'X');
    }

    SECTION("deeply nested broken chains")
    {
        object data{{"a", object{{"b", nullptr}}}};
        CHECK(to_string("{{a.b.c.d.e.f.g}}"_fmt(data)) == "");
    }

    SECTION("circular-like structure should not infinite loop")
    {
        // Note: True circular references might not be possible depending on the data model
        // This tests a deep structure that could expose stack issues
        object inner{{"ref", "value"}};
        object mid{{"inner", inner}};
        object outer{{"mid", mid}};
        object data{{"outer", outer}};
        
        CHECK(to_string("{{#outer}}{{#mid}}{{#inner}}{{ref}}{{/inner}}{{/mid}}{{/outer}}"_fmt(data)) == "value");
    }
}

TEST_CASE("error_path_type_mismatches", "[error-path]")
{
    SECTION("using non-array in section iteration")
    {
        object data{{"not_array", "string_value"}};
        // String is truthy, so section should render once with the string as context
        CHECK(to_string("{{#not_array}}Content{{/not_array}}"_fmt(data)) == "Content");
    }

    SECTION("using array as simple variable")
    {
        object data{{"arr", array{1, 2, 3}}};
        // Arrays render as empty when used as simple variables
        auto result = to_string("{{arr}}"_fmt(data));
        CHECK(result == ""); // Arrays don't have a string representation
    }

    SECTION("boolean values in different contexts")
    {
        object data{{"bool_true", true}, {"bool_false", false}};
        
        // As variables
        CHECK(to_string("{{bool_true}}"_fmt(data)) == "true");
        CHECK(to_string("{{bool_false}}"_fmt(data)) == "false");
        
        // In sections
        CHECK(to_string("{{#bool_true}}YES{{/bool_true}}"_fmt(data)) == "YES");
        CHECK(to_string("{{#bool_false}}NO{{/bool_false}}"_fmt(data)) == "");
    }
}