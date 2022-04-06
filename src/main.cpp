#include <common.hpp>

#include <boost/dynamic_bitset/dynamic_bitset.hpp>
#include <cxxopts.hpp>

#include <hamming.hpp>
#include <utils.hpp>

auto hammify(const std::string &inputFilename,
             const std::string &outputFilename, const uint16_t bufferSize)
    -> void {
  std::ifstream inputFile;
  inputFile.open(inputFilename, std::ios::in | std::ios::binary);
  if (inputFile.fail()) {
    fmt::print("Failed to open {}\n", inputFilename);
    inputFile.clear();
    exit(0);
  }

  std::ofstream outputFile;
  outputFile.open(outputFilename, std::ios::out | std::ios::binary);
  if (outputFile.fail()) {
    fmt::print("Failed to create/open output: {}\n", outputFilename);
    outputFile.clear();
    exit(0);
  }

  uint8_t byteBufferSize = utils::bit::bit2Byte(bufferSize);
  auto hammingWordSize = [](const uint8_t &wordSize) -> uint8_t {
    uint8_t k = utils::hamming::findMinK(wordSize);
    return k + wordSize + 1; // K + N + G
  };
  uint8_t hammingWordBytes =
      utils::bit::findMinBytes(hammingWordSize(bufferSize));

  while (!inputFile.eof()) {
    // Declare bytes buffer
    std::vector<uint8_t> dataBuffer;
    dataBuffer.resize(byteBufferSize);

    // Read the byte
    inputFile.read(reinterpret_cast<char *>(&dataBuffer[0]), byteBufferSize);

    boost::dynamic_bitset<> currentNBytes(bufferSize, 0);
    std::for_each(dataBuffer.begin(), dataBuffer.end(),
                  [&currentNBytes](uint8_t byte) {
                    currentNBytes = utils::bit::concat(
                        currentNBytes, boost::dynamic_bitset<>(8, byte));
                  });

    auto byteHammified = hamming::encode(currentNBytes);

    byteHammified.resize(hammingWordBytes * 8);

    fmt::print("{} {:024b}\n", byteHammified.size(), byteHammified.to_ulong());

    auto ulong = (byteHammified).to_ulong();

    if (ulong != 0) {
      outputFile.write(reinterpret_cast<char *>(&ulong), hammingWordBytes);
    }
  }

  // Print vector content
  // for (auto &byte : dataBuffer) {
  //   fmt::print("{:08b} ", byte);
  //   fmt::print("{:02x} ", byte);
  //   fmt::print("====\n");
  // }

  inputFile.close();
  outputFile.close();
}

auto dehammify(const std::string &inputFilename, const uint16_t bufferSize)
    -> void {
  fmt::print("Dehammifying {}\n", inputFilename);
  std::ifstream inputFile;
  inputFile.open(inputFilename, std::ios::in | std::ios::binary);
  if (inputFile.fail()) {
    std::cout << "Failed to open " << inputFilename << std::endl;
    inputFile.clear();
    exit(0);
  }

  std::ofstream outputFile;
  outputFile.open("new-test.txt", std::ios::out | std::ios::binary);
  if (outputFile.fail()) {
    std::cout << "Failed to open "
              << "new-test.txt" << std::endl;
    outputFile.clear();
    exit(0);
  }

  const auto K = utils::hamming::findMinK(bufferSize);
  const auto hammingWordSize = K + bufferSize + 1;

  uint8_t byteBufferSize = utils::bit::bit2Byte(bufferSize);
  // Declare bytes buffer
  // std::vector<uint8_t> dataBuffer;
  // dataBuffer.resize(byteBufferSize);

  // Read the byte
  std::vector<uint8_t> dataBuffer((std::istreambuf_iterator<char>(inputFile)),
                                  (std::istreambuf_iterator<char>()));

  auto minByte = utils::bit::findMinBytes(hammingWordSize);

  boost::dynamic_bitset<> hammingWord(hammingWordSize, 0);
  uint8_t bitCount = 1;
  for (auto &byte : dataBuffer) {
    fmt::print("{:02x} ", byte);
    fmt::print("==============\n");
    hammingWord =
        utils::bit::concat(hammingWord, boost::dynamic_bitset<>(8, byte));

    if (bitCount == minByte) {
      hammingWord.resize(hammingWordSize);
      fmt::print("{} {:024b}\n", hammingWord.size(), hammingWord.to_ulong());
      auto dehammified = hamming::decode(hammingWord, bufferSize);

      if (dehammified.is_error())
        throw std::runtime_error(dehammified.error()); // Probably just print

      auto ulong = dehammified.some().to_ulong();

      if (ulong != 0) {
        outputFile.write(reinterpret_cast<char *>(&ulong), byteBufferSize);
      }

      hammingWord.clear();
      bitCount = 0;
    }

    bitCount++;
  }

  // boost::dynamic_bitset<> currentNBytes(hammingWordSize, 0);
  // uint8_t index = 0;
  // std::for_each(dataBuffer.rbegin(), dataBuffer.rend(),
  //               [&currentNBytes, &index](uint8_t byte) {
  //                 fmt::print("[{}] - {:08b}\n", index++, byte);
  //                 currentNBytes = utils::bit::concat(
  //                     currentNBytes, boost::dynamic_bitset<>(8, byte));
  //               });
  // fmt::print("\n{:022b}\n", currentNBytes.to_ulong());
  //
  // fmt::print("{} {}\n", hammingWordSize, currentNBytes.size());

  // Maybe<boost::dynamic_bitset<>> recoveredBits =
  //     hamming::decode(currentNBytes, bufferSize);

  // if (recoveredBits.is_error())
  //   throw std::runtime_error(recoveredBits.error());
  //
  // fmt::print("{:08b} ", recoveredBits.some().to_ulong());

  inputFile.close();
  outputFile.close();
}

auto main(int argc, char *argv[]) -> int {
  cxxopts::Options options("hamming",
                           "Hamming code implementation for the OAC class.");

  // clang-format off
  options.add_options()
    ("h,help", "Print usage")
    ("i,input", "Input file", cxxopts::value<std::string>())
    ("o,output", "Output file", cxxopts::value<std::string>()->default_value("output.wham"))
    ("d,decode", "Decode file in order to verify it's integrity", cxxopts::value<bool>()->implicit_value("true"))
    ("b,buffer", "Size of buffer chunk for the hamming code calculation",
    cxxopts::value<uint16_t>()->default_value("8"))
    ("v,verbose", "Verbose mode");
  // clang-format on

  auto result = options.parse(argc, argv);
  std::string filename;

  if (result.count("help")) {
    fmt::print("{}\n", options.help());
    exit(0);
  }

  result.count("input") ? (filename = result["input"].as<std::string>())
                        : throw std::runtime_error("No input file specified");

  const std::string output = result["output"].as<std::string>();
  const uint16_t bufferSize = result["buffer"].as<uint16_t>();
  const bool decode = result["decode"].as<bool>();

  !decode ? hammify(filename, output, bufferSize)
          : dehammify(filename, bufferSize);

  return 0;
}
