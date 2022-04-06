#ifndef HAMMING_HPP
#define HAMMING_HPP

#include <boost/dynamic_bitset/dynamic_bitset.hpp>

#include <maybe.hpp>

namespace hamming {

boost::dynamic_bitset<> encode(const boost::dynamic_bitset<> &input);

Maybe<boost::dynamic_bitset<>> decode(boost::dynamic_bitset<> input,
                                      const uint8_t &bufferSize = 8);

boost::dynamic_bitset<> createHammingWord(const boost::dynamic_bitset<> &value,
                                          const uint8_t &N);

} // namespace hamming

#endif // !HAMMING_HPP
