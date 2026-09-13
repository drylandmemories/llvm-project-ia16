//===--- IA16.h - IA-16 DOS toolchain -------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_IA16_H
#define LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_IA16_H

#include "Gnu.h"

namespace clang::driver {

namespace tools::ia16 {
class LLVM_LIBRARY_VISIBILITY Linker final : public Tool {
public:
  explicit Linker(const ToolChain &TC) : Tool("IA16::Linker", "linker", TC) {}

  bool hasIntegratedCPP() const override { return false; }
  bool isLinkJob() const override { return true; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const llvm::opt::ArgList &Args,
                    const char *LinkingOutput) const override;
};
} // namespace tools::ia16

namespace toolchains {
class LLVM_LIBRARY_VISIBILITY IA16ToolChain final : public Generic_ELF {
public:
  IA16ToolChain(const Driver &D, const llvm::Triple &Triple,
                const llvm::opt::ArgList &Args)
      : Generic_ELF(D, Triple, Args) {}

  Tool *buildLinker() const override;
  const char *getDefaultLinker() const override { return "ld.lld"; }
  RuntimeLibType GetDefaultRuntimeLibType() const override {
    return ToolChain::RLT_CompilerRT;
  }
  CXXStdlibType GetDefaultCXXStdlibType() const override {
    return ToolChain::CST_Libcxx;
  }
  bool IsMathErrnoDefault() const override { return false; }

  void addClangTargetOptions(const llvm::opt::ArgList &DriverArgs,
                             llvm::opt::ArgStringList &CC1Args,
                             Action::OffloadKind DeviceOffloadKind) const override;
  void AddClangSystemIncludeArgs(
      const llvm::opt::ArgList &DriverArgs,
      llvm::opt::ArgStringList &CC1Args) const override;
  void AddClangCXXStdlibIncludeArgs(
      const llvm::opt::ArgList &DriverArgs,
      llvm::opt::ArgStringList &CC1Args) const override;
};
} // namespace toolchains
} // namespace clang::driver

#endif // LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_IA16_H
