#pragma once

#include "../specializations.h"

#include <istream>
#include <ostream>
#include <string>
#include <vector>

class Bytes
{
public:
    using value_type = unsigned char;

    Bytes() = default;

    explicit Bytes(const std::string&);

    Bytes(std::istream&, int size);

    Bytes(Bytes const&) = default;

    Bytes(Bytes&&) = default;

    Bytes& operator=(Bytes const&) = default;

    Bytes& operator=(Bytes&&) = default;

    void write(std::ostream&) const;

    explicit operator std::string() const;

    char const* data() const noexcept
    { return (char const*)data_.data(); }

    bool empty() const
    { return data_.empty(); }

    std::size_t size() const
    { return data_.size(); }

    value_type& back()
    { return data_.back(); }

    value_type back() const
    { return data_.back(); }

    using iterator = std::vector<value_type>::iterator;
    using const_iterator = std::vector<value_type>::const_iterator;

    iterator begin() noexcept
    { return data_.begin(); }

    iterator end() noexcept
    { return data_.end(); }

    const_iterator begin() const noexcept
    { return data_.begin(); }

    const_iterator end() const noexcept
    { return data_.end(); }

    const_iterator cbegin() const noexcept
    { return data_.cbegin(); }

    const_iterator cend() const noexcept
    { return data_.cend(); }

private:
    std::vector<value_type> data_;

    friend Wt::Dbo::sql_value_traits<Bytes, void>;
};

