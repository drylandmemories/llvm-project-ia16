//===--- IA16.h - Declare IA-16 target feature support ----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_BASIC_TARGETS_IA16_H
#define LLVM_CLANG_LIB_BASIC_TARGETS_IA16_H

#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "llvm/Support/Compiler.h"
#include "llvm/TargetParser/IA16TargetParser.h"

namespace clang {
namespace targets {

static const unsigned IA16HugeAddrSpaceMap[] = {
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

class LLVM_LIBRARY_VISIBILITY IA16TargetInfo final : public TargetInfo {
  llvm::IA16::CPUKind CPU = llvm::IA16::CPUKind::I8086;
  bool ProtectedMode = false;

public:
  IA16TargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts);

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

  llvm::SmallVector<Builtin::InfosShard> getTargetBuiltins() const override {
    return {};
  }

  bool hasFeature(StringRef Feature) const override;
  bool isValidCPUName(StringRef Name) const override;
  void fillValidCPUList(SmallVectorImpl<StringRef> &Values) const override;
  bool setCPU(const std::string &Name) override;
  bool handleTargetFeatures(std::vector<std::string> &Features,
                            DiagnosticsEngine &Diags) override;
  bool supportsTargetAttributeTune() const override { return true; }

  ArrayRef<const char *> getGCCRegNames() const override;
  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override {
    return {};
  }

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &Info) const override;
  std::string_view getClobbers() const override { return ""; }

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::CharPtrBuiltinVaList;
  }

  IntType getIntTypeByWidth(unsigned BitWidth, bool IsSigned) const override;
  IntType getLeastIntTypeByWidth(unsigned BitWidth,
                                bool IsSigned) const override;

protected:
  uint64_t getPointerWidthV(LangAS AddrSpace) const override;
  uint64_t getPointerAlignV(LangAS AddrSpace) const override;
  IntType getPtrDiffTypeV(LangAS AddrSpace) const override;
};

} // namespace targets
} // namespace clang

#endif // LLVM_CLANG_LIB_BASIC_TARGETS_IA16_H
