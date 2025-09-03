/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2024 Test Suite Contributors

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#include <catch2/catch_test_macros.hpp>
#include <bustache/render/string.hpp>
#include <limits>
#include <string>
#include "model.hpp"

using namespace bustache;
using namespace test;

TEST_CASE("boundary_tests_empty_values", "[boundary]")
{
    SECTION("empty string variable should render as empty")
    {
        object data{{"empty", ""}};
        CHECK(to_string("Before{{empty}}After"_fmt(data)) == "BeforeAfter");
    }

    SECTION("single character template should render correctly")
    {
        object data{{"x", "X"}};
        CHECK(to_string("a"_fmt(data)) == "a");
        CHECK(to_string("{{x}}"_fmt(data)) == "X");
    }

    SECTION("empty array in section should not render content")
    {
        object data{{"items", array{}}};
        CHECK(to_string("{{#items}}Should not appear{{/items}}"_fmt(data)) == "");
    }

    SECTION("single element array should render once")
    {
        object data{{"items", array{"solo"}}};
        CHECK(to_string("{{#items}}{{.}}{{/items}}"_fmt(data)) == "solo");
    }
}

TEST_CASE("boundary_tests_numeric_limits", "[boundary]")
{
    SECTION("integer at maximum value")
    {
        object data{{"max_int", std::numeric_limits<int>::max()}};
        CHECK(to_string("{{max_int}}"_fmt(data)) == std::to_string(std::numeric_limits<int>::max()));
    }

    SECTION("integer at minimum value")
    {
        object data{{"min_int", std::numeric_limits<int>::min()}};
        CHECK(to_string("{{min_int}}"_fmt(data)) == std::to_string(std::numeric_limits<int>::min()));
    }

    SECTION("zero value should render as 0")
    {
        object data{{"zero", 0}};
        CHECK(to_string("Value: {{zero}}"_fmt(data)) == "Value: 0");
    }

    SECTION("floating point at small precision")
    {
        object data{{"tiny", 0.0000001}};
        auto result = to_string("{{tiny}}"_fmt(data));
        CHECK(result.find("e") != std::string::npos); // Should use scientific notation
    }
}

TEST_CASE("boundary_tests_variable_names", "[boundary]")
{
    SECTION("single character variable name")
    {
        object data{{"a", "A"}};
        CHECK(to_string("{{a}}"_fmt(data)) == "A");
    }

    SECTION("very long variable name")
    {
        std::string long_name(100, 'x');
        object data{{long_name, "value"}};
        std::string template_str = "{{" + long_name + "}}";
        CHECK(to_string(format(template_str)(data)) == "value");
    }

    SECTION("variable name with underscores and numbers")
    {
        object data{{"var_123_test", "success"}};
        CHECK(to_string("{{var_123_test}}"_fmt(data)) == "success");
    }
}

TEST_CASE("boundary_tests_nesting_depth", "[boundary]")
{
    SECTION("deeply nested sections")
    {
        // Create a deeply nested object structure
        object inner{{"value", "deep"}};
        object level3{{"c", inner}};
        object level2{{"b", level3}};
        object level1{{"a", level2}};
        
        CHECK(to_string("{{#a}}{{#b}}{{#c}}{{value}}{{/c}}{{/b}}{{/a}}"_fmt(level1)) == "deep");
    }

    SECTION("maximum nesting with arrays")
    {
        array arr3{1, 2};
        array arr2{arr3};
        array arr1{arr2};
        object data{{"nested", arr1}};
        
        CHECK(to_string("{{#nested}}{{#.}}{{#.}}{{.}}{{/.}}{{/.}}{{/nested}}"_fmt(data)) == "12");
    }
}

TEST_CASE("boundary_tests_whitespace_handling", "[boundary]")
{
    SECTION("template with only whitespace")
    {
        object data{{"x", "X"}};
        CHECK(to_string("   "_fmt(data)) == "   ");
        CHECK(to_string("\t\n\r"_fmt(data)) == "\t\n\r");
    }

    SECTION("variables surrounded by various whitespace")
    {
        object data{{"var", "VALUE"}};
        CHECK(to_string("  {{var}}  "_fmt(data)) == "  VALUE  ");
        CHECK(to_string("\t{{var}}\n"_fmt(data)) == "\tVALUE\n");
    }

    SECTION("empty sections with whitespace")
    {
        object data{{"show", false}};
        CHECK(to_string("  {{#show}}  text  {{/show}}  "_fmt(data)) == "    ");
    }
}