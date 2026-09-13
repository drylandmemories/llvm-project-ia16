//===-- IA16FrameLowering.cpp - IA-16 frame lowering ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "IA16FrameLowering.h"
#include "IA16Subtarget.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetMachine.h"

using namespace llvm;

bool IA16FrameLowering::hasFPImpl(const MachineFunction &MF) const {
  return MF.getTarget().Options.DisableFramePointerElim(MF) ||
         MF.getFrameInfo().hasStackObjects() ||
         MF.getFrameInfo().hasVarSizedObjects() ||
         MF.getFrameInfo().isFrameAddressTaken();
}

void IA16FrameLowering::emitPrologue(MachineFunction &MF,
                                     MachineBasicBlock &MBB) const {
  const auto &TII = *MF.getSubtarget<IA16Subtarget>().getInstrInfo();
  MachineBasicBlock::iterator I = MBB.begin();
  DebugLoc DL = I == MBB.end() ? DebugLoc() : I->getDebugLoc();
  if (hasFP(MF)) {
    BuildMI(MBB, I, DL, TII.get(X86::IA16_PUSH16r)).addReg(X86::BP);
    BuildMI(MBB, I, DL, TII.get(X86::MOV16rr), X86::BP).addReg(X86::SP);
  }
  uint64_t StackSize = MF.getFrameInfo().getStackSize();
  if (StackSize)
    BuildMI(MBB, I, DL, TII.get(X86::SUB16ri), X86::SP)
        .addReg(X86::SP)
        .addImm(StackSize);
}

void IA16FrameLowering::emitEpilogue(MachineFunction &MF,
                                     MachineBasicBlock &MBB) const {
  const auto &TII = *MF.getSubtarget<IA16Subtarget>().getInstrInfo();
  MachineBasicBlock::iterator I = MBB.getFirstTerminator();
  DebugLoc DL = I == MBB.end() ? DebugLoc() : I->getDebugLoc();
  uint64_t StackSize = MF.getFrameInfo().getStackSize();
  if (hasFP(MF)) {
    if (StackSize)
      BuildMI(MBB, I, DL, TII.get(X86::MOV16rr), X86::SP).addReg(X86::BP);
    BuildMI(MBB, I, DL, TII.get(X86::IA16_POP16r), X86::BP);
  } else if (StackSize) {
    BuildMI(MBB, I, DL, TII.get(X86::ADD16ri), X86::SP)
        .addReg(X86::SP)
        .addImm(StackSize);
  }
}

MachineBasicBlock::iterator IA16FrameLowering::eliminateCallFramePseudoInstr(
    MachineFunction &MF, MachineBasicBlock &MBB,
    MachineBasicBlock::iterator I) const {
  if (I->getOpcode() == X86::IA16_ADJCALLSTACKUP) {
    int64_t Amount = I->getOperand(0).getImm();
    if (Amount) {
      const auto &TII = *MF.getSubtarget<IA16Subtarget>().getInstrInfo();
      BuildMI(MBB, I, I->getDebugLoc(), TII.get(X86::ADD16ri), X86::SP)
          .addReg(X86::SP)
          .addImm(Amount);
    }
  }
  return MBB.erase(I);
}
