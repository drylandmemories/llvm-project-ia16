//===-- IA16Subtarget.cpp - IA-16 subtarget information ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "IA16Subtarget.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

static StringRef getCPUOrDefault(StringRef CPU) {
  return CPU.empty() ? IA16::getDefaultCPU() : CPU;
}

static std::string getIA16Features(StringRef FS) {
  SmallString<128> Features("+16bit-mode");
  if (!FS.empty()) {
    Features += ',';
    Features += FS;
  }
  return Features.str().str();
}

IA16Subtarget::IA16Subtarget(const Triple &TT, StringRef CPU,
                             StringRef TuneCPU, StringRef FS,
                             const TargetMachine &TM)
    : X86GenSubtargetInfo(TT, getCPUOrDefault(CPU),
                          TuneCPU.empty() ? getCPUOrDefault(CPU) : TuneCPU,
                          getIA16Features(FS)),
      CPUKind(IA16::parseCPU(getCPUOrDefault(CPU))), InstrInfo(*this),
      TLInfo(TM, *this), FrameLowering() {
  if (CPUKind == IA16::CPUKind::Invalid)
    report_fatal_error("invalid IA-16 CPU");
}
