#include <iostream>
#include <filesystem>
#include <bustache/render/ostream.hpp>
#include <nlohmann/json.hpp>

// Typically, you don't need to explicitly delete the impl_model, but nlohmann::json
// is basically a amoeba that prentends to be any model, and that will cause hard
// errors in concept checking due to recursion.
template<>
struct bustache::impl_model<nlohmann::json>
{
    impl_model() = delete;
};

template<>
struct bustache::impl_compatible<nlohmann::json>
{
    static value_ptr get_value_ptr(nlohmann::json const& self)
    {
        nlohmann::json::value_t const kind(self);
        switch (kind)
        {
        case nlohmann::json::value_t::boolean:
            return value_ptr(self.get_ptr<nlohmann::json::boolean_t const*>());
        case nlohmann::json::value_t::number_integer:
            return value_ptr(self.get_ptr<nlohmann::json::number_integer_t const*>());
        case nlohmann::json::value_t::number_unsigned:
            return value_ptr(self.get_ptr<nlohmann::json::number_unsigned_t const*>());
        case nlohmann::json::value_t::number_float:
            return value_ptr(self.get_ptr<nlohmann::json::number_float_t const*>());
        case nlohmann::json::value_t::string:
            return value_ptr(self.get_ptr<nlohmann::json::string_t const*>());
        case nlohmann::json::value_t::array:
            return value_ptr(self.get_ptr<nlohmann::json::array_t const*>());
        case nlohmann::json::value_t::object:
            return value_ptr(self.get_ptr<nlohmann::json::object_t const*>());
        }
        return value_ptr();
    }
};

std::string read_file(char const* path)
{
    std::string ret;
    if (auto const fd = std::fopen(path, "rb"))
    {
        auto const bytes = std::filesystem::file_size(path);
        ret.resize(bytes);
        std::fread(ret.data(), 1, bytes, fd);
        std::fclose(fd);
    }
    return ret;
}

struct file_context
{
    struct partail
    {
        std::string text;
        bustache::format format;

        void init(std::string_view filename)
        {
            text = read_file(std::string(filename).c_str());
            format = bustache::format(text);
        }
    };
    mutable std::unordered_map<std::string, partail> cache;

    bustache::format const* operator()(std::string_view key) const
    {
        auto [pos, inserted] = cache.try_emplace(std::string(key));
        if (inserted)
            pos->second.init(std::string(key) + ".mustache");
        return pos->second.text.empty() ? nullptr : &pos->second.format;
    }
};

int main()
{
    try
    {
        auto const json = nlohmann::json::parse(read_file("in.json"));
        auto const file = read_file("in.mustache");
        bustache::format fmt(file);
        file_context ctx;
        std::cout << fmt(json).context(ctx);
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what();
    }
}