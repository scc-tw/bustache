/*//////////////////////////////////////////////////////////////////////////////
    Copyright (c) 2016-2021 Jamboree

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////*/
#ifndef BUSTACHE_RENDER_OSTREAM_HPP_INCLUDED
#define BUSTACHE_RENDER_OSTREAM_HPP_INCLUDED

#include <iostream>
#include <span>
#include <bustache/render.hpp>

namespace bustache::detail
{
    template<class CharT, class Traits>
    struct ostream_sink
    {
        std::basic_ostream<CharT, Traits>& out;

        void operator()(std::span<const char> data) const
        {
            out.write(data.data(), data.size());
        }
    };
}

namespace bustache
{
    template<class CharT, class Traits, class Escape = no_escape_t>
    inline void render_ostream
    (
        std::basic_ostream<CharT, Traits>& out, format const& fmt,
        value_ref data, context_handler context = no_context_t{},
        Escape escape = {}, unresolved_handler f = nullptr
    )
    {
        render(detail::ostream_sink<CharT, Traits>{out}, fmt, data, context, escape, f);
    }
    
    template<class CharT, class Traits, class... Opts>
    inline std::basic_ostream<CharT, Traits>&
    operator<<(std::basic_ostream<CharT, Traits>& out, manipulator<Opts...> const& manip)
    {
        render_ostream(out, manip.fmt, manip.data, get_context(manip), get_escape(manip));
        return out;
    }
}

#endif