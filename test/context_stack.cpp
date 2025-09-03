/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2024 Test Suite Contributors

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

TEST_CASE("context_push_pop_behavior", "[context]")
{
    SECTION("sections push and pop context")
    {
        // Entering a section pushes a new context onto the stack
        // Exiting a section pops the context
        object data{
            {"name", "outer"},
            {"section", object{
                {"name", "inner"}
            }}
        };
        
        format tmpl("{{name}} {{#section}}{{name}}{{/section}} {{name}}");
        
        std::string result;
        render_string(result, tmpl, data);
        CHECK(result == "outer inner outer");
    }

    SECTION("nested sections create multiple context levels")
    {
        object data{
            {"value", "level0"},
            {"a", object{
                {"value", "level1"},
                {"b", object{
                    {"value", "level2"},
                    {"c", object{
                        {"value", "level3"}
                    }}
                }}
            }}
        };
        
        format tmpl("{{value}} {{#a}}{{value}} {{#b}}{{value}} {{#c}}{{value}}{{/c}}{{/b}}{{/a}}");
        
        std::string result;
        render_string(result, tmpl, data);
        CHECK(result == "level0 level1 level2 level3");
    }

    SECTION("array iteration creates context for each element")
    {
        array items{
            object{{"id", 1}, {"name", "first"}},
            object{{"id", 2}, {"name", "second"}},
            object{{"id", 3}, {"name", "third"}}
        };
        
        object data{
            {"name", "root"},
            {"items", items}
        };
        
        format tmpl("{{name}} {{#items}}{{name}}:{{id}} {{/items}}{{name}}");
        
        std::string result;
        render_string(result, tmpl, data);
        CHECK(result == "root first:1 second:2 third:3 root");
    }
}

TEST_CASE("variable_shadowing_in_nested_contexts", "[context]")
{
    SECTION("nested context variables shadow outer ones")
    {
        object data{
            {"value", "outer"},
            {"nested", object{
                {"value", "inner"}
            }}
        };
        
        format tmpl("{{value}} {{#nested}}{{value}}{{/nested}} {{value}}");
        
        std::string result;
        render_string(result, tmpl, data);
        CHECK(result == "outer inner outer");
    }

    SECTION("deeply nested shadowing")
    {
        object data{
            {"x", "A"},
            {"level1", object{
                {"x", "B"},
                {"level2", object{
                    {"x", "C"},
                    {"level3", object{
                        {"x", "D"}
                    }}
                }}
            }}
        };
        
        format tmpl("{{x}}{{#level1}}{{x}}{{#level2}}{{x}}{{#level3}}{{x}}{{/level3}}{{x}}{{/level2}}{{x}}{{/level1}}{{x}}");
        
        std::string result;
        render_string(result, tmpl, data);
        CHECK(result == "ABCDCBA");
    }

    SECTION("variable lookup falls through to parent context")
    {
        object data{
            {"outer_only", "visible"},
            {"shared", "outer_shared"},
            {"nested", object{
                {"inner_only", "also_visible"},
                {"shared", "inner_shared"}
            }}
        };
        
        format tmpl("{{#nested}}{{inner_only}} {{outer_only}} {{shared}}{{/nested}}");
        
        std::string result;
        render_string(result, tmpl, data);
        CHECK(result == "also_visible visible inner_shared");
    }
}

TEST_CASE("parent_context_access", "[context]")
{
    SECTION("parent context access is not supported in standard Mustache")
    {
        // Note: Standard Mustache doesn't support '../' for parent context access
        // This test documents the expected behavior (treating '../' as a regular variable name)
        object data{
            {"value", "parent"},
            {"child", object{
                {"value", "current"}
            }}
        };
        
        // In standard Mustache, '../value' is treated as a literal variable name
        format tmpl("{{#child}}{{value}} {{../value}}{{/child}}");
        
        std::string result;
        render_string(result, tmpl, data);
        // '../value' won't resolve to parent context's 'value'
        CHECK(result == "current ");
    }

    SECTION("dotted names for parent context access also unsupported")
    {
        object data{
            {"name", "root"},
            {"parent", object{
                {"name", "middle"},
                {"child", object{
                    {"name", "leaf"}
                }}
            }}
        };
        
        // Dotted names starting with '..' are not special
        format tmpl("{{#parent}}{{#child}}{{name}} {{..name}}{{/child}}{{/parent}}");
        
        std::string result;
        render_string(result, tmpl, data);
        CHECK(result == "leaf ");
    }
}

TEST_CASE("root_context_access", "[context]")
{
    SECTION("accessing root context values from nested sections")
    {
        // Variables not found in current context fall through to parent contexts
        object data{
            {"root_value", "from_root"},
            {"level1", object{
                {"l1_value", "from_l1"},
                {"level2", object{
                    {"l2_value", "from_l2"}
                }}
            }}
        };
        
        format tmpl("{{#level1}}{{#level2}}{{l2_value}} {{l1_value}} {{root_value}}{{/level2}}{{/level1}}");
        
        std::string result;
        render_string(result, tmpl, data);
        CHECK(result == "from_l2 from_l1 from_root");
    }

    SECTION("root values accessible unless shadowed")
    {
        object data{
            {"value", "root"},
            {"a", object{
                {"b", object{
                    {"value", "shadowed"},
                    {"c", object{}}
                }}
            }}
        };
        
        format tmpl("{{value}} {{#a}}{{value}} {{#b}}{{value}} {{#c}}{{value}}{{/c}}{{/b}}{{/a}}");
        
        std::string result;
        render_string(result, tmpl, data);
        CHECK(result == "root root shadowed shadowed");
    }

    SECTION("dotted names can access nested values from root")
    {
        object data{
            {"a", object{
                {"b", object{
                    {"c", "deep_value"}
                }}
            }},
            {"section", object{}}
        };
        
        format tmpl("{{#section}}{{a.b.c}}{{/section}}");
        
        std::string result;
        render_string(result, tmpl, data);
        CHECK(result == "deep_value");
    }
}

TEST_CASE("context_preservation_across_partials", "[context]")
{
    SECTION("partials inherit parent context")
    {
        object data{
            {"name", "World"},
            {"section", object{
                {"name", "Section"}
            }}
        };
        
        auto partial_context = context{
            {"greeting", "Hello {{name}}!"_fmt}
        };
        
        format tmpl("{{>greeting}} {{#section}}{{>greeting}}{{/section}}");
        
        std::string result;
        render_string(result, tmpl, data, partial_context);
        CHECK(result == "Hello World! Hello Section!");
    }

    SECTION("nested partials preserve context stack")
    {
        object data{
            {"value", "A"},
            {"section", object{
                {"value", "B"}
            }}
        };
        
        auto partial_context = context{
            {"outer", "{{value}}{{>inner}}"_fmt},
            {"inner", "[{{value}}]"_fmt}
        };
        
        format tmpl("{{>outer}} {{#section}}{{>outer}}{{/section}}");
        
        std::string result;
        render_string(result, tmpl, data, partial_context);
        CHECK(result == "A[A] B[B]");
    }

    SECTION("context restored after partial completes")
    {
        // This is the original test, kept for regression
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

    SECTION("partials in arrays maintain proper context")
    {
        array items{
            object{{"name", "item1"}},
            object{{"name", "item2"}},
            object{{"name", "item3"}}
        };
        
        object data{
            {"title", "List"},
            {"items", items}
        };
        
        auto partial_context = context{
            {"item_template", "{{name}} "_fmt}
        };
        
        format tmpl("{{title}}: {{#items}}{{>item_template}}{{/items}}");
        
        std::string result;
        render_string(result, tmpl, data, partial_context);
        CHECK(result == "List: item1 item2 item3 ");
    }
}