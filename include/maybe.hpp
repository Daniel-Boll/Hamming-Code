#ifndef MAYBE_HPP
#define MAYBE_HPP

#include <common.hpp>

// Maybe is a class that can hold a value of type T or not.
// It can be used to represent an optional value.
// It can be used to represent an error value.
//
// It is create through static member functions some and error.
//
// Usage:
//       Maybe<double> m = Maybe<double>::some(3.14);
//       Maybe<double> m = Maybe<double>::error("error message");
//       if (m.is_some()) {
//         double value = m.some();
//       } else if (m.is_error()) {
//         std::string error = m.error();
//       }
template <typename T> class Maybe {
public:
  // Create a Maybe that holds a value of type T.
  static Maybe<T> some(T value) { return Maybe<T>(value); }

  // Create a Maybe that holds an error.
  static Maybe<T> error(std::string error) { return Maybe<T>(error); }

  // Create a Maybe that holds a value of type T.
  Maybe(T value) : value_(value), error_("") {}

  // Create a Maybe that holds an error.
  Maybe(std::string error) : value_(), error_(error) {}

  // Return true if this Maybe holds a value.
  bool is_some() const { return error_.empty(); }

  // Return true if this Maybe holds an error.
  bool is_error() const { return !error_.empty(); }

  // Return the value if this Maybe holds a value.
  T some() const {
    if (is_some()) {
      return value_;
    } else {
      throw std::runtime_error("Maybe::some: this Maybe is an error");
    }
  }

  // Return the error if this Maybe holds an error.
  std::string error() const {
    if (is_error()) {
      return error_;
    } else {
      throw std::runtime_error("Maybe::error: this Maybe is a value");
    }
  }

private:
  T value_;
  std::string error_;
};

#endif // !MAYBE_HPP
