#ifndef UTILS_HPP
#define UTILS_HPP

#include <common.hpp>

#include <boost/dynamic_bitset/dynamic_bitset.hpp>

namespace utils {

namespace bit {
constexpr bool isPowerOfTwo(const uint64_t &x) { return x && (!(x & (x - 1))); }

constexpr uint8_t bit2Byte(const uint16_t &bit) { return bit / 8; }

constexpr uint8_t findMinBytes(const uint64_t &N) {
  const uint8_t res = N / 8;
  return res % 8 == 0 ? res : res + 1;
}

inline boost::dynamic_bitset<> concat(const boost::dynamic_bitset<> &a,
                                      const boost::dynamic_bitset<> &b) {
  // If a is full zero, return b
  if (a.none())
    return b;
  boost::dynamic_bitset<> result(a.size() + b.size(), 0);

  for (int i = 0; i < a.size(); i++)
    result[i] = a[i];

  for (int i = 0; i < b.size(); i++)
    result[i + a.size()] = b[i];

  return result;
}

} // namespace bit

namespace hamming {

constexpr bool isNotC(const uint64_t &x) {
  return !utils::bit::isPowerOfTwo(x) && x != 0;
}

// find the minimum necessary number of bits to represent a number of N bits in
// hamming code given the formula 2^k - 1 >= m + k, we can find the minimum k
// needed to represent m
constexpr uint8_t findMinK(const uint64_t &N) {
  uint8_t k = 0;
  while (N > (1 << k) - 1) {
    k++;
  }
  return k;
}

// Extract original C and M from a hamming code
inline boost::dynamic_bitset<> extractC(const boost::dynamic_bitset<> &input,
                                        const uint8_t &bufferSize) {
  boost::dynamic_bitset<> C(findMinK(bufferSize), 0);

  uint8_t auxC = 0;
  for (int i = 0; i < input.size(); i++) {
    if (utils::bit::isPowerOfTwo(i))
      C[auxC++] = input[i];
  }

  return C;
}

inline boost::dynamic_bitset<> extractM(const boost::dynamic_bitset<> &input,
                                        const uint8_t &bufferSize) {
  boost::dynamic_bitset<> M(bufferSize, 0);

  uint8_t auxM = 0;
  for (int i = 0; i < input.size(); i++)
    if (utils::hamming::isNotC(i))
      M[auxM++] = input[i];

  return M;
}

} // namespace hamming
} // namespace utils

#endif // !UTILS_HPP
