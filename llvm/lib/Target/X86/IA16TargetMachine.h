//===-- IA16TargetMachine.h - Define the IA-16 target machine --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_X86_IA16TARGETMACHINE_H
#define LLVM_LIB_TARGET_X86_IA16TARGETMACHINE_H

#include "IA16Subtarget.h"
#include "llvm/CodeGen/CodeGenTargetMachineImpl.h"
#include <memory>
#include <optional>

namespace llvm {

class FunctionPass;

class IA16TargetMachine final : public CodeGenTargetMachineImpl {
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  IA16Subtarget Subtarget;

public:
  IA16TargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                    StringRef FS, const TargetOptions &Options,
                    std::optional<Reloc::Model> RM,
                    std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                    bool JIT);
  ~IA16TargetMachine() override;

  const IA16Subtarget *getSubtargetImpl(const Function &) const override {
    return &Subtarget;
  }

  MachineFunctionInfo *
  createMachineFunctionInfo(BumpPtrAllocator &Allocator, const Function &F,
                            const TargetSubtargetInfo *STI) const override;

  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }
};

FunctionPass *createIA16ISelDag(IA16TargetMachine &TM,
                                CodeGenOptLevel OptLevel);

} // end namespace llvm

#endif // LLVM_LIB_TARGET_X86_IA16TARGETMACHINE_H
