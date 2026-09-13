//===--- IA16.cpp - Implement IA-16 target feature support ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "IA16.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/MacroBuilder.h"
#include "llvm/ADT/StringSwitch.h"

using namespace clang;
using namespace clang::targets;

IA16TargetInfo::IA16TargetInfo(const llvm::Triple &Triple,
                               const TargetOptions &Opts)
    : TargetInfo(Triple) {
  bool IsHugeModel = Opts.CodeModel == "huge";
  if (IsHugeModel)
    AddrSpaceMap = &IA16HugeAddrSpaceMap;
  TLSSupported = false;
  HasFloat128 = false;
  HasLongDouble = true;
  HasUnalignedAccess = true;

  BoolWidth = BoolAlign = 8;
  ShortWidth = ShortAlign = 16;
  IntWidth = IntAlign = 16;
  LongWidth = 32;
  LongAlign = 16;
  LongLongWidth = 64;
  LongLongAlign = 16;
  FloatWidth = 32;
  FloatAlign = 16;
  DoubleWidth = LongDoubleWidth = 64;
  DoubleAlign = LongDoubleAlign = 16;
  DoubleFormat = LongDoubleFormat = &llvm::APFloat::IEEEdouble();

  PointerWidth = IsHugeModel ? 32 : 16;
  PointerAlign = 16;
  SuitableAlign = 16;
  DefaultAlignForAttributeAligned = 16;
  MaxVectorAlign = 16;

  SizeType = IsHugeModel ? UnsignedLong : UnsignedInt;
  PtrDiffType = IsHugeModel ? SignedLong : SignedInt;
  IntPtrType = IsHugeModel ? SignedLong : SignedInt;
  IntMaxType = SignedLongLong;
  Int16Type = SignedInt;
  Int64Type = SignedLongLong;
  WCharType = WIntType = UnsignedInt;
  Char16Type = UnsignedInt;
  Char32Type = UnsignedLong;
  SigAtomicType = SignedInt;

  MaxAtomicPromoteWidth = MaxAtomicInlineWidth = 0;
  resetDataLayout();
}

bool IA16TargetInfo::hasFeature(StringRef Feature) const {
  return Feature == "ia16" || Feature == "i86" ||
         (Feature == "protected-mode" && ProtectedMode);
}

bool IA16TargetInfo::isValidCPUName(StringRef Name) const {
  return llvm::IA16::parseCPU(Name) != llvm::IA16::CPUKind::Invalid;
}

void IA16TargetInfo::fillValidCPUList(
    SmallVectorImpl<StringRef> &Values) const {
  llvm::IA16::fillValidCPUList(Values);
}

bool IA16TargetInfo::setCPU(const std::string &Name) {
  llvm::IA16::CPUKind Parsed = llvm::IA16::parseCPU(Name);
  if (Parsed == llvm::IA16::CPUKind::Invalid)
    return false;
  CPU = Parsed;
  return true;
}

bool IA16TargetInfo::handleTargetFeatures(std::vector<std::string> &Features,
                                          DiagnosticsEngine &Diags) {
  for (StringRef Feature : Features) {
    if (Feature == "+protected-mode")
      ProtectedMode = true;
    else if (Feature == "-protected-mode")
      ProtectedMode = false;
  }
  if (ProtectedMode && CPU != llvm::IA16::CPUKind::I80286) {
    Diags.Report(diag::err_invalid_feature_combination)
        << "IA-16 protected mode requires -march=i80286 or -march=i286";
    return false;
  }
  return true;
}

ArrayRef<const char *> IA16TargetInfo::getGCCRegNames() const {
  static const char *const GCCRegNames[] = {
      "ax", "dx", "cx", "bx", "si", "di", "bp", "sp", "cs", "ds",
      "es", "ss", "al", "ah", "dl", "dh", "cl", "ch", "bl", "bh"};
  return llvm::ArrayRef(GCCRegNames);
}

bool IA16TargetInfo::validateAsmConstraint(
    const char *&Name, TargetInfo::ConstraintInfo &Info) const {
  switch (*Name) {
  case 'a': // AX, AL, or AH.
  case 'b': // BX, BL, or BH.
  case 'c': // CX, CL, or CH.
  case 'd': // DX, DL, or DH.
  case 'S': // SI.
  case 'D': // DI.
  case 'q': // Any byte-addressable general register.
  case 'R': // Any general IA-16 register.
    Info.setAllowsRegister();
    return true;
  case 'A': // DX:AX pair.
    Info.setAllowsRegister();
    return true;
  case 'I': // Constant shift count accepted by 80186 and later.
    Info.setRequiresImmediate(0, 31);
    return true;
  case 'J': // Constant zero.
    Info.setRequiresImmediate(0);
    return true;
  case 'K': // Signed 8-bit constant.
    Info.setRequiresImmediate(-128, 127);
    return true;
  case 'L': // Unsigned 8-bit constant.
    Info.setRequiresImmediate(0, 255);
    return true;
  default:
    return false;
  }
}

TargetInfo::IntType IA16TargetInfo::getIntTypeByWidth(unsigned BitWidth,
                                                      bool IsSigned) const {
  if (BitWidth == 16)
    return IsSigned ? SignedInt : UnsignedInt;
  return TargetInfo::getIntTypeByWidth(BitWidth, IsSigned);
}

TargetInfo::IntType
IA16TargetInfo::getLeastIntTypeByWidth(unsigned BitWidth, bool IsSigned) const {
  if (BitWidth == 16)
    return IsSigned ? SignedInt : UnsignedInt;
  return TargetInfo::getLeastIntTypeByWidth(BitWidth, IsSigned);
}

uint64_t IA16TargetInfo::getPointerWidthV(LangAS AddrSpace) const {
  switch (getTargetAddressSpace(AddrSpace)) {
  case 1: // Far data.
  case 2: // Huge data.
  case 5: // Far code.
    return 32;
  default:
    return 16;
  }
}

uint64_t IA16TargetInfo::getPointerAlignV(LangAS AddrSpace) const { return 16; }

TargetInfo::IntType IA16TargetInfo::getPtrDiffTypeV(LangAS AddrSpace) const {
  return getTargetAddressSpace(AddrSpace) == 2 ? SignedLong : SignedInt;
}

static void defineCPUNameMacro(MacroBuilder &Builder, StringRef Prefix,
                               llvm::IA16::CPUKind CPU) {
  Builder.defineMacro((Prefix + llvm::IA16::getCanonicalCPUName(CPU).upper())
                          .str());
}

void IA16TargetInfo::getTargetDefines(const LangOptions &Opts,
                                      MacroBuilder &Builder) const {
  // gcc-ia16 uses this date-valued macro as its compatibility version.
  Builder.defineMacro("__ia16__", "20240218L");
  Builder.defineMacro("__IA16__");
  Builder.defineMacro("__IA16_ABI_VERSION__", "0x000200");
  Builder.defineMacro("__X86__");
  Builder.defineMacro("__I86__");
  Builder.defineMacro("__i86");
  Builder.defineMacro("_M_I86");

  unsigned MachineLevel = CPU == llvm::IA16::CPUKind::I80286 ? 200 :
                          llvm::IA16::getCPUFeatures(CPU) != 0 ? 100 : 0;
  Builder.defineMacro("_M_IX86", Twine(MachineLevel));
  defineCPUNameMacro(Builder, "__IA16_ARCH_", CPU);

  llvm::IA16::CPUKind TuneCPU = CPU;
  if (!getTargetOpts().TuneCPU.empty())
    TuneCPU = llvm::IA16::parseCPU(getTargetOpts().TuneCPU);
  defineCPUNameMacro(Builder, "__IA16_TUNE_", TuneCPU);

  uint32_t Features = llvm::IA16::getCPUFeatures(CPU);
  if (Features & llvm::IA16::FeatureShiftImmediate)
    Builder.defineMacro("__IA16_FEATURE_SHIFT_IMM");
  if (Features & llvm::IA16::FeaturePushImmediate)
    Builder.defineMacro("__IA16_FEATURE_PUSH_IMM");
  if (Features & llvm::IA16::FeatureIMulImmediate)
    Builder.defineMacro("__IA16_FEATURE_IMUL_IMM");
  if (Features & llvm::IA16::FeaturePUSHA)
    Builder.defineMacro("__IA16_FEATURE_PUSHA");
  if (Features & llvm::IA16::FeatureEnterLeave)
    Builder.defineMacro("__IA16_FEATURE_ENTER_LEAVE");
  if (Features & llvm::IA16::FeatureProtectedMode)
    Builder.defineMacro("__IA16_FEATURE_80286");
  if (ProtectedMode)
    Builder.defineMacro("__IA16_PROTECTED_MODE__");
  else
    Builder.defineMacro("__IA16_REAL_MODE__");

  // The initial frontend exposes the ABI address spaces through Clang's GNU
  // address_space attribute. Dedicated spellings are added by Sema later.
  Builder.defineMacro("__FAR");
  Builder.defineMacro("__SEG_SS");
  Builder.defineMacro("__far", "__attribute__((address_space(1)))");
  Builder.defineMacro("__huge", "__attribute__((address_space(2)))");
  Builder.defineMacro("__seg_ss", "__attribute__((address_space(3)))");
  Builder.defineMacro("__near", "__attribute__((address_space(0)))");

  std::string Model = getTargetOpts().CodeModel;
  if (Model.empty() || Model == "default")
    Model = "small";
  StringRef ModelName = llvm::StringSwitch<StringRef>(Model)
                            .Cases({"tiny", "small", "medium", "compact",
                                    "large", "huge"}, Model)
                            .Default("small");
  std::string UpperModel = ModelName.upper();
  Builder.defineMacro("__IA16_CMODEL_" + UpperModel + "__");
  Builder.defineMacro("__" + UpperModel + "__");
  Builder.defineMacro("__IA16_ABI_SEGELF");

  if (Opts.C11)
    Builder.defineMacro("__STDC_NO_ATOMICS__");
}
