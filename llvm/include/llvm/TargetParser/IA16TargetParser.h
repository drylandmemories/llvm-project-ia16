//===-- IA16TargetParser - Parser for IA-16 CPUs ---------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TARGETPARSER_IA16TARGETPARSER_H
#define LLVM_TARGETPARSER_IA16TARGETPARSER_H

#include "llvm/Support/Compiler.h"
#include <cstdint>

namespace llvm {
template <typename T> class SmallVectorImpl;
class StringRef;

namespace IA16 {

enum class CPUKind : uint8_t {
  Invalid,
  I8086,
  I8088,
  I80186,
  I80188,
  I80286,
};

enum FeatureKind : uint32_t {
  FeatureNone = 0,
  FeatureShiftImmediate = 1U << 0,
  FeaturePushImmediate = 1U << 1,
  FeatureIMulImmediate = 1U << 2,
  FeaturePUSHA = 1U << 3,
  FeatureEnterLeave = 1U << 4,
  FeatureProtectedMode = 1U << 5,
};

/// Parse an -march or -mtune spelling. The i186 and i286 aliases are accepted.
LLVM_ABI CPUKind parseCPU(StringRef CPU);

/// Return the canonical spelling for a CPU, or an empty string if invalid.
LLVM_ABI StringRef getCanonicalCPUName(CPUKind CPU);

/// IA-16 defaults to the oldest supported processor.
LLVM_ABI StringRef getDefaultCPU();

/// Return the instruction-generation feature mask for a CPU.
LLVM_ABI uint32_t getCPUFeatures(CPUKind CPU);

/// Add all accepted CPU spellings, including aliases, to Values.
LLVM_ABI void fillValidCPUList(SmallVectorImpl<StringRef> &Values);

} // namespace IA16
} // namespace llvm

#endif // LLVM_TARGETPARSER_IA16TARGETPARSER_H
