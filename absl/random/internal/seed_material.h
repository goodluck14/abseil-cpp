// Copyright 2017 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef ABSL_RANDOM_INTERNAL_SEED_MATERIAL_H_
#define ABSL_RANDOM_INTERNAL_SEED_MATERIAL_H_

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

#include "absl/base/attributes.h"
#include "absl/random/internal/fast_uniform_bits.h"
#include "absl/types/optional.h"
#include "absl/types/span.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace random_internal {

// Returns the number of 32-bit blocks needed to contain the given number of
// bits.
constexpr size_t SeedBitsToBlocks(size_t seed_size) {
  return (seed_size + 31) / 32;
}

// Amount of entropy (measured in bits) used to instantiate a Seed Sequence,
// with which to create a URBG.
constexpr size_t kEntropyBitsNeeded = 256;

// Amount of entropy (measured in 32-bit blocks) used to instantiate a Seed
// Sequence, with which to create a URBG.
constexpr size_t kEntropyBlocksNeeded =
    random_internal::SeedBitsToBlocks(kEntropyBitsNeeded);

static_assert(kEntropyBlocksNeeded > 0,
              "Entropy used to seed URBGs must be nonzero.");

// Attempts to fill a span of uint32_t-values using an OS-provided source of
// true entropy (eg. /dev/urandom) into an array of uint32_t blocks of data. The
// resulting array may be used to initialize an instance of a class conforming
// to the C++ Standard "Seed Sequence" concept [rand.req.seedseq].
//
// If values.data() == nullptr, the behavior is undefined.
[[nodiscard]]
bool ReadSeedMaterialFromOSEntropy(absl::Span<uint32_t> values);

// Attempts to fill a span of uint32_t-values using variates generated by an
// existing instance of a class conforming to the C++ Standard "Uniform Random
// Bit Generator" concept [rand.req.urng]. The resulting data may be used to
// initialize an instance of a class conforming to the C++ Standard
// "Seed Sequence" concept [rand.req.seedseq].
//
// If urbg == nullptr or values.data() == nullptr, the behavior is undefined.
template <typename URBG>
[[nodiscard]] bool ReadSeedMaterialFromURBG(URBG* urbg,
                                            absl::Span<uint32_t> values) {
  random_internal::FastUniformBits<uint32_t> distr;

  assert(urbg != nullptr && values.data() != nullptr);
  if (urbg == nullptr || values.data() == nullptr) {
    return false;
  }

  for (uint32_t& seed_value : values) {
    seed_value = distr(*urbg);
  }
  return true;
}

// Mixes given sequence of values with into given sequence of seed material.
// Time complexity of this function is O(sequence.size() *
// seed_material.size()).
//
// Algorithm is based on code available at
// https://gist.github.com/imneme/540829265469e673d045
// by Melissa O'Neill.
void MixIntoSeedMaterial(absl::Span<const uint32_t> sequence,
                         absl::Span<uint32_t> seed_material);

// Returns salt value.
//
// Salt is obtained only once and stored in static variable.
//
// May return empty value if optaining the salt was not possible.
absl::optional<uint32_t> GetSaltMaterial();

}  // namespace random_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_RANDOM_INTERNAL_SEED_MATERIAL_H_
