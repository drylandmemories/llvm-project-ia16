//===-- IA16MachineFunctionInfo.h - IA-16 machine function info -*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_X86_IA16MACHINEFUNCTIONINFO_H
#define LLVM_LIB_TARGET_X86_IA16MACHINEFUNCTIONINFO_H

#include "llvm/CodeGen/MachineFunction.h"

namespace llvm {

class IA16MachineFunctionInfo final : public MachineFunctionInfo {
  int VarArgsFrameIndex = 0;
  Register SRetReturnReg = 0;

public:
  IA16MachineFunctionInfo(const Function &, const TargetSubtargetInfo *) {}

  MachineFunctionInfo *
  clone(BumpPtrAllocator &, MachineFunction &DestMF,
        const DenseMap<MachineBasicBlock *, MachineBasicBlock *> &)
      const override {
    return DestMF.cloneInfo<IA16MachineFunctionInfo>(*this);
  }

  int getVarArgsFrameIndex() const { return VarArgsFrameIndex; }
  void setVarArgsFrameIndex(int Index) { VarArgsFrameIndex = Index; }
  Register getSRetReturnReg() const { return SRetReturnReg; }
  void setSRetReturnReg(Register Reg) { SRetReturnReg = Reg; }
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_X86_IA16MACHINEFUNCTIONINFO_H
