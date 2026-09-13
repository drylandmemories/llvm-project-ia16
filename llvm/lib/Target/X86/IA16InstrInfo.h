//===-- IA16InstrInfo.h - IA-16 instruction information --------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_X86_IA16INSTRINFO_H
#define LLVM_LIB_TARGET_X86_IA16INSTRINFO_H

#include "IA16RegisterInfo.h"
#include "X86InstrInfo.h"

namespace llvm {

class IA16Subtarget;

class IA16InstrInfo final : public X86GenInstrInfo {
  IA16RegisterInfo RI;

public:
  explicit IA16InstrInfo(const IA16Subtarget &STI);

  const IA16RegisterInfo &getRegisterInfo() const { return RI; }

  void copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                   const DebugLoc &DL, Register DestReg, Register SrcReg,
                   bool KillSrc, bool RenamableDest = false,
                   bool RenamableSrc = false) const override;
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_X86_IA16INSTRINFO_H
