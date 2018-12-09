// Andrew Naplavkov

#ifndef STEP_BLOB_HPP
#define STEP_BLOB_HPP

#include <cstddef>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <step/utility.hpp>
#include <string>
#include <string_view>

/// Binary Large OBject (BLOB) - contiguous byte storage
namespace step::blob {

struct view : std::basic_string_view<std::byte> {
    using std::basic_string_view<std::byte>::basic_string_view;
    view(const std::byte*) = delete;
};

struct container : std::vector<std::byte> {
    using std::vector<std::byte>::vector;
    operator view() const noexcept { return {data(), size()}; }
};

using variant =
    std::variant<std::monostate, int64_t, double, std::string_view, view>;

template <class T>
if_trivially_copyable<T, const T*> read(view& src, size_t count)
{
    auto result = reinterpret_cast<const T*>(src.data());
    src.remove_prefix(count * sizeof(T));
    return result;
}

template <class T>
T read(view& src)
{
    return *read<T>(src, 1);
}

template <>
inline variant read<variant>(view& src)
{
    switch (read<uint8_t>(src)) {
        case variant_index<variant, std::monostate>():
            return {};
        case variant_index<variant, int64_t>():
            return read<int64_t>(src);
        case variant_index<variant, double>():
            return read<double>(src);
        case variant_index<variant, std::string_view>(): {
            auto count = read<std::string_view::size_type>(src);
            return std::string_view{
                read<std::string_view::value_type>(src, count), count};
        }
        case variant_index<variant, view>(): {
            auto count = read<view::size_type>(src);
            return view{read<view::value_type>(src, count), count};
        }
    }
    throw std::runtime_error{"invalid variant type"};
}

template <class T>
if_trivially_copyable<T> write(const T* src, size_t count, container& dest)
{
    auto first = reinterpret_cast<const std::byte*>(src);
    auto last = first + count * sizeof(T);
    dest.insert(dest.end(), first, last);
}

template <class T>
void write(const T& src, container& dest)
{
    write(&src, 1, dest);
}

template <>
inline void write<variant>(const variant& src, container& dest)
{
    write<uint8_t>(src.index(), dest);
    std::visit(overloaded{[&](std::monostate) {},
                          [&](int64_t v) { write(v, dest); },
                          [&](double v) { write(v, dest); },
                          [&](auto v) {
                              write(v.size(), dest);
                              write(v.data(), v.size(), dest);
                          }},
               src);
}

template <class T>
view& operator>>(view& src, T& dest)
{
    dest = read<T>(src);
    return src;
}

template <class T>
container& operator<<(container& dest, const T& src)
{
    write(src, dest);
    return dest;
}

struct table {
    const std::vector<variant>& header;
    const container& data;

    friend std::ostream& operator<<(std::ostream& dest, const table& src)
    {
        auto cols = src.header.size();
        auto line = std::vector<variant>(cols);
        auto widths = std::vector<size_t>(cols);
        auto vars = std::vector<variant>{};
        for (auto data = static_cast<view>(src.data); !data.empty();)
            vars.push_back(read<variant>(data));
        row{widths, src.header.begin()}.fit(dest);
        for (auto it = vars.begin(); it != vars.end(); it += cols)
            row{widths, it}.fit(dest);
        dest << std::setfill(' ') << row{widths, src.header.begin()}
             << std::setfill('-') << row{widths, line.begin()}
             << std::setfill(' ');
        for (auto it = vars.begin(); it != vars.end(); it += cols)
            dest << row{widths, it};
        return dest;
    }

private:
    struct text {
        const variant& var;
    };

    struct row {
        std::vector<size_t>& widths;
        std::vector<variant>::const_iterator first;

        void fit(std::ostream& dest)
        {
            auto it = first;
            for (size_t& width : widths) {
                std::ostringstream os;
                os.copyfmt(dest);
                os << text{*it++};
                width = std::max<>(width, os.str().size());
            }
        }
    };

    friend std::ostream& operator<<(std::ostream& dest, const text& src)
    {
        std::visit(overloaded{[&](std::monostate) { dest << ""; },
                              [&](auto v) { dest << v; },
                              [&](view v) {
                                  dest << (std::to_string(v.size()) + " bytes");
                              }},
                   src.var);
        return dest;
    }

    friend std::ostream& operator<<(std::ostream& dest, const row& src)
    {
        auto it = src.first;
        for (size_t width : src.widths)
            dest << "|" << std::setw(1) << "" << std::setw(width)
                 << (std::holds_alternative<std::string_view>(*it) ? std::left
                                                                   : std::right)
                 << text{*it++} << std::setw(1) << "";
        return dest << "|\n";
    }
};

}  // namespace step::blob

#endif  // STEP_BLOB_HPP
