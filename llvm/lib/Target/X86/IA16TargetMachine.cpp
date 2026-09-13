//===-- IA16TargetMachine.cpp - IA-16 target machine ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "IA16TargetMachine.h"
#include "TargetInfo/X86TargetInfo.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

namespace llvm {
void registerIA16TargetMachine() {
  static RegisterTargetMachine<IA16TargetMachine> IA16(getTheIA16Target());
}
} // end namespace llvm

static Reloc::Model getIA16RelocModel(std::optional<Reloc::Model> RM) {
  return RM.value_or(Reloc::Static);
}

static CodeModel::Model
getIA16CodeModel(std::optional<CodeModel::Model> CM) {
  // Clang maps the six segmented memory models onto LLVM's existing code-model
  // enum. Unlike the generic helper, IA-16 intentionally accepts Tiny.
  return CM.value_or(CodeModel::Small);
}

IA16TargetMachine::IA16TargetMachine(
    const Target &T, const Triple &TT, StringRef CPU, StringRef FS,
    const TargetOptions &Options, std::optional<Reloc::Model> RM,
    std::optional<CodeModel::Model> CM, CodeGenOptLevel OL, bool JIT)
    : CodeGenTargetMachineImpl(
          T, TT.computeDataLayout(), TT, CPU.empty() ? "i8086" : CPU, FS,
          Options, getIA16RelocModel(RM), getIA16CodeModel(CM), OL),
      TLOF(std::make_unique<TargetLoweringObjectFileELF>()),
      Subtarget(TT, CPU, /*TuneCPU=*/CPU, FS, *this) {
  if (JIT)
    report_fatal_error("JIT compilation is not supported for IA-16");
  initAsmInfo();
}

IA16TargetMachine::~IA16TargetMachine() = default;

namespace {
class IA16PassConfig final : public TargetPassConfig {
public:
  IA16PassConfig(IA16TargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {}

  IA16TargetMachine &getIA16TargetMachine() const {
    return getTM<IA16TargetMachine>();
  }

  bool addInstSelector() override {
    addPass(createIA16ISelDag(getIA16TargetMachine(), getOptLevel()));
    return false;
  }
};
} // end anonymous namespace

TargetPassConfig *IA16TargetMachine::createPassConfig(PassManagerBase &PM) {
  return new IA16PassConfig(*this, PM);
}
