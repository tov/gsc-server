#pragma once

#include <vector>

template <class T> class auto_vector : public std::vector<T> {
public:
  using Base = std::vector<T>;
  using typename Base::const_iterator;
  using typename Base::const_reference;
  using typename Base::const_reverse_iterator;
  using typename Base::difference_type;
  using typename Base::iterator;
  using typename Base::reference;
  using typename Base::reverse_iterator;
  using typename Base::value_type;
  using typename Base::vector;

  reference at(size_t ix) {
    if (!in_bounds_(ix))
      Base::resize(ix + 1);

    return Base::operator[](ix);
  }

  reference operator[](size_t ix) { return at(ix); }

  const_reference at(size_t ix) const {
    if (in_bounds_(ix))
      return Base::operator[](ix);
    else
      return value_type{};
  }

  const_reference operator[](size_t ix) const { return at(ix); }

private:
  bool in_bounds_(size_t ix) const { return ix < Base::size(); }
};
