//===-- IA16Subtarget.h - IA-16 subtarget information ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_X86_IA16SUBTARGET_H
#define LLVM_LIB_TARGET_X86_IA16SUBTARGET_H

#include "IA16FrameLowering.h"
#include "IA16ISelLowering.h"
#include "IA16InstrInfo.h"
#include "llvm/ADT/BitmaskEnum.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/TargetParser/IA16TargetParser.h"

#define GET_SUBTARGETINFO_HEADER
#include "X86GenSubtargetInfo.inc"

namespace llvm {

class IA16Subtarget final : public X86GenSubtargetInfo {
  IA16::CPUKind CPUKind;
  IA16InstrInfo InstrInfo;
  IA16TargetLowering TLInfo;
  IA16FrameLowering FrameLowering;

public:
  IA16Subtarget(const Triple &TT, StringRef CPU, StringRef TuneCPU,
                StringRef FS, const TargetMachine &TM);

  IA16::CPUKind getCPUKind() const { return CPUKind; }
  bool is8086() const {
    return CPUKind == IA16::CPUKind::I8086 ||
           CPUKind == IA16::CPUKind::I8088;
  }
  bool is186OrLater() const {
    return IA16::getCPUFeatures(CPUKind) & IA16::FeatureShiftImmediate;
  }
  bool is286() const { return CPUKind == IA16::CPUKind::I80286; }

  // X86's generated HwMode implementation assumes that every instance is an
  // X86Subtarget.  IA-16 deliberately has a separate subtarget class while
  // sharing the register descriptions, so provide the IA-16 mode directly.
  unsigned getHwModeSet() const override;
  unsigned getHwMode(HwModeType Type = HwMode_Default) const override;

  const IA16InstrInfo *getInstrInfo() const override { return &InstrInfo; }
  const IA16RegisterInfo *getRegisterInfo() const override {
    return &InstrInfo.getRegisterInfo();
  }
  const IA16TargetLowering *getTargetLowering() const override {
    return &TLInfo;
  }
  const IA16FrameLowering *getFrameLowering() const override {
    return &FrameLowering;
  }
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_X86_IA16SUBTARGET_H
