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
#include "llvm/Support/MathExtras.h"
#include "llvm/Target/TargetMachine.h"

using namespace llvm;

IA16InstrInfo::IA16InstrInfo(const IA16Subtarget &STI)
    : X86GenInstrInfo(STI, RI, X86::IA16_ADJCALLSTACKDOWN,
                      X86::IA16_ADJCALLSTACKUP, X86::CATCHRET, X86::RET16) {}

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

unsigned IA16InstrInfo::getInstSizeInBytes(const MachineInstr &MI) const {
  if (MI.isDebugInstr() || MI.isMetaInstruction())
    return 0;
  if (MI.getOpcode() == TargetOpcode::INLINEASM ||
      MI.getOpcode() == TargetOpcode::INLINEASM_BR) {
    const MachineFunction &MF = *MI.getParent()->getParent();
    return getInlineAsmLength(MI.getOperand(0).getSymbolName(),
                              *MF.getTarget().getMCAsmInfo());
  }
  if (MI.getOpcode() == X86::JCC_1)
    return 2;
  if (MI.getOpcode() == X86::JMP_2)
    return 3;

  // IA-16 instructions are always shorter than the architectural x86 maximum
  // of 15 bytes.  A conservative upper bound can relax an extra branch but can
  // never leave an out-of-range short Jcc in the output.
  return 15;
}

MachineBasicBlock *
IA16InstrInfo::getBranchDestBlock(const MachineInstr &MI) const {
  assert((MI.getOpcode() == X86::JCC_1 || MI.getOpcode() == X86::JMP_2) &&
         "unexpected IA-16 branch opcode");
  return MI.getOperand(0).getMBB();
}

bool IA16InstrInfo::isBranchOffsetInRange(unsigned BranchOpcode,
                                          int64_t BranchOffset) const {
  if (BranchOpcode == X86::JCC_1)
    return isInt<8>(BranchOffset);
  if (BranchOpcode == X86::JMP_2)
    return true;
  llvm_unreachable("unexpected IA-16 branch opcode");
}

bool IA16InstrInfo::analyzeBranch(MachineBasicBlock &MBB,
                                  MachineBasicBlock *&TBB,
                                  MachineBasicBlock *&FBB,
                                  SmallVectorImpl<MachineOperand> &Cond,
                                  bool) const {
  TBB = nullptr;
  FBB = nullptr;
  Cond.clear();

  auto Last = MBB.getLastNonDebugInstr();
  if (Last == MBB.end())
    return false;
  if (!Last->isBranch())
    return Last->isTerminator();
  if (Last->getOpcode() != X86::JCC_1 && Last->getOpcode() != X86::JMP_2)
    return true;

  if (Last->getOpcode() == X86::JCC_1) {
    TBB = Last->getOperand(0).getMBB();
    Cond.push_back(Last->getOperand(1));
    return false;
  }

  TBB = Last->getOperand(0).getMBB();
  auto FirstTerminator = MBB.getFirstTerminator();
  if (FirstTerminator == Last)
    return false;
  auto Previous = std::prev(Last);
  while (Previous->isDebugInstr())
    --Previous;
  if (Previous->getOpcode() != X86::JCC_1)
    return true;
  FBB = TBB;
  TBB = Previous->getOperand(0).getMBB();
  Cond.push_back(Previous->getOperand(1));
  return false;
}

unsigned IA16InstrInfo::removeBranch(MachineBasicBlock &MBB,
                                     int *BytesRemoved) const {
  unsigned Count = 0;
  unsigned Bytes = 0;
  while (true) {
    auto Last = MBB.getLastNonDebugInstr();
    if (Last == MBB.end() ||
        (Last->getOpcode() != X86::JCC_1 && Last->getOpcode() != X86::JMP_2))
      break;
    Bytes += getInstSizeInBytes(*Last);
    Last->eraseFromParent();
    ++Count;
  }
  if (BytesRemoved)
    *BytesRemoved = Bytes;
  return Count;
}

unsigned IA16InstrInfo::insertBranch(
    MachineBasicBlock &MBB, MachineBasicBlock *TBB, MachineBasicBlock *FBB,
    ArrayRef<MachineOperand> Cond, const DebugLoc &DL, int *BytesAdded) const {
  assert(TBB && "IA-16 branch needs a destination");
  assert(Cond.size() <= 1 && "invalid IA-16 branch condition");
  unsigned Count = 0;
  unsigned Bytes = 0;
  if (!Cond.empty()) {
    BuildMI(&MBB, DL, get(X86::JCC_1)).addMBB(TBB).add(Cond.front());
    ++Count;
    Bytes += 2;
  } else {
    assert(!FBB && "unconditional IA-16 branch cannot have a false block");
  }
  if (FBB || Cond.empty()) {
    BuildMI(&MBB, DL, get(X86::JMP_2)).addMBB(FBB ? FBB : TBB);
    ++Count;
    Bytes += 3;
  }
  if (BytesAdded)
    *BytesAdded = Bytes;
  return Count;
}

bool IA16InstrInfo::reverseBranchCondition(
    SmallVectorImpl<MachineOperand> &Cond) const {
  assert(Cond.size() == 1 && "invalid IA-16 branch condition");
  Cond[0].setImm(X86::GetOppositeBranchCondition(
      static_cast<X86::CondCode>(Cond[0].getImm())));
  return false;
}
