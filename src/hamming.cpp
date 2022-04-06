#include <common.hpp>

#include <hamming.hpp>
#include <utils.hpp>

/*
 * Hamming Example (1 byte)
 *
 * We have the following structure for 1 byte of data:
 *     ┌──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┐
 *     │12│11│10│09│08│07│06│05│04│03│02│01│00│
 *     ├──┼──┼──┼──┼──┼──┼──┼──┼──┼──┼──┼──┼──┤
 *     │M8│M7│M6│M5│C8│M4│M3│M2│C4│M1│C2│C1│.G│
 *     └──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┘
 *
 * And the following strucutre for the Cs:
 *     ┌──┬──┬──┬──┬──┬──┬──┬──┬──┐
 *     │C1│M1│M2│  │M4│M5│  │M7│  │
 *     ├──┼──┼──┼──┼──┼──┼──┼──┼──┤
 *     │C2│M1│  │M3│M4│  │M6│M7│  │
 *     ├──┼──┼──┼──┼──┼──┼──┼──┼──┤
 *     │C4│  │M2│M3│M4│  │  │  │M8│
 *     ├──┼──┼──┼──┼──┼──┼──┼──┼──┤
 *     │C8│  │  │  │  │M5│M6│M7│M8│
 *     └──┴──┴──┴──┴──┴──┴──┴──┴──┘
 *
 * So, for the number 11110000
 *     ┌──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┐
 *     │12│11│10│09│08│07│06│05│04│03│02│01│00│
 *     ├──┼──┼──┼──┼──┼──┼──┼──┼──┼──┼──┼──┼──┤
 *     │1 │1 │1 │1 │x │0 │0 │0 │x │0 │x │x │x │
 *     └──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┘
 *
 * C1: M5 and M7 are on, so C1 is          0  | C1
 * C2: M6 and M7 are on, so C2 is          0  | C2
 * C4: M8 is one, so C4 is                 1  | C4
 * C8: M5, M6, M7, and M8 are on, so C8 is 0  | C8
 * */

boost::dynamic_bitset<>
hamming::createHammingWord(const boost::dynamic_bitset<> &value,
                           const uint8_t &N) {
  boost::dynamic_bitset<> bits(N, 0);
  int_fast8_t mx = 0;
  for (int_fast8_t i = 0; i < N; i++)
    if (utils::hamming::isNotC(i))
      bits[i] = value[mx++];

  return bits;
}

// For each power of two indeces in the bitset we calculate the C formula,
// which is:
//       N
//       ___
// C  =  ╲    M
//  x    ╱     α
//       ‾‾‾
//      i = 3
//
// And M  is every M locatted in any index that have the C bit set
//      α
//
// Example: C8 = 1000;
// it will get the following Ms:
//     ┌──────┬───────┬───────┬───────┬───────┐
//     │Mx    │ M5    │ M7    │ M7    │ M8    │
//     ├──────┼───────┼───────┼───────┼───────┤
//     │Binary│ (1)001│ (1)010│ (1)011│ (1)100│
//     ├──────┼───────┼───────┼───────┼───────┤
//     │Index │ 9     │ 10    │ 11    │ 12    │
//     └──────┴───────┴───────┴───────┴───────┘
boost::dynamic_bitset<> hamming::encode(const boost::dynamic_bitset<> &input) {
  if (input.none())
    return input;

  const uint16_t N = utils::hamming::findMinK(input.size()) + input.size() + 1;
  auto bits = hamming::createHammingWord(input, N);

  for (int i = 0; i < N; i++) {
    if (!utils::bit::isPowerOfTwo(i))
      continue;

    // Find every M that has the `position` bit set
    for (int j = 0; j < N; j++)
      if (i != j && i & j && bits.test(j))
        bits.flip(i);

    // fmt::print("===========================\n");
    // fmt::print("C{:>2} {}\n", i, bits);
    // fmt::print("===========================\n");
  }

  // Calculate the G, which is bitwise if the whole word
  bits[0] = bits.count() % 2;

  return bits;
}

Maybe<boost::dynamic_bitset<>> hamming::decode(boost::dynamic_bitset<> input,
                                               const uint8_t &bufferSize) {
  if (input.none())
    return input;

  const uint8_t K = utils::hamming::findMinK(bufferSize);
  const uint16_t N = K + bufferSize + 1;

  auto originalC = utils::hamming::extractC(input, bufferSize);
  boost::dynamic_bitset<> primeC(K, 0);

  uint8_t currentC = 0;
  for (int i = 0; i < N; i++) {
    if (!utils::bit::isPowerOfTwo(i))
      continue;

    for (int j = 0; j < N; j++)
      if (i != j && i & j && input.test(j))
        primeC.flip(currentC);

    currentC++;
  }

  // fmt::print("C's: {:04b} {:04b}\n", originalC.to_ulong(),
  // primeC.to_ulong());
  auto indexToReplace = primeC ^ originalC;

  // fmt::print("Trying access index: {}\n", indexToReplace.to_ulong());

  if (indexToReplace.to_ulong() > input.size())
    return Maybe<boost::dynamic_bitset<>>::error(
        "The word is corrupted and cannot be recovered. (Trying to access "
        "index greater than input)");

  if (indexToReplace.to_ulong() > 0 && indexToReplace.to_ulong() < input.size())
    input.flip(indexToReplace.to_ulong());

  uint8_t primeG = input.count() % 2;
  // Remove coutability of G
  if (input.test(0))
    primeG = !primeG;

  if (primeG != input[0])
    return Maybe<boost::dynamic_bitset<>>::error(
        "The word is corrupted and cannot be recovered. (G did not match)");

  auto recoveredWord = utils::hamming::extractM(input, bufferSize);

  return Maybe<boost::dynamic_bitset<>>::some(recoveredWord);
}
