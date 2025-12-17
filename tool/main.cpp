#include <toke/cxx_api.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>

#include <cstdlib>

#include "file_set.h"
#include "initializer.h"
#include "options.h"
#include "pair_counter.h"
#include "vocab.h"

namespace {

namespace fs = std::filesystem;

using Path = fs::path;

} // namespace

auto
main(const int argc, char** argv) -> int
{
  for (int i = 1; i < argc; i++) {
    std::cout << "arg[" << i << "]: " << argv[i] << std::endl;
  }

  Options options;

  if (!options.parse(argc, argv)) {
    return EXIT_FAILURE;
  }

  std::cout << "initializing vocab" << std::endl;

  auto fileSet = FileSet::create(options.directoryPath, /*cache=*/true, options.filterConfig);

  Vocab vocab(options.filterConfig);

  auto initializer = Initializer::create(&vocab);

  fileSet->accept(*initializer);

  if (vocab.size() == 0) {
    std::cerr << "vocab is empty. exiting" << std::endl;
    return EXIT_FAILURE;
  }

  if (vocab.size() >= options.numTokens) {
    std::cerr << "base vocab is already " << vocab.size() << ", no pairs will be merged." << std::endl;
    std::ofstream file(options.outputPath, std::ios::binary | std::ios::out);
    file << vocab.toString();
    return EXIT_SUCCESS;
  }

  auto pairCounter = PairCounter::create(&vocab);

  std::cout << "scanning text..." << std::endl;

  while (vocab.size() < options.numTokens) {

    const auto t0 = std::chrono::high_resolution_clock::now();

    fileSet->accept(*pairCounter);

    const auto result = pairCounter->getHighestFrequencyPair();
    if (result.count <= 1) {
      break;
    }

    auto newToken = TokenDef::concat(vocab.at(result.left), vocab.at(result.right));

    vocab.define(std::move(newToken));

    const auto remaining = options.numTokens - vocab.size();

    const auto t1 = std::chrono::high_resolution_clock::now();

    const auto dt =
      static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()) * 1.0e-3F;

    const auto remainingTime = dt * static_cast<float>(remaining);

    std::cout << "scanning text...";
    std::cout << " | ";
    std::cout << "vocab size is " << vocab.size() << " of " << options.numTokens;
    std::cout << " | ";
    std::cout << "tokens_per_sec: " << dt << " [sec]";
    std::cout << " | ";
    std::cout << "remaining time: " << (remainingTime * (1.0F / 60.0F)) << " [min]";
    std::cout << std::endl;
  }

  return EXIT_SUCCESS;
}
