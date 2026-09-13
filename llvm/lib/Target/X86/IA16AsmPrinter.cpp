//===-- IA16AsmPrinter.cpp - IA-16 machine instruction printer -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/X86MCTargetDesc.h"
#include "TargetInfo/X86TargetInfo.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Module.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCSectionELF.h"
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

  void emitStartOfAsmFile(Module &M) override {
    MCSection *current = OutStreamer->getCurrentSectionOnly();
    MCSection *note = OutContext.getELFSection(".note.ia16.abi", ELF::SHT_NOTE,
                                               /*Flags=*/0);
    OutStreamer->switchSection(note);

    // Record the versioned IA-16 ABI implemented by this object. Keep the
    // owner, type, and descriptor stable: LLD uses them to reject links
    // between objects with incompatible IA-16 ABI revisions.
    emitAlignment(Align(4));
    OutStreamer->emitInt32(5);  // size of "IA16\0"
    OutStreamer->emitInt32(13); // size of "IA16-ABI:0.2\0"
    OutStreamer->emitInt32(1);  // IA-16 ABI version note
    OutStreamer->emitBytes(StringRef("IA16", 5));
    emitAlignment(Align(4));
    OutStreamer->emitBytes(StringRef("IA16-ABI:0.2", 13));
    emitAlignment(Align(4));

    // Mode is a separate, additive note so objects remain ABI v0.2 and tools
    // that do not know the note can safely ignore it. Updated linkers use the
    // explicit mode to prevent real-mode SEGELF calculations in protected
    // output; an object without this note does not establish a mode.
    const auto *ProtectedMode = mdconst::extract_or_null<ConstantInt>(
        M.getModuleFlag("ia16-protected-mode"));
    if (ProtectedMode) {
      MCSection *mode = OutContext.getELFSection(".note.ia16.mode",
                                                 ELF::SHT_NOTE, /*Flags=*/0);
      OutStreamer->switchSection(mode);
      emitAlignment(Align(4));
      OutStreamer->emitInt32(5); // size of "IA16\0"
      if (ProtectedMode->isZero()) {
        OutStreamer->emitInt32(15); // size of "IA16-MODE:real\0"
        OutStreamer->emitInt32(2);  // IA-16 mode note
        OutStreamer->emitBytes(StringRef("IA16", 5));
        emitAlignment(Align(4));
        OutStreamer->emitBytes(StringRef("IA16-MODE:real", 15));
      } else {
        OutStreamer->emitInt32(20); // size of "IA16-MODE:protected\0"
        OutStreamer->emitInt32(2);  // IA-16 mode note
        OutStreamer->emitBytes(StringRef("IA16", 5));
        emitAlignment(Align(4));
        OutStreamer->emitBytes(StringRef("IA16-MODE:protected", 20));
      }
      emitAlignment(Align(4));
    }

    OutStreamer->switchSection(current);
  }

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
      if (MO.isGlobal()) {
        const MCExpr *Expr = MCSymbolRefExpr::create(getSymbol(MO.getGlobal()),
                                                     OutContext);
        if (MO.getOffset())
          Expr = MCBinaryExpr::createAdd(
              Expr, MCConstantExpr::create(MO.getOffset(), OutContext),
              OutContext);
        Inst.addOperand(MCOperand::createExpr(Expr));
        continue;
      }
      if (MO.isSymbol()) {
        const MCExpr *Expr = MCSymbolRefExpr::create(
            OutContext.getOrCreateSymbol(MO.getSymbolName()), OutContext);
        Inst.addOperand(MCOperand::createExpr(Expr));
        continue;
      }
      if (MO.isMBB()) {
        const MCExpr *Expr =
            MCSymbolRefExpr::create(MO.getMBB()->getSymbol(), OutContext);
        Inst.addOperand(MCOperand::createExpr(Expr));
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
