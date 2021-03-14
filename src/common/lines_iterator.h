#pragma once

#include <iterator>
#include <string_view>

namespace detail {
namespace {

template <class ITERABLE>
static auto
begin(ITERABLE const& iterable)
{
    using std::begin;
    return begin(iterable);
}

template <class ITERABLE>
static auto
end(ITERABLE const& iterable)
{
    using std::end;
    return end(iterable);
}

}  // end namespace detail
}  // end anonymous namespac


template <class CHAR_ITER>
class lines
{
public:
    using Char_iterator = CHAR_ITER;

    lines(Char_iterator begin, Char_iterator end);

    template <class ITERABLE>
    explicit lines(ITERABLE const&);

    class iterator;
    iterator begin() const;
    iterator end() const;

private:
    Char_iterator begin_, end_;
};

template <class CHAR_ITER>
class lines<CHAR_ITER>::iterator
{
    friend class lines;

    using char_iter_traits  = std::iterator_traits<Char_iterator>;
    using char_type         = typename char_iter_traits::value_type;

public:
    using difference_type   = typename char_iter_traits::difference_type;
    using value_type        = std::basic_string_view<char_type>;
    using pointer           = value_type const*;
    using reference         = value_type const&;
    using iterator_category = std::forward_iterator_tag;

    // Constructs the empty iterator.
    iterator();

    // Constructs an iterator to the beginning of something that's
    // iterable.
    template <class ITERABLE>
    explicit iterator(ITERABLE const&);

    bool operator==(iterator) const;
    bool operator!=(iterator) const;

    reference operator*() const;
    pointer operator->() const;

    iterator& operator++();
    iterator operator++(int);

private:
    Char_iterator current_, limit_;
    value_type value_;

    iterator(Char_iterator, Char_iterator);

    bool empty_() const;
    void next_line_();
    void skip_eol_();
};

///
/// Deduction guides
///

template <class ITERABLE>
lines(ITERABLE const& iterable)
    -> lines<decltype(detail::begin(iterable))>;

///
/// Member function definitions for class lines
///

template <class I>
lines<I>::lines(Char_iterator begin, Char_iterator end)
        : begin_(begin),
          end_(end)
{ }

template <class I>
template <class ITERABLE>
lines<I>::lines(ITERABLE const& iterable)
        : lines(detail::begin(iterable), detail::end(iterable))
{ }

template <class I>
typename lines<I>::iterator
lines<I>::begin() const
{
    return iterator(begin_, end_);
}

template <class I>
typename lines<I>::iterator
lines<I>::end() const
{
    return iterator();
}

///
/// Member function definitions for class lines<I>::iterator
///

template <class I>
lines<I>::iterator::iterator(Char_iterator begin, Char_iterator end)
        : current_(begin),
          limit_(end)
{
    next_line_();
}

template <class I>
lines<I>::iterator::iterator()
        : current_(),
          limit_()
{ }

template <class I>
bool
lines<I>::iterator::operator==(iterator that) const
{
    return this->current_ == that.current_ ||
        (this->empty_() && that.empty_());
}

template <class I>
bool
lines<I>::iterator::operator!=(iterator that) const
{
    return !(*this == that);
}

template <class I>
typename lines<I>::iterator::reference
lines<I>::iterator::operator*() const
{
    return value_;
}

template <class I>
typename lines<I>::iterator::pointer
lines<I>::iterator::operator->() const
{
    return std::addressof(value_);
}

template <class I>
typename lines<I>::iterator&
lines<I>::iterator::operator++()
{
    skip_eol_();
    next_line_();
    return *this;
}

template <class I>
typename lines<I>::iterator
lines<I>::iterator::operator++(int)
{
    auto result(*this);
    ++*this;
    return result;
}

template <class I>
bool
lines<I>::iterator::empty_() const
{
    return current_ == limit_ && value_.empty();
}

template <class I>
void
lines<I>::iterator::next_line_()
{
    Char_iterator eol = current_;

    while (eol != limit_ && *eol != '\n' && *eol != '\r')
        ++eol;

    value_ = value_type(&*current_, eol - current_);
    current_ = eol;
}

template <class I>
void
lines<I>::iterator::skip_eol_()
{
    if (*current_ == '\n') {
        ++current_;
    } else if (*current_ == '\r') {
        ++current_;
        if (current_ != limit_ && *current_ == '\n') {
            ++current_;
        }
    }
}
