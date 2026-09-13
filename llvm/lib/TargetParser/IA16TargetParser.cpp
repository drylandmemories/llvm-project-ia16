//===-- IA16TargetParser - Parser for IA-16 CPUs -------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/TargetParser/IA16TargetParser.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

IA16::CPUKind IA16::parseCPU(StringRef CPU) {
  return StringSwitch<CPUKind>(CPU)
      .Case("i8086", CPUKind::I8086)
      .Case("i8088", CPUKind::I8088)
      .Cases({"i80186", "i186"}, CPUKind::I80186)
      .Case("i80188", CPUKind::I80188)
      .Cases({"i80286", "i286"}, CPUKind::I80286)
      .Default(CPUKind::Invalid);
}

StringRef IA16::getCanonicalCPUName(CPUKind CPU) {
  switch (CPU) {
  case CPUKind::Invalid:
    return {};
  case CPUKind::I8086:
    return "i8086";
  case CPUKind::I8088:
    return "i8088";
  case CPUKind::I80186:
    return "i80186";
  case CPUKind::I80188:
    return "i80188";
  case CPUKind::I80286:
    return "i80286";
  }
  llvm_unreachable("invalid IA-16 CPU");
}

StringRef IA16::getDefaultCPU() { return "i8086"; }

uint32_t IA16::getCPUFeatures(CPUKind CPU) {
  constexpr uint32_t Features186 = FeatureShiftImmediate |
                                   FeaturePushImmediate | FeatureIMulImmediate |
                                   FeaturePUSHA | FeatureEnterLeave;

  switch (CPU) {
  case CPUKind::Invalid:
  case CPUKind::I8086:
  case CPUKind::I8088:
    return FeatureNone;
  case CPUKind::I80186:
  case CPUKind::I80188:
    return Features186;
  case CPUKind::I80286:
    return Features186 | FeatureProtectedMode;
  }
  llvm_unreachable("invalid IA-16 CPU");
}

void IA16::fillValidCPUList(SmallVectorImpl<StringRef> &Values) {
  Values.append(
      {"i8086", "i8088", "i80186", "i80188", "i80286", "i186", "i286"});
}
