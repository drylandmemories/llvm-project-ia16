//===-- IA16InstrInfo.cpp - IA-16 instruction information ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "IA16InstrInfo.h"
#include "IA16Subtarget.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

IA16InstrInfo::IA16InstrInfo(const IA16Subtarget &STI)
    : X86GenInstrInfo(STI, RI, X86::ADJCALLSTACKDOWN32,
                      X86::ADJCALLSTACKUP32, X86::CATCHRET, X86::RET16) {}

void IA16InstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator I,
                                const DebugLoc &DL, Register DestReg,
                                Register SrcReg, bool KillSrc,
                                bool RenamableDest,
                                bool RenamableSrc) const {
  if (X86::IA16_GR16RegClass.contains(DestReg, SrcReg)) {
    BuildMI(MBB, I, DL, get(X86::MOV16rr), DestReg)
        .addReg(SrcReg, getKillRegState(KillSrc), 0);
    return;
  }
  if (X86::IA16_GR8RegClass.contains(DestReg, SrcReg)) {
    BuildMI(MBB, I, DL, get(X86::MOV8rr), DestReg)
        .addReg(SrcReg, getKillRegState(KillSrc), 0);
    return;
  }
  report_fatal_error("unsupported IA-16 physical register copy");
}
