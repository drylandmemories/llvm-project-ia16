//===----------- IA16TargetParserTest.cpp - IA-16 parser tests -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/TargetParser/IA16TargetParser.h"
#include "llvm/ADT/SmallVector.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace llvm;
using namespace llvm::IA16;
using ::testing::ElementsAre;

namespace {

TEST(IA16TargetParserTest, CPUParsingAndAliases) {
  EXPECT_EQ(CPUKind::I8086, parseCPU("i8086"));
  EXPECT_EQ(CPUKind::I8088, parseCPU("i8088"));
  EXPECT_EQ(CPUKind::I80186, parseCPU("i80186"));
  EXPECT_EQ(CPUKind::I80186, parseCPU("i186"));
  EXPECT_EQ(CPUKind::I80188, parseCPU("i80188"));
  EXPECT_EQ(CPUKind::I80286, parseCPU("i80286"));
  EXPECT_EQ(CPUKind::I80286, parseCPU("i286"));
  EXPECT_EQ(CPUKind::Invalid, parseCPU("i386"));
  EXPECT_EQ(CPUKind::Invalid, parseCPU("any"));
  EXPECT_EQ("i8086", getDefaultCPU());
  EXPECT_EQ("i80186", getCanonicalCPUName(parseCPU("i186")));
  EXPECT_EQ("i80286", getCanonicalCPUName(parseCPU("i286")));
}

TEST(IA16TargetParserTest, GenerationFeatures) {
  EXPECT_EQ(FeatureNone, getCPUFeatures(CPUKind::I8086));
  EXPECT_EQ(FeatureNone, getCPUFeatures(CPUKind::I8088));

  uint32_t Features186 = getCPUFeatures(CPUKind::I80186);
  EXPECT_NE(0U, Features186 & FeatureShiftImmediate);
  EXPECT_NE(0U, Features186 & FeaturePushImmediate);
  EXPECT_NE(0U, Features186 & FeatureIMulImmediate);
  EXPECT_NE(0U, Features186 & FeaturePUSHA);
  EXPECT_NE(0U, Features186 & FeatureEnterLeave);
  EXPECT_EQ(0U, Features186 & FeatureProtectedMode);

  uint32_t Features286 = getCPUFeatures(CPUKind::I80286);
  EXPECT_EQ(Features186, Features286 & ~FeatureProtectedMode);
  EXPECT_NE(0U, Features286 & FeatureProtectedMode);
}

TEST(IA16TargetParserTest, ValidCPUList) {
  SmallVector<StringRef, 8> Values;
  fillValidCPUList(Values);
  EXPECT_THAT(Values, ElementsAre("i8086", "i8088", "i80186", "i80188",
                                  "i80286", "i186", "i286"));
}

} // namespace
