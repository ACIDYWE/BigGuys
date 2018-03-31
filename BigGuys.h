#pragma once

#include <iostream>
#include <stdexcept>
#include <cstring>
#include <string>
#include <iomanip>
#include <tuple>

using std::string;

template <typename T>
class BigGuys {
    public:
      BigGuys(size_t t=0);
      BigGuys(string const);
      BigGuys(BigGuys<T> const &);
      ~BigGuys();
      T& operator[] (size_t) const;
      BigGuys<T>& operator= (BigGuys<T> const &);
      template<class U> friend std::ostream& operator << (std::ostream &, const BigGuys<U> &);
      template<class U> friend std::istream& operator >> (std::istream &, BigGuys<U> &);
      BigGuys<T> operator+ (BigGuys<T> const &);
      BigGuys<T> operator- (BigGuys<T> const &);
      BigGuys<T> operator* (BigGuys<T> const &);
      std::tuple<BigGuys<T>, BigGuys<T>> operator/ (BigGuys<T> &);
      bool operator> (BigGuys<T> const &) const;
      bool operator >= (BigGuys<T> const &source) const;
      bool operator == (BigGuys<T> const &source);
      BigGuys<T> mul_base(T);
      std::tuple<BigGuys<T>, T> div_base(T);
      T mod_base(T);
      size_t get_cap() const;
      size_t get_len() const;
      BigGuys<T> power(const BigGuys<T>&, const BigGuys<T>&) const;
    private:
      size_t get_binary_len() const;
      size_t get_binary_index(size_t) const;
      void clear_insig();
      T* guy;
      size_t len;
      size_t cap;
};
