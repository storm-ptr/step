// Andrew Naplavkov

#ifndef STEP_ROWSET_HPP
#define STEP_ROWSET_HPP

#include <cctype>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <step/utility.hpp>
#include <string>

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

using variant_t =
    std::variant<std::monostate, int64_t, double, std::string_view, blob_view>;

using row_t = std::vector<variant_t>;

struct variant_istream {
    blob_view data;
};

struct variant_ostream {
    blob data;
};

struct rowset {
    std::vector<std::string> columns;
    blob data;
};

}  // namespace step

namespace std {

inline std::ostream& operator<<(std::ostream& dest, const step::variant_t& src)
{
    std::visit(
        step::overloaded{[&](std::monostate) { dest << ""; },
                         [&](auto v) { dest << v; },
                         [&](step::blob_view v) {
                             // call stream once
                             dest << (std::to_string(v.size()) + " bytes");
                         }},
        src);
    return dest;
}

}  // namespace std

namespace step {

template <class T>
if_arithmetic_t<T, const T*> read(blob_view& src, size_t count)
{
    auto result = reinterpret_cast<const T*>(src.data());
    src.remove_prefix(count * sizeof(T));
    return result;
}

template <class T>
if_arithmetic_t<T, T> read(blob_view& src)
{
    return *read<T>(src, 1);
}

template <class T, class Size = typename T::size_type>
T read(blob_view& src)
{
    Size count = *read<Size>(src, 1);
    return {read<typename T::value_type>(src, count), count};
}

template <class T>
if_arithmetic_t<T> write(const T* src, size_t count, blob& dest)
{
    auto first = reinterpret_cast<const std::byte*>(src);
    auto last = first + count * sizeof(T);
    dest.insert(dest.end(), first, last);
}

template <class T>
if_arithmetic_t<T> write(const T& src, blob& dest)
{
    write(&src, 1, dest);
}

template <class T, class Size = typename T::size_type>
void write(const T& src, blob& dest)
{
    Size count = src.size();
    write(&count, 1, dest);
    write(src.data(), count, dest);
}

template <class T>
blob_view& operator>>(blob_view& src, T& dest)
{
    dest = read<T>(src);
    return src;
}

template <class T>
blob& operator<<(blob& dest, const T& src)
{
    write(src, dest);
    return dest;
}

inline variant_t read(variant_istream& src)
{
    switch (step::read<uint8_t>(src.data)) {
        case variant_index<variant_t, std::monostate>():
            return {};
        case variant_index<variant_t, int64_t>():
            return step::read<int64_t>(src.data);
        case variant_index<variant_t, double>():
            return step::read<double>(src.data);
        case variant_index<variant_t, std::string_view>(): {
            auto result = step::read<std::string_view>(src.data);
            step::read<char>(src.data);  // zero terminated
            return result;
        }
        case variant_index<variant_t, blob_view>():
            return step::read<blob_view>(src.data);
    }
    throw std::logic_error{"invalid variant"};
}

inline void write(const variant_t& src, variant_ostream& dest)
{
    dest.data << static_cast<uint8_t>(src.index());
    std::visit(overloaded{[&](std::monostate) {},
                          [&](auto v) { dest.data << v; },
                          [&](std::string_view v) { dest.data << v << '\0'; }},
               src);
}

inline variant_istream& operator>>(variant_istream& src, variant_t& dest)
{
    dest = read(src);
    return src;
}

inline variant_ostream& operator<<(variant_ostream& dest, const variant_t& src)
{
    write(src, dest);
    return dest;
}

template <class T>
if_arithmetic_t<T, variant_ostream&> operator<<(variant_ostream& dest, T src)
{
    if constexpr (std::is_floating_point_v<T>)
        write((double)src, dest);
    else
        write((int64_t)src, dest);
    return dest;
}

inline auto range(const rowset& src)
{
    auto result = std::vector<row_t>{};
    for (auto is = variant_istream{src.data}; !is.data.empty();)
        for (auto& v : result.emplace_back(src.columns.size()))
            is >> v;
    return result;
}

namespace detail {

inline std::string format(const variant_t& src, const std::ostream& dest)
{
    std::ostringstream os;
    os.copyfmt(dest);
    os << src;
    return os.str();
}

struct line {
    std::vector<size_t>& sizes;
    const row_t& row;

    void fit(const std::ostream& dest)
    {
        for (size_t i = 0; i < sizes.size(); ++i)
            sizes[i] = std::max(sizes[i], format(row[i], dest).size());
    }

    friend std::ostream& operator<<(std::ostream& dest, const line& src)
    {
        for (size_t i = 0; i < src.sizes.size(); ++i) {
            auto str = format(src.row[i], dest);
            auto indent = src.sizes[i] - str.size();
            dest << "|" << std::setw(1) << "";
            if (std::holds_alternative<std::string_view>(src.row[i]))
                dest << str << std::setw(indent) << "";
            else
                dest << std::setw(indent) << "" << str;
            dest << std::setw(1) << "";
        }
        return dest << "|\n";
    }
};

}  // namespace detail

inline std::ostream& operator<<(std::ostream& dest, const rowset& src)
{
    auto columns = row_t{src.columns.begin(), src.columns.end()};
    auto rows = range(src);
    auto sizes = std::vector<size_t>(columns.size());
    detail::line{sizes, columns}.fit(dest);
    for (auto& row : rows)
        detail::line{sizes, row}.fit(dest);
    dest << std::setfill(' ') << detail::line{sizes, columns}
         << std::setfill('-') << detail::line{sizes, row_t(columns.size())}
         << std::setfill(' ');
    for (auto& row : rows)
        dest << detail::line{sizes, row};
    return dest;
}

}  // namespace step

#endif  // STEP_ROWSET_HPP
