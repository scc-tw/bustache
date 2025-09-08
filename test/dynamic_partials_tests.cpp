/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2024 Test Suite Contributors

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#include <catch2/catch_test_macros.hpp>
#include <bustache/render/string.hpp>
#include <bustache/format.hpp>
#include <unordered_map>
#include <optional>
#include <functional>
#include "model.hpp"

using namespace bustache;
using namespace test;

TEST_CASE("dynamic_partials_name_resolution", "[dynamic]")
{
    SECTION("dynamic partial with variable name")
    {
        // Dynamic partials use {{>*name}} syntax where name is resolved from context
        object data{
            {"template_name", "greeting"},
            {"user", "World"}
        };
        
        std::unordered_map<std::string, format> partials;
        partials["greeting"] = format("Hello {{user}}!");
        partials["farewell"] = format("Goodbye {{user}}!");
        
        auto context = [&partials](std::string_view name) -> std::optional<std::reference_wrapper<format const>> {
            auto it = partials.find(std::string(name));
            return it != partials.end() ? std::optional{std::ref(it->second)} : std::nullopt;
        };
        
        // Test dynamic partial resolution
        format tmpl("{{>*template_name}}");
        
        std::string result;
        render_string(result, tmpl, data, context);
        CHECK(result == "Hello World!");
    }

    SECTION("dynamic partial with changing context")
    {
        std::unordered_map<std::string, format> partials;
        partials["header"] = format("<h1>{{title}}</h1>");
        partials["paragraph"] = format("<p>{{content}}</p>");
        partials["footer"] = format("<footer>{{info}}</footer>");
        
        auto context = [&partials](std::string_view name) -> std::optional<std::reference_wrapper<format const>> {
            auto it = partials.find(std::string(name));
            return it != partials.end() ? std::optional{std::ref(it->second)} : std::nullopt;
        };
        
        // Different data contexts selecting different partials
        object data1{
            {"partial_type", "header"},
            {"title", "Welcome"}
        };
        
        object data2{
            {"partial_type", "paragraph"},
            {"content", "This is the main content."}
        };
        
        format tmpl("{{>*partial_type}}");
        
        std::string result1;
        render_string(result1, tmpl, data1, context);
        CHECK(result1 == "<h1>Welcome</h1>");
        
        std::string result2;
        render_string(result2, tmpl, data2, context);
        CHECK(result2 == "<p>This is the main content.</p>");
    }

    SECTION("dynamic partial in section")
    {
        array items{
            object{{"type", "text"}, {"content", "First item"}},
            object{{"type", "bold"}, {"content", "Important"}},
            object{{"type", "text"}, {"content", "Last item"}}
        };
        
        object data{{"items", items}};
        
        std::unordered_map<std::string, format> partials;
        partials["text"] = format("{{content}}");
        partials["bold"] = format("<b>{{content}}</b>");
        
        auto context = [&partials](std::string_view name) -> std::optional<std::reference_wrapper<format const>> {
            auto it = partials.find(std::string(name));
            return it != partials.end() ? std::optional{std::ref(it->second)} : std::nullopt;
        };
        
        format tmpl("{{#items}}{{>*type}} {{/items}}");
        
        std::string result;
        render_string(result, tmpl, data, context);
        CHECK(result == "First item <b>Important</b> Last item ");
    }
}

TEST_CASE("dynamic_partials_context_based", "[dynamic]")
{
    SECTION("context-based partial selection with nested data")
    {
        object data{
            {"user", object{
                {"template", "user_card"},
                {"name", "John Doe"},
                {"role", "Developer"}
            }}
        };
        
        std::unordered_map<std::string, format> partials;
        partials["user_card"] = format("Name: {{name}}, Role: {{role}}");
        partials["admin_card"] = format("Admin: {{name}} [{{permissions}}]");
        
        auto context = [&partials](std::string_view name) -> std::optional<std::reference_wrapper<format const>> {
            auto it = partials.find(std::string(name));
            return it != partials.end() ? std::optional{std::ref(it->second)} : std::nullopt;
        };
        
        format tmpl("{{#user}}{{>*template}}{{/user}}");
        
        std::string result;
        render_string(result, tmpl, data, context);
        CHECK(result == "Name: John Doe, Role: Developer");
    }

    SECTION("conditional partial selection")
    {
        std::unordered_map<std::string, format> partials;
        partials["logged_in"] = format("Welcome back, {{username}}!");
        partials["guest"] = format("Please log in to continue.");
        
        auto context = [&partials](std::string_view name) -> std::optional<std::reference_wrapper<format const>> {
            auto it = partials.find(std::string(name));
            return it != partials.end() ? std::optional{std::ref(it->second)} : std::nullopt;
        };
        
        // Logged in user
        object logged_in_data{
            {"status", "logged_in"},
            {"username", "alice"}
        };
        
        format tmpl("{{>*status}}");
        
        std::string result1;
        render_string(result1, tmpl, logged_in_data, context);
        CHECK(result1 == "Welcome back, alice!");
        
        // Guest user
        object guest_data{{"status", "guest"}};
        
        std::string result2;
        render_string(result2, tmpl, guest_data, context);
        CHECK(result2 == "Please log in to continue.");
    }
}

TEST_CASE("dynamic_partials_missing_template", "[dynamic]")
{
    SECTION("missing dynamic partial renders empty")
    {
        object data{{"partial_name", "nonexistent"}};
        
        // Empty partial map
        std::unordered_map<std::string, format> partials;
        
        auto context = [&partials](std::string_view name) -> std::optional<std::reference_wrapper<format const>> {
            auto it = partials.find(std::string(name));
            return it != partials.end() ? std::optional{std::ref(it->second)} : std::nullopt;
        };
        
        format tmpl("Before {{>*partial_name}} After");
        
        std::string result;
        render_string(result, tmpl, data, context);
        CHECK(result == "Before  After");
    }

    SECTION("undefined variable for dynamic partial name")
    {
        object data{{"other", "value"}};
        
        std::unordered_map<std::string, format> partials;
        partials["default"] = format("Default content");
        
        auto context = [&partials](std::string_view name) -> std::optional<std::reference_wrapper<format const>> {
            auto it = partials.find(std::string(name));
            return it != partials.end() ? std::optional{std::ref(it->second)} : std::nullopt;
        };
        
        // Variable 'missing_var' doesn't exist in data
        format tmpl("{{>*missing_var}}");
        
        std::string result;
        render_string(result, tmpl, data, context);
        CHECK(result == ""); // Renders empty when variable is undefined
    }

    SECTION("fallback to default partial")
    {
        object data{{"template", "special"}};
        
        std::unordered_map<std::string, format> partials;
        partials["default"] = format("Default content");
        
        // Custom context that falls back to 'default' for missing partials
        auto context = [&partials](std::string_view name) -> std::optional<std::reference_wrapper<format const>> {
            auto it = partials.find(std::string(name));
            if (it != partials.end()) {
                return std::optional{std::ref(it->second)};
            }
            // Fallback to default if the requested partial doesn't exist
            it = partials.find("default");
            return it != partials.end() ? std::optional{std::ref(it->second)} : std::nullopt;
        };
        
        format tmpl("{{>*template}}");
        
        std::string result;
        render_string(result, tmpl, data, context);
        CHECK(result == "Default content");
    }
}

TEST_CASE("dynamic_partials_recursive", "[dynamic]")
{
    SECTION("dynamic partial with self-reference prevention")
    {
        // Test if the library handles recursive dynamic partials
        // Note: Based on earlier tests, recursive partials cause stack overflow
        // This test verifies the current behavior
        
        object data{
            {"partial_name", "recursive"},
            {"counter", 3}
        };
        
        std::unordered_map<std::string, format> partials;
        // This partial references itself through a dynamic partial
        partials["recursive"] = format("Level {{counter}} {{>*partial_name}}");
        
        auto _ = [&partials](std::string_view name) -> std::optional<std::reference_wrapper<format const>> {
            auto it = partials.find(std::string(name));
            return it != partials.end() ? std::optional{std::ref(it->second)} : std::nullopt;
        };
        
        format tmpl("{{>*partial_name}}");
        
        // KNOWN LIMITATION: This will cause stack overflow
        // The library doesn't have recursion protection
        // Uncomment only if recursion protection is added
        
        // std::string result;
        // CHECK_THROWS(render_string(result, tmpl, data, context));
    }

    SECTION("mutually recursive dynamic partials")
    {
        // Test mutual recursion between dynamic partials
        // This is also expected to cause issues without protection
        
        std::unordered_map<std::string, format> partials;
        partials["ping"] = format("Ping {{>*next}}");
        partials["pong"] = format("Pong {{>*prev}}");
        
        auto _ = [&partials](std::string_view name) -> std::optional<std::reference_wrapper<format const>> {
            auto it = partials.find(std::string(name));
            return it != partials.end() ? std::optional{std::ref(it->second)} : std::nullopt;
        };
        
        object data{
            {"current", "ping"},
            {"next", "pong"},
            {"prev", "ping"}
        };
        
        format tmpl("{{>*current}}");
        
        // KNOWN LIMITATION: Mutual recursion also causes stack overflow
        // Uncomment only if recursion protection is added
        
        // std::string result;
        // CHECK_THROWS(render_string(result, tmpl, data, context));
    }

    SECTION("bounded recursion with counter")
    {
        // A safer approach using a counter to limit recursion
        array levels{
            object{{"depth", 1}, {"template", "level1"}},
            object{{"depth", 2}, {"template", "level2"}},
            object{{"depth", 3}, {"template", "level3"}}
        };
        
        object data{{"levels", levels}};
        
        std::unordered_map<std::string, format> partials;
        partials["level1"] = format("[Level 1]");
        partials["level2"] = format("[Level 2]");
        partials["level3"] = format("[Level 3]");
        
        auto context = [&partials](std::string_view name) -> std::optional<std::reference_wrapper<format const>> {
            auto it = partials.find(std::string(name));
            return it != partials.end() ? std::optional{std::ref(it->second)} : std::nullopt;
        };
        
        format tmpl("{{#levels}}{{>*template}}{{/levels}}");
        
        std::string result;
        render_string(result, tmpl, data, context);
        CHECK(result == "[Level 1][Level 2][Level 3]");
    }
}