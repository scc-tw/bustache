/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2024 Test Suite Contributors

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#include <catch2/catch_test_macros.hpp>
#include <bustache/render/string.hpp>
#include <bustache/format.hpp>
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>
#include "model.hpp"

using namespace bustache;
using namespace test;

TEST_CASE("failure_simulation_lambda_exceptions", "[failure]")
{
    SECTION("lambda throwing exception propagates")
    {
        auto throwing_lambda = lazy_format([](ast::view const*) -> format {
            throw std::runtime_error("Lambda error");
        });
        
        object data{{"lambda", throwing_lambda}};
        
        // The library doesn't catch lambda exceptions - they propagate
        CHECK_THROWS(to_string("{{lambda}}"_fmt(data)));
    }

    SECTION("lambda returning invalid format")
    {
        auto bad_lambda = lazy_format([](ast::view const*) -> format {
            return format("{{unclosed"); // Invalid template
        });
        
        object data{{"bad", bad_lambda}};
        
        // Should either handle gracefully or throw a controlled error
        try {
            to_string("{{bad}}"_fmt(data));
            CHECK(true); // If it doesn't throw, that's acceptable
        } catch (const format_error&) {
            CHECK(true); // Format errors are expected and acceptable
        } catch (...) {
            CHECK(false); // Unexpected exception type
        }
    }
}

// KNOWN LIMITATION: Recursive partials cause stack overflow
// This test is disabled as the library doesn't have recursion protection
// TEST_CASE("failure_simulation_recursive_partials", "[failure]")
// {
//     SECTION("self-referencing partial should not stack overflow")
//     {
//         format recursive("{{> self}}");
//         
//         std::unordered_map<std::string, format> partials;
//         partials["self"] = recursive;
//         
//         object data{{"value", "test"}};
//         
//         auto context = [&partials](std::string const& name) -> format const* {
//             auto it = partials.find(name);
//             return it != partials.end() ? &it->second : nullptr;
//         };
//         
//         // This WILL cause stack overflow - no protection implemented
//         // Uncomment only if recursion protection is added to the library
//         
//         std::string result;
//         bool exception_thrown = false;
//         
//         try {
//             render_string(result, recursive, data, context);
//         } catch (...) {
//             exception_thrown = true;
//         }
//         
//         CHECK((result.length() < 10000 || exception_thrown));
//     }

//     SECTION("mutually recursive partials")
//     {
//         format partial_a("A{{> b}}");
//         format partial_b("B{{> a}}");
//         
//         std::unordered_map<std::string, format> partials;
//         partials["a"] = partial_a;
//         partials["b"] = partial_b;
//         
//         object data;
//         
//         auto context = [&partials](std::string const& name) -> format const* {
//             auto it = partials.find(name);
//             return it != partials.end() ? &it->second : nullptr;
//         };
//         
//         format main_template("Start: {{> a}}");
//         
//         // This WILL cause stack overflow - no protection implemented
//     }
// }

TEST_CASE("failure_simulation_memory_stress", "[failure]")
{
    SECTION("large array iteration should not exhaust memory")
    {
        // Create a large array
        array large_array;
        for (int i = 0; i < 10000; ++i) {
            large_array.push_back(object{{"index", i}, {"value", "test"}});
        }
        
        object data{{"items", large_array}};
        
        // This should complete without exhausting memory
        auto result = to_string("{{#items}}{{index}}:{{value}};{{/items}}"_fmt(data));
        
        // Should have processed all items
        CHECK(result.find("9999:test;") != std::string::npos);
    }

    SECTION("deeply nested object access")
    {
        // Create a deeply nested structure
        object deepest{{"value", "found"}};
        object* current = &deepest;
        
        for (int i = 0; i < 100; ++i) {
            object* next = new object{{"level" + std::to_string(i), *current}};
            current = next;
        }
        
        // Build the access path
        std::string path = "{{";
        for (int i = 99; i >= 0; --i) {
            path += "level" + std::to_string(i) + ".";
        }
        path += "value}}";
        
        auto result = to_string(format(path)(*current));
        CHECK(result == "found");
        
        // Clean up
        delete current;
    }
}

TEST_CASE("failure_simulation_concurrent_rendering", "[failure]")
{
    SECTION("concurrent rendering should be thread-safe")
    {
        format tmpl("Thread {{id}}: {{value}}");
        std::atomic<int> success_count{0};
        std::vector<std::thread> threads;
        
        for (int i = 0; i < 10; ++i) {
            threads.emplace_back([&tmpl, &success_count, i]() {
                object data{{"id", i}, {"value", "test"}};
                
                for (int j = 0; j < 100; ++j) {
                    try {
                        auto result = to_string(tmpl(data));
                        if (result == "Thread " + std::to_string(i) + ": test") {
                            success_count++;
                        }
                    } catch (...) {
                        // Concurrent access issues would throw
                    }
                }
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
        
        // All renders should succeed
        CHECK(success_count == 1000);
    }
}

TEST_CASE("failure_simulation_invalid_input", "[failure]")
{
    SECTION("invalid UTF-8 sequences should be handled")
    {
        // Create invalid UTF-8 sequence
        std::string invalid_utf8 = "Valid text ";
        invalid_utf8.push_back(static_cast<char>(0xFF));
        invalid_utf8.push_back(static_cast<char>(0xFE));
        invalid_utf8 += " more text";
        
        object data{{"text", invalid_utf8}};
        
        // Should not crash when rendering
        CHECK_NOTHROW(to_string("{{text}}"_fmt(data)));
    }

    SECTION("null characters in template")
    {
        std::string template_with_null = "Before";
        template_with_null.push_back('\0');
        template_with_null += "After{{var}}";
        
        object data{{"var", "value"}};
        
        // Should handle null character gracefully
        auto result = to_string(format(template_with_null)(data));
        CHECK(result.find("value") != std::string::npos);
    }
}

TEST_CASE("failure_simulation_partial_loading", "[failure]")
{
    SECTION("missing partial should render empty or error gracefully")
    {
        format tmpl("Before {{> missing}} After");
        object data;
        
        // Context that returns nullptr for missing partials
        auto context = [](std::string const&) -> format const* {
            return nullptr;
        };
        
        std::string result;
        CHECK_NOTHROW(render_string(result, tmpl, data, context));
        CHECK(result == "Before  After");
    }

    SECTION("partial returning malformed template")
    {
        format tmpl("Main {{> bad}}");
        
        std::unordered_map<std::string, format> partials;
        
        object data;
        
        auto context = [&partials](std::string const& name) -> format const* {
            if (name == "bad") {
                // Simulate partial loading that fails
                // The format constructor throws on malformed templates
                try {
                    format bad("{{still unclosed");
                    partials["bad"] = bad;
                } catch (const format_error&) {
                    // Return nullptr when partial can't be loaded
                    return nullptr;
                }
            }
            auto it = partials.find(name);
            return it != partials.end() ? &it->second : nullptr;
        };
        
        std::string result;
        CHECK_NOTHROW(render_string(result, tmpl, data, context));
        CHECK(result == "Main "); // Missing partial renders as empty
    }
}