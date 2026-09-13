//===-- IA16FrameLowering.h - IA-16 frame lowering -------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_X86_IA16FRAMELOWERING_H
#define LLVM_LIB_TARGET_X86_IA16FRAMELOWERING_H

#include "llvm/CodeGen/TargetFrameLowering.h"

namespace llvm {

class IA16FrameLowering final : public TargetFrameLowering {
protected:
  bool hasFPImpl(const MachineFunction &MF) const override;

public:
  IA16FrameLowering()
      : TargetFrameLowering(StackGrowsDown, Align(2), 0, Align(2)) {}

  void emitPrologue(MachineFunction &MF,
                    MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF,
                    MachineBasicBlock &MBB) const override;
  MachineBasicBlock::iterator eliminateCallFramePseudoInstr(
      MachineFunction &MF, MachineBasicBlock &MBB,
      MachineBasicBlock::iterator I) const override;
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_X86_IA16FRAMELOWERING_H
