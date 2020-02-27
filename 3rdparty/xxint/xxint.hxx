#ifndef INT_PLUS_PLUS_H_
#define INT_PLUS_PLUS_H_

#include <iostream>
#include <climits>
#include <limits>
#include <stdexcept>

/// Namespace for int++.
namespace xxint {

/*
 * EXCEPTIONS
 */

/// Thrown when the result would be too large to fit in the required type.
struct overflow_too_large : std::overflow_error
{
    using overflow_error::overflow_error;
};

/// Thrown when the result would be too small to fit in the required type.
struct overflow_too_small : std::overflow_error
{
    using overflow_error::overflow_error;
};

/// Thrown when the operation would divide by 0.
struct overflow_div_zero : std::overflow_error
{
    using overflow_error::overflow_error;
};

/*
 * POLICIES
 */

/// Policies are class templates that specify how the `Checked` class
/// template behaves.
///
/// A policy is parametrized by the underlying checked type. It then specifies:
///
///  - Whether the type should wrap.
///
///  - What the type should do or return on division by zero.
///
///  - If non-wrapping, what the type should do or return when an operation
///    returns a value too small or too large for the type.
///
/// A policy is used by supplying it as the second argument to `Checked`. For
/// example, we can make a saturating short by writing
///
/// ```cpp
/// Checked<short, policy::saturating> cs;
/// ```
namespace policy {

/// Saturates on overflow, throws on divide-by-zero.
template<class T>
struct saturating
{
    /// Indicates that this policy does not wrap around.
    static constexpr bool is_wrapping = false;

    /// Returns the largest value of the type for when the result would be
    /// too large.
    static constexpr T too_large(const char*)
    {
        return std::numeric_limits<T>::max();
    }

    /// Returns the smallest value of the type for when the result would be
    /// too small.
    static T constexpr too_small(const char*)
    {
        return std::numeric_limits<T>::min();
    }

    /// Throws an `overflow_div_zero` exception.
    static T constexpr div_zero(const char* who)
    {
        throw overflow_div_zero(who);
    }
};

/// Throws on overflow or divide-by-zero.
template<class T>
struct throwing
{
    /// Indicates that this policy does not wrap around.
    static constexpr bool is_wrapping = false;

    /// Throws an `overflow_too_large` exception.
    static T constexpr too_large(const char* who)
    {
        throw overflow_too_large(who);
    }

    /// Throws an `overflow_too_small` exception.
    static T constexpr too_small(const char* who)
    {
        throw overflow_too_small(who);
    }

    /// Throws an `overflow_div_zero` exception.
    static T constexpr div_zero(const char* who)
    {
        throw overflow_div_zero(who);
    }
};

/// Wraps instead of overflowing, throws on divide-by-zero
template<class T>
struct wrapping
{
    /// Indicates that this is a wrapping policy.
    static constexpr bool is_wrapping = true;

    /// Throws an `overflow_div_zero` exception.
    static T constexpr div_zero(const char* who)
    {
        throw overflow_div_zero(who);
    }
};

} // end namespace policy

/*
 * INTERNAL DEFINITIONS
 * Includes type size calculations and comparisons.
 */

namespace detail {

/// Is type `A` wide enough to hold ever value of type `B`?
template<class A, class B>
constexpr bool is_as_wide_as()
{
    if (std::is_signed<B>::value == std::is_signed<A>::value)
        return sizeof(B) <= sizeof(A);

    if (std::is_unsigned<B>::value && std::is_signed<A>::value)
        return sizeof(B) < sizeof(A);

    return false;
}

/// Does type `A` include values lower than `B_MIN`?
template<class A, class B>
constexpr bool goes_lower_than()
{
    if (std::is_unsigned<A>::value) return false;

    if (std::is_unsigned<B>::value) return true;

    return sizeof(A) > sizeof(B);
}

/// Does type `A` include values higher than `B_MAX`?
template<class A, class B>
constexpr bool goes_higher_than()
{
    if (sizeof(A) == sizeof(B))
        return std::is_unsigned<A>::value && std::is_signed<B>::value;

    return sizeof(A) > sizeof(B);
}

/// Gets the minimum value of type `T` in type `Repr`.
///
/// PRECONDITION: `!goes_lower_than<T, Repr>()`
template<class T, class Repr>
constexpr Repr
min_as()
{
    return static_cast<Repr>(std::numeric_limits<T>::min());
}

/// Gets the maximum value of type `T` in type `Repr`.
///
/// PRECONDITION: `!goes_higher_than<T, Repr>()`
template<class T, class Repr>
constexpr Repr
max_as()
{
    return static_cast<Repr>(std::numeric_limits<T>::max());
}

/// Is `from` too small to fit in type `To`?
template<class To, class From>
constexpr bool is_too_small_for(From from)
{
    if (goes_lower_than<From, To>())
        return from < min_as<To, From>();
    else
        return false;
}

/// Is `from` too large to fit in type `To`?
template<class To, class From>
constexpr bool is_too_large_for(From from)
{
    if (goes_higher_than<From, To>())
        return from > max_as<To, From>();
    else
        return false;
}

/// Do the two values have the same sign?
///
/// 0 is considered non-negative.
template<class T>
constexpr bool same_sign(T a, T b)
{
    return (a ^ b) >= 0;
}

} // end detail

/*
 * CONVERSIONS
 */

/// Specialized based on the ranges of the `To` and `From` types, and the
/// `Policy`.
///
/// A structure type `Convert<To, From, Policy>` defines a public static member
/// `To convert(From)` that converts from `From` to `To`, acting according to
/// `Policy` if the value does not fit. For example, we can perform a
/// saturating conversion from `short` to `signed char`:
///
/// ```cpp
/// short s = 130;
/// signed char c = Convert<signed char, short, policy::saturating>::convert(s);
/// assert(c == 127);
/// ```
///
/// Additionally, if all values of type `From` fit in type `To` (that is, if
/// `To` is as wide as `From`), then `Convert<To, From, Policy>` defines a
/// public static member `To widen(From)`. This can be used to statically
/// allow only widening conversions.
template <class To,
          class from,
          template <class> class Policy = policy::throwing,
          class Enable = void>
struct Convert;

/// This specialization implements widening (non-lossy) conversions.
template <class To, class From, template <class> class Policy>
struct Convert<To, From, Policy,
        std::enable_if_t<detail::is_as_wide_as<To, From>()>>
{
    /// Widens to `To` from `From`.
    ///
    /// Because this is a widening, policy `Policy` is irrelevant.
    static constexpr To convert(From from)
    {
        return static_cast<To>(from);
    }

    /// Only for widening conversions do we provide the widen function.
    static constexpr To widen(From from)
    {
        return static_cast<To>(from);
    }
};

/// Non-wrapping conversion where the value might be too low.
template <class To, class From, template <class> class Policy>
struct Convert<To, From, Policy,
        std::enable_if_t<detail::goes_lower_than<From, To>()
                         && !detail::goes_higher_than<From, To>()
                         && !Policy<To>::is_wrapping>>
{
    /// Converts to `To` from `From`, handling bounds errors according to
    /// policy `Policy`.
    static constexpr To convert(From from)
    {
        if (detail::is_too_small_for<To>(from))
            return Policy<To>::too_small("Convert");
        return static_cast<To>(from);
    }
};

/// Non-wrapping conversion where the value might be too low or too high.
template <class To, class From, template <class> class Policy>
struct Convert<To, From, Policy,
        std::enable_if_t<detail::goes_lower_than<From, To>()
                         && detail::goes_higher_than<From, To>()
                         && !Policy<To>::is_wrapping>>
{
    /// Converts to `To` from `From`, handling bounds errors according to
    /// policy `Policy`.
    static constexpr To convert(From from)
    {
        if (detail::is_too_small_for<To>(from))
            return Policy<To>::too_small("Convert");
        if (detail::is_too_large_for<To>(from))
            return Policy<To>::too_large("Convert");
        return static_cast<To>(from);
    }
};

/// Non-wrapping conversion where the value might be too high.
template <class To, class From, template <class> class Policy>
struct Convert<To, From, Policy,
        std::enable_if_t<!detail::goes_lower_than<From, To>()
                         && detail::goes_higher_than<From, To>()
                         && !Policy<To>::is_wrapping>>
{
    /// Converts to `To` from `From`, handling bounds errors according to
    /// policy `Policy`.
    static constexpr To convert(From from)
    {
        if (detail::is_too_large_for<To>(from))
            return Policy<To>::too_large("Convert");
        return static_cast<To>(from);
    }
};

/// Wrapping, non-widening conversion.
template <class To, class From, template <class> class Policy>
struct Convert<To, From, Policy,
        std::enable_if_t<Policy<To>::is_wrapping &&
                         !detail::is_as_wide_as<To, From>()>>
{
    /// Converts to `To` from `From`, handling bounds errors according to
    /// policy `Policy`.
    static constexpr To convert(From from)
    {
        using UFrom = std::make_unsigned_t<From>;
        using UTo   = std::make_unsigned_t<From>;

        return static_cast<To>(static_cast<UTo>(static_cast<UFrom>(from)));
    }
};

/// Convenience function for converting using `policy::throwing`.
template <class To, class From>
constexpr To convert_exn(From from)
{
    return Convert<To, From, policy::throwing>::convert(from);
}

/// Convenience function for converting using `policy::saturating`.
template <class To, class From>
constexpr To convert_sat(From from)
{
    return Convert<To, From, policy::saturating>::convert(from);
}

/// Convenience function that types only for widening conversions.
template <class To, class From>
constexpr To convert_widen(From from)
{
    return Convert<To, From, policy::throwing>::widen(from);
}

/*
 * CHECKED INTEGERS
 */

/// `Checked<T, P>` specifies an integer type `T` and a policy `P`. It is
/// specialized based on signedness and wrapping.
template <class T,
          template<class> class P = policy::throwing,
          class Enable = void>
class Checked;

/// Non-wrapping, signed integers.
template <class T, template<class> class P>
class Checked<T, P,
        std::enable_if_t<std::is_signed<T>::value && !P<T>::is_wrapping>>
{
private:
    using unsigned_t = std::make_unsigned_t<T>;
    using policy_t   = P<T>;

    T value_;

    static constexpr T T_MIN_ = std::numeric_limits<T>::min();
    static constexpr T T_MAX_ = std::numeric_limits<T>::max();

    template <typename U>
    static constexpr Checked rebuild_(U value)
    {
        return static_cast<T>(value);
    }

    template <class U, template <class> class Q, class F>
    friend class Checked;

public:
    /// Non-converting constructor, defaults to 0.
    constexpr Checked(T value = T()) : value_(value)
    { }

    /// Converts from any numeric type according to policy `P`.
    template <class U>
    constexpr Checked(U value) : value_(Convert<T, U, P>::convert(value))
    { }

    /// Converts automatically from any equal or narrower `Checked` type.
    template <class U, template<class> class Q>
    constexpr Checked(Checked<U, Q> other) : value_(Convert<T, U, P>::widen(other.value_))
    { }

    /// Gets the contained `T` value.
    constexpr T get() const
    {
        return value_;
    }

    /// Converts to another `Checked` type, checking the conversion according
    /// to the old policy `P`.
    template <class U, template <class> class Q = P>
    constexpr Checked<U, Q> convert() const
    {
        return Checked<U, Q>(Convert<U, T, P>::convert(get()));
    }

    /// Checked negation.
    constexpr Checked operator-() const
    {
        if (std::is_signed<T>::value && value_ == T_MIN_)
            return policy_t::too_large("Checked::operator-()");
        else
            return rebuild_(-value_);
    }

    /// Absolute value.
    constexpr unsigned_t abs() const
    {
        if (value_ == T_MIN_) {
            return unsigned_t(std::numeric_limits<T>::max()) + 1;
        } else if (value_ < 0) {
            return unsigned_t(-value_);
        } else {
            return unsigned_t(value_);
        }
    }

    /// Checked addition.
    constexpr Checked operator+(Checked other) const
    {
#if __has_builtin(__builtin_add_overflow)
        Checked result;
        if (__builtin_add_overflow(value_, other.value_, &result.value_)) {
            if (value_ >= 0)
                return policy_t::too_large("Checked::operator+(Checked)");
            else
                return policy_t::too_small("Checked::operator+(Checked)");
        } else {
            return result;
        }
#else
        if (value_ >= 0) {
            if (other.value_ > T_MAX_ - value_)
                return policy_t::too_large("Checked::operator+(Checked)");
        } else {
            if (other.value_ < T_MIN_ - value_)
                return policy_t::too_small("Checked::operator+(Checked)");
        }

        return rebuild_(value_ + other.value_);
#endif
    }

    /// Checked subtraction.
    constexpr Checked operator-(Checked other) const
    {
#if __has_builtin(__builtin_sub_overflow)
        Checked result;
        if (__builtin_sub_overflow(value_, other.value_, &result.value_)) {
            if (value_ >= 0)
                return policy_t::too_large("Checked::operator-(Checked)");
            else
                return policy_t::too_small("Checked::operator-(Checked)");
        } else {
            return result;
        }
#else
        if (value_ >= 0) {
            if (other.value_ < value_ - T_MAX_)
                return policy_t::too_large("Checked::operator-(Checked)");
        } else {
            if (other.value_ > value_ - T_MIN_)
                return policy_t::too_small("Checked::operator-(Checked)");
        }

        return rebuild_(value_ - other.value_);
#endif
    }

    /// Checked multiplication.
    constexpr Checked operator*(Checked other) const
    {
        auto overflow = [=]() {
            if (detail::same_sign(value_, other.value_))
                return policy_t::too_large("Checked::operator*(Checked)");
            else
                return policy_t::too_small("Checked::operator*(Checked)");
        };

#if __has_builtin(__builtin_mul_overflow)
        Checked result;
        if (__builtin_mul_overflow(value_, other.value_, &result.value_)) {
            return overflow();
        } else {
            return result;
        }
#else
        // This is slow right now, because it does a division. There are better
        // ways of doing it, depending on the size of T.
        if (other.value_ != 0) {
            if (abs() > unsigned_t(T_MAX_) / other.abs()) {
                return overflow();
            }
        }

        return rebuild_(value_ * other.value_);
#endif
    }

    /// Checked division.
    constexpr Checked operator/(Checked other) const
    {
        if (value_ == T_MIN_ && other.value_ == -1)
            return policy_t::too_large("Checked::operator/(Checked)");

        if (other.value_ == 0) {
            return policy_t::div_zero("Checked::operator/(Checked)");
        }

        return rebuild_(value_ / other.value_);
    }

    /// Checked modulus.
    constexpr Checked operator%(Checked other) const
    {
        if (other.value_ == 0) {
            return policy_t::div_zero("Checked::operator%(Checked)");
        }

        return rebuild_(value_ % other.value_);
    }

    /// Bitwise and.
    constexpr Checked operator&(Checked other) const
    {
        return rebuild_(value_ & other.value_);
    }

    /// Bitwise or.
    constexpr Checked operator|(Checked other) const
    {
        return rebuild_(value_ | other.value_);
    }

    /// Bitwise xor.
    constexpr Checked operator^(Checked other) const
    {
        return rebuild_(value_ ^ other.value_);
    }

    /// Checked left shift.
    constexpr Checked operator<<(u_int8_t other) const
    {
        if (value_ == 0)
            return rebuild_(0);

        if (value_ > 0) {
            if (other >= sizeof(T) * CHAR_BIT
                || T_MAX_ >> other < value_)
                return policy_t::too_large("Checked::operator<<(u_int8_t)");
        } else {
            if (other >= sizeof(T) * CHAR_BIT
                || T_MIN_ >> other > value_)
                return policy_t::too_small("Checked::operation<<(u_int8_t)");
        }

        return rebuild_(value_ << other);
    }

    /// Right shift.
    constexpr Checked operator>>(u_int8_t other) const
    {
        return rebuild_(value_ >> other);
    }

    /// Bitwise not.
    constexpr Checked operator~() const
    {
        return rebuild_(~value_);
    }

    /// Checked +=
    constexpr Checked& operator+=(Checked other)
    {
        return *this = *this + other;
    }

    /// Checked -=
    constexpr Checked& operator-=(Checked other)
    {
        return *this = *this - other;
    }

    /// Checked *=
    constexpr Checked& operator*=(Checked other)
    {
        return *this = *this * other;
    }

    /// Checked /=
    constexpr Checked& operator/=(Checked other)
    {
        return *this = *this / other;
    }

    /// Checked %=
    constexpr Checked& operator%=(Checked other)
    {
        return *this = *this % other;
    }

    /// &=
    constexpr Checked& operator&=(Checked other)
    {
        return *this = *this & other;
    }

    /// |=
    constexpr Checked& operator|=(Checked other)
    {
        return *this = *this | other;
    }

    /// ^=
    constexpr Checked& operator^=(Checked other)
    {
        return *this = *this | other;
    }

    /// Checked <<=
    constexpr Checked& operator<<=(u_int8_t other)
    {
        return *this = *this << other;
    }

    /// >>=
    constexpr Checked& operator>>=(u_int8_t other)
    {
        return *this = *this >> other;
    }

    /// Checked preincrement
    constexpr Checked& operator++()
    {
        return *this += 1;
    }

    /// Checked predecrement.
    constexpr Checked& operator--()
    {
        return *this -= 1;
    }

    /// Checked postincrement.
    constexpr Checked& operator++(int)
    {
        Checked old = *this;
        ++*this;
        return old;
    }

    /// Checked postdecrement.
    constexpr Checked& operator--(int)
    {
        Checked old = *this;
        --*this;
        return old;
    }
};

/// Non-wrapping, unsigned integers.
template <class T, template <class> class P>
class Checked<T, P,
        std::enable_if_t<std::is_unsigned<T>::value && !P<T>::is_wrapping>>
{
private:
    using policy_t   = P<T>;

    T value_;

    static constexpr T T_MAX_ = std::numeric_limits<T>::max();

    template <typename U>
    static constexpr Checked rebuild_(U value)
    {
        return static_cast<T>(value);
    }

    template <class U, template <class> class Q, class F>
    friend class Checked;

public:
    /// Non-converting constructor, defaults to 0.
    constexpr Checked(T value = T()) : value_(value)
    { }

    /// Converts from any numeric type according to policy `P`.
    template <class U>
    constexpr Checked(U value) : value_(Convert<T, U, P>::convert(value))
    { }

    /// Converts automatically from any equal or narrower `Checked` type.
    template <class U, template <class> class Q>
    constexpr Checked(Checked<U, Q> other) : value_(Convert<T, U, P>::widen(other.value_))
    { }

    /// Gets the contained `T` value.
    constexpr T get() const
    {
        return value_;
    }

    /// Converts to another `Checked` type, checking the conversion according
    /// to the old policy `P`.
    template <class U, template <class> class Q = P>
    constexpr Checked<U, Q> convert() const
    {
        return Checked<U, Q>(Convert<U, T, P>::convert(get()));
    }

    /// Checked negation.
    constexpr Checked operator-() const
    {
        if (value_ == T(0))
            return rebuild_(value_);
        else
            return policy_t::too_small("Checked::operator-()");
    }

    /// Absolute value.
    constexpr T abs() const
    {
        return value_;
    }

    /// Checked addition.
    constexpr Checked operator+(Checked other) const
    {
#if __has_builtin(__builtin_add_overflow)
        Checked result;
        if (__builtin_add_overflow(value_, other.value_, &result.value_)) {
            return policy_t::too_large("Checked::operator+(Checked)");
        } else {
            return result;
        }
#else
        if (value_ > T_MAX_ - other.value_)
            return policy_t::too_large("Checked::operator+(Checked)");

        return rebuild_(value_ + other.value_);
#endif
    }

    /// Checked subtraction.
    constexpr Checked operator-(Checked other) const
    {
        if (other.value_ > value_)
            return policy_t::too_small("Checked::operator-(Checked)");

        return rebuild_(value_ - other.value_);
    }

    /// Checked multiplication.
    Checked operator*(Checked other) const
    {
#if __has_builtin(__builtin_mul_overflow)
        Checked result;
        if (__builtin_mul_overflow(value_, other.value_, &result.value_)) {
            return policy_t::too_large("Checked::operator*(Checked)");
        } else {
            return result;
        }
#else
        // This is slow right now, because it does a division. There are better
        // ways of doing it, depending on the size of T.
        if (other.value_ != 0) {
            if (value_ > T_MAX_ / other.value_)
                return policy_t::too_large("Checked::operator*(Checked)");
        }

        return rebuild_(value_ * other.value_);
#endif
    }

    /// Checked division.
    constexpr Checked operator/(Checked other) const
    {
        if (other.value_ == 0)
            return policy_t::div_zero("Checked::operator/(Checked)");

        return rebuild_(value_ / other.value_);
    }

    /// Checked remainder.
    constexpr Checked operator%(Checked other) const
    {
        if (other.value_ == 0)
            return policy_t::div_zero("Checked::operator%(Checked)");

        return rebuild_(value_ % other.value_);
    }

    /// Bitwise and.
    constexpr Checked operator&(Checked other) const
    {
        return rebuild_(value_ & other.value_);
    }

    /// Bitwise or.
    constexpr Checked operator|(Checked other) const
    {
        return rebuild_(value_ | other.value_);
    }

    /// Bitwise xor.
    constexpr Checked operator^(Checked other) const
    {
        return rebuild_(value_ ^ other.value_);
    }

    /// Checked left shift.
    constexpr Checked operator<<(u_int8_t other) const
    {
        if (value_ == 0)
            return rebuild_(0);

        if (other >= sizeof(T) * CHAR_BIT)
            return policy_t::too_large("Checked::operator<<(u_int8_t)");

        if (T_MAX_ >> other < value_)
            return policy_t::too_large("Checked::operator<<(u_int8_t)");

        return rebuild_(value_ << other);
    }

    /// Right shift.
    constexpr Checked operator>>(u_int8_t other) const
    {
        return rebuild_(value_ >> other);
    }

    /// Bitwise not.
    constexpr Checked operator~() const
    {
        return rebuild_(~value_);
    }

    /// Checked +=
    constexpr Checked& operator+=(Checked other)
    {
        return *this = *this + other;
    }

    /// Checked -=
    constexpr Checked& operator-=(Checked other)
    {
        return *this = *this - other;
    }

    /// Checked *=
    constexpr Checked& operator*=(Checked other)
    {
        return *this = *this * other;
    }

    /// Checked /=
    constexpr Checked& operator/=(Checked other)
    {
        return *this = *this / other;
    }

    /// Checked %=
    constexpr Checked& operator%=(Checked other)
    {
        return *this = *this % other;
    }

    /// &=
    constexpr Checked& operator&=(Checked other)
    {
        return *this = *this & other;
    }

    /// |=
    constexpr Checked& operator|=(Checked other)
    {
        return *this = *this | other;
    }

    /// ^=
    constexpr Checked& operator^=(Checked other)
    {
        return *this = *this | other;
    }

    /// Checked <<=
    constexpr Checked& operator<<=(u_int8_t other)
    {
        return *this = *this << other;
    }

    /// >>=
    constexpr Checked& operator>>=(u_int8_t other)
    {
        return *this = *this >> other;
    }

    /// Checked preincrement.
    constexpr Checked& operator++()
    {
        return *this += 1;
    }

    /// Checked predecrement.
    constexpr Checked& operator--()
    {
        return *this -= 1;
    }

    /// Checked postincrement.
    constexpr Checked& operator++(int)
    {
        Checked old = *this;
        ++*this;
        return old;
    }

    /// Checked postdecrement.
    constexpr Checked& operator--(int)
    {
        Checked old = *this;
        --*this;
        return old;
    }
};

/// Wrapping integers (potentially signed)
template <class T, template <class> class P>
class Checked<T, P, std::enable_if_t<P<T>::is_wrapping>>
{
private:
    using policy_t = P<T>;
    using unsigned_t = std::make_unsigned_t<T>;

    unsigned_t value_;

    template <class U, template <class> class Q, class F>
    friend class Checked;

    // Make sure we have two's complement numbers, because Wrapping<T> depends
    // on conversion to unsigned and back:
    static_assert(static_cast<unsigned_t>(T(-3)) ==
                          std::numeric_limits<unsigned_t>::max() - 2,
                  "Two's complement check");
    static_assert(static_cast<T>(
                          std::numeric_limits<unsigned_t>::max() - 2)
                  == T(-3),
                  "Two's complement check");

public:
    /// Non-converting constructor, defaults to 0.
    constexpr Checked(T value = T()) : value_{unsigned_t(value)}
    { }

    /// Converts from any numeric type by wrapping.
    template <class U>
    constexpr Checked(U value) : value_{unsigned_t(value)}
    { }

    /// Converts automatically from any equal or narrower `Checked` type.
    template <class U, template <class> class Q>
    constexpr Checked(Checked<U, Q> other)
            : Checked(convert_widen<T>(other.value_))
    { }

    /// Gets the contained `T` value.
    constexpr T get() const
    {
        return static_cast<T>(value_);
    }

    /// Converts to another `Checked` type, checking the conversion according
    /// to the old policy `P`.
    template <class U, template <class> class Q = P>
    constexpr Checked<U, Q> convert() const
    {
        return Checked<U, Q>(Convert<U, T, P>::convert(get()));
    }

    /// Wrapping negation.
    constexpr Checked operator-() const {
        if (get() == std::numeric_limits<T>::min())
            return Checked(std::numeric_limits<T>::min());
        else
            return Checked(-get());
    }

    /// Absolute value.
    constexpr unsigned_t abs() const
    {
        if (get() == std::numeric_limits<T>::min()) {
            return unsigned_t(std::numeric_limits<T>::max()) + 1;
        } else if (get() < 0) {
            return unsigned_t(-get());
        } else {
            return unsigned_t(get());
        }
    }

    /// Wrapping addition.
    constexpr Checked operator+(Checked other) const
    {
        return Checked(value_ + other.value_);
    }

    /// Wrapping subtraction.
    constexpr Checked operator-(Checked other) const
    {
        return Checked(value_ - other.value_);
    }

    /// Wrapping multiplication.
    constexpr Checked operator*(Checked other) const
    {
        return Checked(value_ * other.value_);
    }

    /// Wrapping division.
    constexpr Checked operator/(Checked other) const
    {
        if (other.value_ == 0)
            return policy_t::div_zero("Checked::operator/(Checked)");

        return Checked(value_ / other.value_);
    }

    /// Wrapping remainder.
    constexpr Checked operator%(Checked other) const
    {
        if (other.value_ == 0)
            return policy_t::div_zero("Checked::operator/(Checked)");

        return Checked(value_ % other.value_);
    }

    /// Bitwise and.
    constexpr Checked operator&(Checked other) const
    {
        return Checked(value_ & other.value_);
    }

    /// Bitwise or.
    constexpr Checked operator|(Checked other) const
    {
        return Checked(value_ | other.value_);
    }

    /// Bitwise xor.
    constexpr Checked operator^(Checked other) const
    {
        return Checked(value_ ^ other.value_);
    }

    /// Wrapping left shift.
    constexpr Checked operator<<(u_int8_t other) const
    {
        return Checked(value_ << other);
    }

    /// Right shift.
    constexpr Checked operator>>(u_int8_t other) const
    {
        return Checked(get() >> other);
    }

    /// Bitwise not.
    constexpr Checked operator~() const
    {
        return Checked(~value_);
    }

    /// Wrapping +=
    constexpr Checked& operator+=(Checked other)
    {
        return *this = *this + other;
    }

    /// Wrapping -=
    constexpr Checked& operator-=(Checked other)
    {
        return *this = *this - other;
    }

    /// Wrapping *=
    constexpr Checked& operator*=(Checked other)
    {
        return *this = *this * other;
    }

    /// Wrapping /=
    constexpr Checked& operator/=(Checked other)
    {
        return *this = *this / other;
    }

    /// Wrapping %=
    constexpr Checked& operator%=(Checked other)
    {
        return *this = *this % other;
    }

    /// &=
    constexpr Checked& operator&=(Checked other)
    {
        return *this = *this & other;
    }

    /// |=
    constexpr Checked& operator|=(Checked other)
    {
        return *this = *this | other;
    }

    /// ^=
    constexpr Checked& operator^=(Checked other)
    {
        return *this = *this ^ other;
    }

    /// Wrapping <<=
    constexpr Checked& operator<<=(u_int8_t other)
    {
        return *this = *this << other;
    }

    /// >>=
    constexpr Checked& operator>>=(u_int8_t other)
    {
        return *this = *this >> other;
    }

    /// Wrapping preincrement.
    constexpr Checked& operator++()
    {
        return *this += 1;
    }

    /// Wrapping predecrement.
    constexpr Checked& operator--()
    {
        return *this -= 1;
    }

    /// Wrapping postincrement.
    constexpr Checked& operator++(int)
    {
        Checked old = *this;
        ++*this;
        return old;
    }

    /// Wrapping predecrement.
    constexpr Checked& operator--(int)
    {
        Checked old = *this;
        --*this;
        return old;
    }
};

/// Alias for saturating integers.
template <class T>
using Saturating = Checked<T, policy::saturating>;

/// Alias for wrapping integers.
template <class T>
using Wrapping = Checked<T, policy::wrapping>;

/*
 * Checked integer comparisons and stream operations:
 */

/// Equality between possibly mixed checked types.
template <class T, template <class> class P,
        class U, template <class> class Q>
constexpr bool operator==(Checked<T, P> a, Checked<U, Q> b)
{
    if (detail::is_too_large_for<U>(a.get()))
        return false;

    if (detail::is_too_small_for<U>(a.get()))
        return false;

    return static_cast<U>(a.get()) == b.get();
}

/// Inequality between possibly mixed checked types.
template <class T, template <class> class P,
        class U, template <class> class Q>
constexpr bool operator!=(Checked<T, P> a, Checked<U, Q> b)
{
    return !(a == b);
}

/// Less-than for possibly mixed checked types.
template <class T, template <class> class P,
        class U, template <class> class Q>
constexpr bool operator<(Checked<T, P> a, Checked<U, Q> b)
{
    if (detail::is_too_large_for<U>(a.get()))
        return false;

    if (detail::is_too_small_for<U>(a.get()))
        return true;

    return static_cast<U>(a.get()) < b.get();
}

/// Less-than-or-equal for possibly mixed checked types.
template <class T, template <class> class P,
        class U, template <class> class Q>
constexpr bool operator<=(Checked<T, P> a, Checked<U, Q> b)
{
    return !(b < a);
}

/// Greater-than for possibly mixed checked types.
template <class T, template <class> class P,
        class U, template <class> class Q>
constexpr bool operator>(Checked<T, P> a, Checked<U, Q> b)
{
    return b < a;
}

/// Greater-than-or-equal for possibly mixed checked types.
template <class T, template <class> class P,
        class U, template <class> class Q>
constexpr bool operator>=(Checked<T, P> a, Checked<U, Q> b)
{
    return !(a < b);
}

/// Equality for a checked type and an unwrapped integer type.
template <class T, template <class> class P, class U>
constexpr bool operator==(Checked<T, P> a, U b)
{
    return a == Checked<U, P>(b);
}

/// Inequality for a checked type and an unwrapped integer type.
template <class T, template <class> class P, class U>
constexpr bool operator!=(Checked<T, P> a, U b)
{
    return a != Checked<U, P>(b);
}

/// Less-than for a checked type and an unwrapped integer type.
template <class T, template <class> class P, class U>
constexpr bool operator<(Checked<T, P> a, U b)
{
    return a < Checked<U, P>(b);
}

/// Less-than-or-equal for a checked type and an unwrapped integer type.
template <class T, template <class> class P, class U>
constexpr bool operator<=(Checked<T, P> a, U b)
{
    return a <= Checked<U, P>(b);
}

/// Greater-than for a checked type and an unwrapped integer type.
template <class T, template <class> class P, class U>
constexpr bool operator>(Checked<T, P> a, U b)
{
    return a > Checked<U, P>(b);
}

/// Greater-than-or-equal for a checked type and an unwrapped integer type.
template <class T, template <class> class P, class U>
constexpr bool operator>=(Checked<T, P> a, U b)
{
    return a >= Checked<U, P>(b);
}

/// Stream insertion for checked types.
template <class T, template <class> class P>
std::ostream& operator<<(std::ostream& o, Checked<T, P> a)
{
    return o << a.get();
}

/// Stream extraction for checked types.
///
/// This reads directly into a variable of type `T`; it performs no checking.
template <class T, template <class> class P>
std::istream& operator>>(std::istream& i, Checked<T, P>& a)
{
    T temp;
    i >> temp;
    a = Checked<T, P>(temp);
    return i;
}

}

#endif
