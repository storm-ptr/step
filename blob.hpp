// Andrew Naplavkov

#ifndef STEP_BLOB_HPP
#define STEP_BLOB_HPP

#include <cstddef>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <step/detail/utility.hpp>
#include <string>
#include <string_view>

namespace step {

/// Binary Large OBject (BLOB) - contiguous byte storage
struct blob_view : std::basic_string_view<std::byte> {
    using std::basic_string_view<std::byte>::basic_string_view;
    blob_view(const std::byte*) = delete;
};

struct blob : std::vector<std::byte> {
    using std::vector<std::byte>::vector;
    operator blob_view() const noexcept { return {data(), size()}; }
};

using variant =
    std::variant<std::monostate, int64_t, double, std::string_view, blob_view>;

template <class T>
if_trivially_copyable<T, const T*> read(blob_view& src, size_t count)
{
    auto result = reinterpret_cast<const T*>(src.data());
    src.remove_prefix(count * sizeof(T));
    return result;
}

template <class T>
const T& read(blob_view& src)
{
    return *read<T>(src, 1);
}

template <class T>
if_trivially_copyable<T> write(const T* src, size_t count, blob& dest)
{
    auto first = reinterpret_cast<const std::byte*>(src);
    auto last = first + count * sizeof(T);
    dest.insert(dest.end(), first, last);
}

template <class T>
void write(const T& src, blob& dest)
{
    write(&src, 1, dest);
}

inline variant read_variant(blob_view& src)
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
        case variant_index<variant, blob_view>(): {
            auto count = read<blob_view::size_type>(src);
            return blob_view{read<blob_view::value_type>(src, count), count};
        }
    }
    throw std::runtime_error{"invalid variant type"};
}

inline void write_variant(const variant& src, blob& dest)
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

inline auto make_variants(blob_view src)
{
    std::vector<variant> result;
    while (!src.empty())
        result.push_back(read_variant(src));
    return result;
}

struct txt {
    const variant& var;

    friend std::ostream& operator<<(std::ostream& dest, const txt& src)
    {
        std::visit(overloaded{[&](std::monostate) { dest << ""; },
                              [&](auto v) { dest << v; },
                              [&](blob_view v) {
                                  dest << (std::to_string(v.size()) + " bytes");
                              }},
                   src.var);
        return dest;
    }
};

struct table {
    const std::vector<variant>& header;
    const std::vector<variant>& data;

    friend std::ostream& operator<<(std::ostream& dest, const table& src)
    {
        auto cols = src.header.size();
        auto line = std::vector<variant>(cols);
        auto widths = std::vector<size_t>(cols);
        row{widths, src.header.begin()}.fit(dest);
        for (auto it = src.data.begin(); it != src.data.end(); it += cols)
            row{widths, it}.fit(dest);
        dest << std::setfill(' ') << row{widths, src.header.begin()}
             << std::setfill('-') << row{widths, line.begin()}
             << std::setfill(' ');
        for (auto it = src.data.begin(); it != src.data.end(); it += cols)
            dest << row{widths, it};
        return dest;
    }

private:
    struct row {
        std::vector<size_t>& widths;
        std::vector<variant>::const_iterator first;

        friend std::ostream& operator<<(std::ostream& dest, const row& src)
        {
            auto it = src.first;
            for (size_t width : src.widths)
                dest << "|" << std::setw(1) << "" << std::setw(width)
                     << (std::holds_alternative<std::string_view>(*it)
                             ? std::left
                             : std::right)
                     << txt{*it++} << std::setw(1) << "";
            return dest << "|\n";
        }

        void fit(std::ostream& dest)
        {
            auto it = first;
            for (size_t& width : widths) {
                std::ostringstream os;
                os.copyfmt(dest);
                os << txt{*it++};
                width = std::max<>(width, os.str().size());
            }
        }
    };
};

}  // namespace step

#endif  // STEP_BLOB_HPP
