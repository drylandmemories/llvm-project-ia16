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

bool IA16InstrInfo::expandPostRAPseudo(MachineInstr &MI) const {
  if (MI.getOpcode() != X86::IA16_ZEXT8_16)
    return false;

  MachineBasicBlock &MBB = *MI.getParent();
  const DebugLoc &DL = MI.getDebugLoc();
  Register Dst = MI.getOperand(0).getReg();
  Register Src = MI.getOperand(1).getReg();
  Register LowDst = RI.getSubReg(Dst, X86::sub_8bit);
  assert(LowDst && "IA-16 zero-extension destination needs a low byte");

  BuildMI(MBB, MI, DL, get(X86::XOR16rr), Dst)
      .addReg(Dst, RegState::Undef)
      .addReg(Dst, RegState::Undef);
  BuildMI(MBB, MI, DL, get(X86::MOV8rr), LowDst)
      .addReg(Src, getKillRegState(MI.getOperand(1).isKill()));
  MI.eraseFromParent();
  return true;
}

void IA16InstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator I,
                                const DebugLoc &DL, Register DestReg,
                                Register SrcReg, bool KillSrc,
                                bool RenamableDest,
                                bool RenamableSrc) const {
  if (X86::IA16_GR16RegClass.contains(DestReg) &&
      X86::IA16_GR8RegClass.contains(SrcReg)) {
    Register Low = RI.getSubReg(DestReg, X86::sub_8bit);
    Register High = RI.getSubReg(DestReg, X86::sub_8bit_hi);
    if (!Low || !High)
      report_fatal_error("IA-16 byte-to-word copy needs AX/BX/CX/DX");
    if (SrcReg == Low) {
      BuildMI(MBB, I, DL, get(X86::MOV8ri), High).addImm(0);
      return;
    }
    if (SrcReg == High) {
      BuildMI(MBB, I, DL, get(X86::MOV8rr), Low).addReg(SrcReg);
      BuildMI(MBB, I, DL, get(X86::MOV8ri), High).addImm(0);
      return;
    }
    BuildMI(MBB, I, DL, get(X86::XOR16rr), DestReg)
        .addReg(DestReg, RegState::Undef)
        .addReg(DestReg, RegState::Undef);
    BuildMI(MBB, I, DL, get(X86::MOV8rr), Low)
        .addReg(SrcReg, getKillRegState(KillSrc));
    return;
  }
  if (X86::IA16_GR8RegClass.contains(DestReg) &&
      X86::IA16_GR16RegClass.contains(SrcReg)) {
    Register Low = RI.getSubReg(SrcReg, X86::sub_8bit);
    if (!Low)
      report_fatal_error("IA-16 word-to-byte copy needs AX/BX/CX/DX");
    if (DestReg != Low)
      BuildMI(MBB, I, DL, get(X86::MOV8rr), DestReg)
          .addReg(Low, getKillRegState(KillSrc));
    return;
  }
  if (X86::GR16_NOREXRegClass.contains(DestReg, SrcReg)) {
    BuildMI(MBB, I, DL, get(X86::MOV16rr), DestReg)
        .addReg(SrcReg, getKillRegState(KillSrc), 0);
    return;
  }
  if (X86::GR8_NOREXRegClass.contains(DestReg, SrcReg)) {
    BuildMI(MBB, I, DL, get(X86::MOV8rr), DestReg)
        .addReg(SrcReg, getKillRegState(KillSrc), 0);
    return;
  }
  report_fatal_error(Twine("unsupported IA-16 physical register copy from ") +
                     RI.getName(SrcReg) + " to " + RI.getName(DestReg));
}

void IA16InstrInfo::storeRegToStackSlot(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator I, Register SrcReg,
    bool IsKill, int FrameIndex, const TargetRegisterClass *RC, Register,
    MachineInstr::MIFlag Flags) const {
  unsigned Opc = RC->hasSubClassEq(&X86::IA16_GR8RegClass) ? X86::MOV8mr
                                                           : X86::MOV16mr;
  BuildMI(MBB, I, DebugLoc(), get(Opc))
      .addFrameIndex(FrameIndex)
      .addImm(1)
      .addReg(0)
      .addImm(0)
      .addReg(0)
      .addReg(SrcReg, getKillRegState(IsKill))
      .setMIFlag(Flags);
}

void IA16InstrInfo::loadRegFromStackSlot(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator I, Register DestReg,
    int FrameIndex, const TargetRegisterClass *RC, Register, unsigned SubReg,
    MachineInstr::MIFlag Flags) const {
  unsigned Opc = RC->hasSubClassEq(&X86::IA16_GR8RegClass) ? X86::MOV8rm
                                                           : X86::MOV16rm;
  BuildMI(MBB, I, DebugLoc(), get(Opc), DestReg)
      .addFrameIndex(FrameIndex)
      .addImm(1)
      .addReg(0)
      .addImm(0)
      .addReg(0)
      .setMIFlag(Flags);
}
