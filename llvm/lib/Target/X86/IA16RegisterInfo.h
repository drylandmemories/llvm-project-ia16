//===-- IA16RegisterInfo.h - IA-16 register information --------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_X86_IA16REGISTERINFO_H
#define LLVM_LIB_TARGET_X86_IA16REGISTERINFO_H

#include "MCTargetDesc/X86MCTargetDesc.h"
#include "X86RegisterInfo.h"

namespace llvm {

class IA16RegisterInfo final : public X86GenRegisterInfo {
public:
  IA16RegisterInfo();

  const MCPhysReg *
  getCalleeSavedRegs(const MachineFunction *MF) const override;
  const uint32_t *getCallPreservedMask(const MachineFunction &MF,
                                       CallingConv::ID CC) const override;
  BitVector getReservedRegs(const MachineFunction &MF) const override;
  const TargetRegisterClass *
  getPointerRegClass(unsigned Kind = 0) const override;
  bool eliminateFrameIndex(MachineBasicBlock::iterator II, int SPAdj,
                           unsigned FIOperandNum,
                           RegScavenger *RS = nullptr) const override;
  Register getFrameRegister(const MachineFunction &MF) const override;
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_X86_IA16REGISTERINFO_H
