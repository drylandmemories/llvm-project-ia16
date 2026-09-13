//===-- IA16RegisterInfo.cpp - IA-16 register information ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "IA16RegisterInfo.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/Support/ErrorHandling.h"
#include <mutex>
#include <vector>

using namespace llvm;

IA16RegisterInfo::IA16RegisterInfo() : X86GenRegisterInfo(X86::IP) {}

const MCPhysReg *
IA16RegisterInfo::getCalleeSavedRegs(const MachineFunction *) const {
  // The cdecl baseline preserves BX, SI, DI, and BP.  Segment-register
  // preservation is represented separately by the DS-preservation ABI mode.
  static const MCPhysReg CalleeSaved[] = {X86::BX, X86::SI, X86::DI, 0};
  return CalleeSaved;
}

const uint32_t *IA16RegisterInfo::getCallPreservedMask(
    const MachineFunction &, CallingConv::ID) const {
  static std::vector<uint32_t> Mask(
      MachineOperand::getRegMaskSize(X86::NUM_TARGET_REGS), 0);
  static std::once_flag Once;
  std::call_once(Once, [&] {
    for (MCPhysReg Reg :
         {X86::BX, X86::SI, X86::DI, X86::BP, X86::SP, X86::SS}) {
      for (MCRegAliasIterator I(Reg, this, true); I.isValid(); ++I)
        Mask[*I / 32] |= 1u << (*I % 32);
    }
  });
  return Mask.data();
}

BitVector IA16RegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs(), true);

  // Unreserve precisely the physical registers represented by the IA-16
  // allocation classes and their aliases.  SP remains reserved.
  for (MCPhysReg Reg : X86::IA16_GR16RegClass)
    Reserved.reset(Reg);
  for (MCPhysReg Reg : X86::IA16_GR8RegClass)
    Reserved.reset(Reg);

  Reserved.set(X86::SP);
  if (MF.getFrameInfo().getObjectIndexBegin() < 0)
    Reserved.set(X86::BP);

  return Reserved;
}

const TargetRegisterClass *
IA16RegisterInfo::getPointerRegClass(unsigned Kind) const {
  return Kind == 0 ? &X86::IA16_GR16RegClass : &X86::IA16_GR16RegClass;
}

bool IA16RegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                           int SPAdj, unsigned FIOperandNum,
                                           RegScavenger *) const {
  assert(SPAdj == 0 && "unexpected IA-16 stack adjustment");
  MachineInstr &MI = *II;
  MachineFunction &MF = *MI.getParent()->getParent();
  int FrameIndex = MI.getOperand(FIOperandNum).getIndex();
  bool HasFP = MF.getFrameInfo().getObjectIndexBegin() < 0;
  int64_t Offset = MF.getFrameInfo().getObjectOffset(FrameIndex);
  if (HasFP) {
    if (FrameIndex < 0)
      Offset += 2; // Account for the BP pushed by the prologue.
  } else {
    Offset += MF.getFrameInfo().getStackSize();
  }
  Offset += MI.getOperand(FIOperandNum + 3).getImm();

  // X86 memory operands are base, scale, index, displacement, segment.
  MI.getOperand(FIOperandNum)
      .ChangeToRegister(HasFP ? X86::BP : X86::SP, false);
  MI.getOperand(FIOperandNum + 3).ChangeToImmediate(Offset);
  return false;
}

Register IA16RegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return MF.getFrameInfo().getObjectIndexBegin() < 0 ? X86::BP : X86::SP;
}
