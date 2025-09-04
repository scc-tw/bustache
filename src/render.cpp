/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2016-2023 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/

#include <bustache/render.hpp>
#include <cassert>
#include <span>

namespace bustache::detail
{
    struct object_ptr
    {
        void const* data;
        void(*_get)(void const* self, std::string const& key, value_handler visit);

        static object_ptr from(value_ptr val)
        {
            auto vptr = val.get_vptr();
            if (vptr->kind == model::object)
                return {val.get_data(), static_cast<value_vtable const*>(vptr)->get};
            return {nullptr, object_trait::get_default};
        }

        static object_ptr from_nested(value_ptr val)
        {
            auto vptr = val.get_vptr();
            if (vptr->kind < model::lazy_value)
                return {val.get_data(), static_cast<value_vtable const*>(vptr)->get};
            return {nullptr, object_trait::get_default};
        }

        constexpr explicit operator bool() const { return !!data; }

        void get(std::string const& key, value_handler visit) const
        {
            _get(data, key, visit);
        }
    };

    struct content_scope
    {
        content_scope const* const parent;
        object_ptr data;
    };

    template<class Visit>
    void lookup(content_scope const* scope, std::string const& key, Visit const& visit)
    {
        bool found = false;
        do
        {
            scope->data.get(key, [&](value_ptr val)
            {
                if (val)
                {
                    visit(val);
                    found = true;
                }
            });
            if (found)
                return;
            scope = scope->parent;
        } while (scope);
        visit(nullptr);
    }

    struct subkey
    {
        char const* i;
        char const* const e;

        constexpr explicit operator bool() const { return i != e; }
    };

    struct nested_resolver
    {
        using iter = char const*;
        subkey sub;
        std::string& key_cache;
        value_handler handle;
        bool done;

        void next(object_ptr obj)
        {
            auto const k0 = ++sub.i;
            while (sub)
            {
                if (*sub.i == '.')
                {
                    key_cache.assign(k0, sub.i);
                    return obj.get(key_cache, [this](value_ptr val)
                    {
                        if (auto const obj = object_ptr::from(val))
                            next(obj);
                    });
                }
                else
                    ++sub.i;
            }
            key_cache.assign(k0, sub.i);
            obj.get(key_cache, [this](value_ptr val)
            {
                if (val)
                {
                    handle(val);
                    done = true;
                }
            });
        }
    };

    struct override_context
    {
        ast::override_map const* map;
        ast::context const* ctx;
    };

    struct override_find_result
    {
        ast::content_list const* found;
        ast::context const* ctx;
    };

    struct content_visitor
    {
        using result_type = void;

        ast::context const* ctx;
        content_scope const* scope;
        value_ptr cursor;
        std::vector<override_context> chain;
        mutable std::string key_cache;

        output_handler raw_os;
        output_handler escape_os;
        context_handler context;
        unresolved_handler variable_unresolved;
        std::string indent;
        bool needs_indent;

        content_visitor
        (
            ast::context const& ctx, content_scope const& scope, value_ptr cursor,
            output_handler raw_os, output_handler escape_os, context_handler context,
            unresolved_handler f
        )
            : ctx(&ctx), scope(&scope), cursor(cursor)
            , raw_os(raw_os), escape_os(escape_os), context(context)
            , variable_unresolved(f)
            , needs_indent()
        {}

        content_visitor(content_visitor const&) = delete;

        template<class Visit>
        void resolve(std::string_view key, Visit visit) const
        {
            auto ki = key.data();
            auto const ke = ki + key.size();
            if (ki == ke)
                return visit(nullptr, subkey{});
            if (*ki == '.')
            {
                subkey sub{ki, ke};
                if (++ki == ke)
                    sub.i = ki;
                return visit(cursor, sub);
            }
            // Unqualified.
            auto const k0 = ki;
            while (ki != ke && *ki != '.') ++ki;
            key_cache.assign(k0, ki);
            lookup(scope, key_cache, [&visit, sub = subkey{ki, ke}](value_ptr val)
            {
                visit(val, sub);
            });
        }

        void visit_within(ast::context const& new_ctx, ast::content_list const& contents)
        {
            auto const old_ctx = ctx;
            ctx = &new_ctx;
            for (auto const content : contents)
                new_ctx.visit(*this, content);
            ctx = old_ctx;
        }

        void visit_within(ast::document const& doc)
        {
            visit_within(doc.ctx, doc.contents);
        }

        override_find_result find_override(std::string const& key) const;

        void print_value(output_handler os, value_ptr val, char const* sepc, bool interpolation);

        void handle_variable(ast::type tag, value_ptr val, char const* sepc);

        void expand(ast::content_list const& contents)
        {
            for (auto const content : contents)
                ctx->visit(*this, content);
        }

        void expand_on_object(ast::content_list const& contents, value_ptr val)
        {
            auto const old_cursor = cursor;
            auto vptr = val.get_vptr();
            object_ptr data{val.get_data(), static_cast<value_vtable const*>(vptr)->get};
            content_scope curr{scope, data};
            cursor = val;
            scope = &curr;
            expand(contents);
            scope = curr.parent;
            cursor = old_cursor;
        }

        void expand_on_value(ast::content_list const& contents, value_ptr val)
        {
            if (val.get_vptr()->kind == model::object)
                expand_on_object(contents, val);
            else
            {
                cursor = val;
                expand(contents);
            }
        }

        bool expand_section(ast::type tag, ast::content_list const& contents, value_ptr val);

        void handle_section(ast::type tag, ast::block const& block, value_ptr val);

        void resolve_and_handle(std::string_view key, unresolved_handler unresolved, value_handler handle);

        std::string const& deref_dyn_name(std::string const& key)
        {
            if (key.starts_with('*'))
            {
                std::string_view const s(key.data() + 1, key.size() - 1);
                resolve_and_handle(s, nullptr, [this](value_ptr val)
                {
                    key_cache.clear();
                    auto const os = [this](std::span<const char> data)
                    {
                        key_cache.append(data.data(), data.size());
                    };
                    print_value(os, val, nullptr, false);
                });
                return key_cache;
            }
            return key;
        }

        void operator()(ast::type, ast::text const* text);

        void operator()(ast::type tag, ast::variable const* variable)
        {
            char const* sepc = nullptr;
            std::string_view key = variable->key;
            if (auto const split = variable->split)
            {
                sepc = key.data() + (split + 1);
                key = std::string_view(key.data(), split);
            }
            resolve_and_handle(key, variable_unresolved, [=, this](value_ptr val)
            {
                handle_variable(tag, val, sepc);
            });
        }

        void operator()(ast::type tag, ast::block const* block)
        {
            if (tag == ast::type::inheritance)
            {
                auto const result = find_override(block->key);
                if (result.found)
                    visit_within(*result.ctx, *result.found);
                else
                {
                    for (auto const content : block->contents)
                        ctx->visit(*this, content);
                }
            }
            else
            {
                resolve_and_handle(block->key, nullptr, [&](value_ptr val)
                {
                    handle_section(tag, *block, val);
                });
            }
        }

        void operator()(ast::type, ast::partial const* partial);

        void operator()(ast::type, void const*) const {} // never called
    };

    override_find_result content_visitor::find_override(std::string const& key) const
    {
        for (auto const pm : chain)
        {
            auto const it = pm.map->find(key);
            if (it != pm.map->end())
                return {&it->second, pm.ctx};
        }
        return {};
    }

    void content_visitor::print_value(output_handler os, value_ptr val, char const* sepc, bool interpolation)
    {
        auto vptr = val.get_vptr();
        auto data = val.get_data();
        switch (vptr->kind)
        {
        case model::lazy_value:
            static_cast<lazy_value_vtable const*>(vptr)->call(data, nullptr, [=, this](value_ptr val)
            {
                print_value(os, val, sepc, interpolation);
            });
            break;
        case model::lazy_format:
            if (interpolation)
            {
                auto const fmt = static_cast<lazy_format_vtable const*>(vptr)->call(data, nullptr);
                visit_within(fmt.doc());
            }
            break;
        default:
            static_cast<value_vtable const*>(vptr)->print(data, os, sepc);
            break;
        }
    }

    void content_visitor::handle_variable(ast::type tag, value_ptr val, char const* sepc)
    {
        if (needs_indent)
        {
            raw_os(std::span<const char>(indent.data(), indent.size()));
            needs_indent = false;
        }
        print_value(tag == ast::type::var_raw ? raw_os : escape_os, val, sepc, true);
    }

    bool content_visitor::expand_section(ast::type tag, ast::content_list const& contents, value_ptr val)
    {
        bool inverted = false;
        auto vptr = val.get_vptr();
        auto kind = vptr->kind;
        if (kind < model::lazy_value)
        {
            switch (tag)
            {
            case ast::type::inversion:
                inverted = true;
                [[fallthrough]];
            case ast::type::filter:
                kind = model::atom;
                break;
            case ast::type::loop:
                kind = model::list;
                break;
            default:
                break;
            }
        }
        else if (tag == ast::type::inversion) // Inverted lazy.
            return false;
        switch (kind)
        {
        case model::null:
            return inverted;
        case model::atom:
            return static_cast<value_vtable const*>(vptr)->test(val.get_data()) ^ inverted;
        case model::object:
            expand_on_object(contents, val);
            return false;
        case model::list:
        {
            auto const vt = static_cast<value_vtable const*>(vptr);
            auto const old_cursor = cursor;
            if (!vt->iterate)
                expand_on_value(contents, val);
            else
            {
                vt->iterate(val.get_data(), [&](value_ptr val)
                {
                    expand_on_value(contents, val);
                });
            }
            cursor = old_cursor;
            return false;
        }
        case model::lazy_value:
        {
            bool ret = false;
            ast::view const view{*ctx, contents};
            static_cast<lazy_value_vtable const*>(vptr)->call(val.get_data(), &view, [&](value_ptr val)
            {
                ret = expand_section(tag, contents, val);
            });
            return ret;
        }
        case model::lazy_format:
        {
            if (tag == ast::type::filter)
                return true;
            ast::view const view{*ctx, contents};
            auto const fmt = static_cast<lazy_format_vtable const*>(vptr)->call(val.get_data(), &view);
            visit_within(fmt.doc());
            return false;
        }
        }
        std::abort(); // Unreachable.
    }

    void content_visitor::handle_section(ast::type tag, ast::block const& block, value_ptr val)
    {
        if (expand_section(tag, block.contents, val))
        {
            for (auto const content : block.contents)
                ctx->visit(*this, content);
        }
    }

    void content_visitor::resolve_and_handle(std::string_view key, unresolved_handler unresolved, value_handler handle)
    {
        resolve(key, [=, this](value_ptr val, subkey sub)
        {
            if (sub)
            {
                if (auto const obj = object_ptr::from_nested(val))
                {
                    nested_resolver nested{sub, key_cache, handle, false};
                    if (nested.next(obj), nested.done)
                        return;
                }
            }
            else if (val)
                return handle(val);
            handle(unresolved ? unresolved(key_cache) : nullptr);
        });
    }

    void content_visitor::operator()(ast::type, ast::text const* text)
    {
        auto i = text->data();
        auto const n = text->size();
        assert(n && "empty text shouldn't be in ast");
        if (indent.empty())
        {
            raw_os(std::span<const char>(i, n));
            return;
        }
        auto const ib = indent.data();
        auto const in = indent.size();
        if (needs_indent)
            raw_os(std::span<const char>(ib, in));
        auto i0 = i;
        auto const e = i + (n - 1); // Don't flush indent on last newline.
        while (i != e)
        {
            if (*i++ == '\n')
            {
                raw_os(std::span<const char>(i0, static_cast<std::size_t>(i - i0)));
                raw_os(std::span<const char>(ib, in));
                i0 = i;
            }
        }
        needs_indent = *i++ == '\n';
        raw_os(std::span<const char>(i0, static_cast<std::size_t>(i - i0)));
    }

    void content_visitor::operator()(ast::type, ast::partial const* partial)
    {
        if (auto const opt_format = context(deref_dyn_name(partial->key)); opt_format)
        {
            auto const& doc = opt_format->get().doc();
            if (doc.contents.empty())
                return;
            auto const old_size = indent.size();
            auto const old_chain = chain.size();
            auto const old_scope = scope;
            auto const old_cursor = cursor;
            indent += partial->indent;
            needs_indent |= !partial->indent.empty();
            if (!partial->overriders.empty())
                chain.push_back({&partial->overriders, ctx});
            visit_within(doc);
            scope = old_scope;
            cursor = old_cursor;
            chain.resize(old_chain);
            indent.resize(old_size);
        }
    }

    void render(output_handler raw_os, output_handler escape_os, format const& fmt, value_ptr data, context_handler context, unresolved_handler f)
    {
        content_scope scope{nullptr, object_ptr::from(data)};
        auto const& doc = fmt.doc();
        content_visitor visitor{doc.ctx, scope, data, raw_os, escape_os, context, f};
        for (auto const content : doc.contents)
            doc.ctx.visit(visitor, content);
    }
}

namespace bustache
{
    void impl_print<std::string_view>::print(std::string_view self, output_handler os, char const* spec)
    {
        if (spec)
            detail::print_fmt(self, os, spec);
        else
            os(std::span<const char>(self.data(), self.size()));
    }

    void impl_print<bool>::print(bool self, output_handler os, char const* spec)
    {
        if (spec)
            detail::print_fmt(self, os, spec);
        else
            self ? os(std::span<const char>("true", 4)) : os(std::span<const char>("false", 5));
    }
}
