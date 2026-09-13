//===-- IA16AsmPrinter.cpp - IA-16 machine instruction printer -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/X86MCTargetDesc.h"
#include "TargetInfo/X86TargetInfo.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

namespace {
class IA16AsmPrinter final : public AsmPrinter {
public:
  IA16AsmPrinter(TargetMachine &TM, std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer), ID) {}

  StringRef getPassName() const override { return "IA-16 Assembly Printer"; }

  void emitInstruction(const MachineInstr *MI) override {
    MCInst Inst;
    Inst.setOpcode(MI->getOpcode());
    if (MI->getOpcode() == X86::RET16) {
      EmitToStreamer(*OutStreamer, Inst);
      return;
    }
    for (const MachineOperand &MO : MI->operands()) {
      if (MO.isRegMask() || (MO.isReg() && MO.isImplicit()))
        continue;
      if (MO.isReg()) {
        Inst.addOperand(MCOperand::createReg(MO.getReg()));
        continue;
      }
      if (MO.isImm()) {
        Inst.addOperand(MCOperand::createImm(MO.getImm()));
        continue;
      }
      report_fatal_error("unsupported IA-16 assembly operand");
    }
    EmitToStreamer(*OutStreamer, Inst);
  }

  static char ID;
};
} // end anonymous namespace

char IA16AsmPrinter::ID = 0;

namespace llvm {
void registerIA16AsmPrinter() {
  static RegisterAsmPrinter<IA16AsmPrinter> IA16(getTheIA16Target());
}
} // end namespace llvm
