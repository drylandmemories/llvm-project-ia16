//===--- IA16.cpp - IA-16 DOS toolchain ---------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "IA16.h"
#include "clang/Driver/CommonArgs.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/InputInfo.h"
#include "clang/Driver/Job.h"
#include "clang/Options/Options.h"
#include "llvm/Support/Path.h"

using namespace clang;
using namespace clang::driver;
using namespace clang::driver::tools;
using namespace llvm::opt;

void ia16::Linker::ConstructJob(Compilation &C, const JobAction &JA,
                                const InputInfo &Output,
                                const InputInfoList &Inputs,
                                const ArgList &Args,
                                const char *LinkingOutput) const {
  const Driver &D = getToolChain().getDriver();
  StringRef Model = Args.getLastArgValue(options::OPT_mcmodel_EQ, "small");
  bool IsCOM = Model == "tiny";

  std::string LinkedPath;
  if (D.isSaveTempsEnabled()) {
    LinkedPath = llvm::sys::path::stem(Output.getFilename()).str();
    LinkedPath += ".ia16.elf";
  } else {
    LinkedPath = D.GetTemporaryPath("ia16-link", "elf");
  }
  const char *LinkedELF = Args.MakeArgString(LinkedPath);
  if (!D.isSaveTempsEnabled())
    C.addTempFile(LinkedELF);

  ArgStringList LinkArgs;
  LinkArgs.push_back("-m");
  LinkArgs.push_back("elf_i386");
  LinkArgs.push_back("--image-base=0");
  LinkArgs.push_back("--emit-relocs");
  LinkArgs.push_back("--gc-sections");
  LinkArgs.push_back("-N");
  LinkArgs.push_back(IsCOM ? "-Ttext=0x100" : "-Ttext=0");
  LinkArgs.push_back("--entry=_start");
  LinkArgs.push_back("-o");
  LinkArgs.push_back(LinkedELF);

  if (!D.SysRoot.empty())
    LinkArgs.push_back(Args.MakeArgString("--sysroot=" + D.SysRoot));

  bool NeedStartFiles =
      !Args.hasArg(options::OPT_nostdlib, options::OPT_nostartfiles);
  if (NeedStartFiles)
    LinkArgs.push_back(Args.MakeArgString(getToolChain().GetFilePath("crt0.o")));

  AddLinkerInputs(getToolChain(), Inputs, Args, LinkArgs, JA);

  if (!Args.hasArg(options::OPT_nostdlib, options::OPT_nodefaultlibs)) {
    LinkArgs.push_back("-lc");
    LinkArgs.push_back("-lclang_rt.builtins-ia16");
  }

  for (const Arg *A : Args.filtered(options::OPT_Wl_COMMA,
                                    options::OPT_Xlinker)) {
    A->claim();
    for (StringRef Value : A->getValues())
      LinkArgs.push_back(Args.MakeArgString(Value));
  }

  const char *Linker =
      Args.MakeArgString(getToolChain().GetProgramPath("ld.lld"));
  InputInfo LinkedOutput(&JA, LinkedELF, LinkedELF);
  C.addCommand(std::make_unique<Command>(
      JA, *this, ResponseFileSupport::AtFileCurCP(), Linker, LinkArgs, Inputs,
      LinkedOutput));

  ArgStringList PackArgs;
  PackArgs.push_back(IsCOM ? "--format=com" : "--format=mz");
  PackArgs.push_back("-o");
  PackArgs.push_back(Output.getFilename());
  PackArgs.push_back(LinkedELF);
  const char *Packer =
      Args.MakeArgString(getToolChain().GetProgramPath("llvm-ia16-mz"));
  InputInfoList PackInputs{LinkedOutput};
  C.addCommand(std::make_unique<Command>(
      JA, *this, ResponseFileSupport::None(), Packer, PackArgs, PackInputs,
      Output));
}

Tool *toolchains::IA16ToolChain::buildLinker() const {
  return new tools::ia16::Linker(*this);
}

void toolchains::IA16ToolChain::addClangTargetOptions(
    const ArgList &DriverArgs, ArgStringList &CC1Args,
    Action::OffloadKind DeviceOffloadKind) const {
  Generic_ELF::addClangTargetOptions(DriverArgs, CC1Args, DeviceOffloadKind);
  if (!DriverArgs.hasArg(options::OPT_funwind_tables,
                         options::OPT_fasynchronous_unwind_tables))
    CC1Args.push_back("-funwind-tables=0");
}

void toolchains::IA16ToolChain::AddClangSystemIncludeArgs(
    const ArgList &DriverArgs, ArgStringList &CC1Args) const {
  if (DriverArgs.hasArg(options::OPT_nostdinc))
    return;
  if (!getDriver().SysRoot.empty())
    addSystemInclude(DriverArgs, CC1Args,
                     getDriver().SysRoot + "/include");
}

void toolchains::IA16ToolChain::AddClangCXXStdlibIncludeArgs(
    const ArgList &, ArgStringList &) const {
  // C++ ABI support is intentionally deferred for the first IA-16 release.
}
