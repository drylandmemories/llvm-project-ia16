//===-- IA16ISelDAGToDAG.cpp - IA-16 DAG instruction selector -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "IA16ISelLowering.h"
#include "IA16TargetMachine.h"
#include "MCTargetDesc/X86MCTargetDesc.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

#define DEBUG_TYPE "ia16-isel"
#define PASS_NAME "IA-16 DAG Instruction Selection"

namespace {

class IA16DAGToDAGISel final : public SelectionDAGISel {
public:
  IA16DAGToDAGISel(IA16TargetMachine &TM, CodeGenOptLevel OptLevel)
      : SelectionDAGISel(TM, OptLevel) {}

  void Select(SDNode *N) override {
    if (N->isMachineOpcode()) {
      N->setNodeId(-1);
      return;
    }

    SDLoc DL(N);
    switch (N->getOpcode()) {
    case ISD::EntryToken:
    case ISD::Register:
    case ISD::TargetConstant:
    case ISD::CopyToReg:
    case ISD::CopyFromReg:
    case ISD::TokenFactor:
      N->setNodeId(-1);
      return;
    case ISD::Constant: {
      auto *C = cast<ConstantSDNode>(N);
      EVT VT = N->getValueType(0);
      if (VT == MVT::i16) {
        SDValue Imm = CurDAG->getTargetConstant(C->getAPIntValue(), DL, VT);
        CurDAG->SelectNodeTo(N, X86::MOV16ri, MVT::i16, Imm);
        return;
      }
      if (VT == MVT::i8) {
        SDValue Imm = CurDAG->getTargetConstant(C->getAPIntValue(), DL, VT);
        CurDAG->SelectNodeTo(N, X86::MOV8ri, MVT::i8, Imm);
        return;
      }
      break;
    }
    case ISD::ADD:
    case ISD::SUB:
    case ISD::AND:
    case ISD::OR:
    case ISD::XOR:
      if (N->getValueType(0) == MVT::i16) {
        unsigned Opc = 0;
        switch (N->getOpcode()) {
        case ISD::ADD:
          Opc = X86::ADD16rr;
          break;
        case ISD::SUB:
          Opc = X86::SUB16rr;
          break;
        case ISD::AND:
          Opc = X86::AND16rr;
          break;
        case ISD::OR:
          Opc = X86::OR16rr;
          break;
        case ISD::XOR:
          Opc = X86::XOR16rr;
          break;
        default:
          llvm_unreachable("handled IA-16 arithmetic opcode");
        }
        CurDAG->SelectNodeTo(N, Opc, MVT::i16, N->getOperand(0),
                             N->getOperand(1));
        return;
      }
      break;
    case ISD::LOAD: {
      auto *Load = cast<LoadSDNode>(N);
      if (Load->getMemoryVT() != MVT::i16)
        break;
      auto *FI = dyn_cast<FrameIndexSDNode>(Load->getBasePtr());
      if (!FI)
        break;
      SDValue Base = CurDAG->getTargetFrameIndex(FI->getIndex(), MVT::i16);
      SDValue Scale = CurDAG->getTargetConstant(1, DL, MVT::i8);
      SDValue Index = CurDAG->getRegister(0, MVT::i16);
      SDValue Disp = CurDAG->getSignedTargetConstant(0, DL, MVT::i32);
      SDValue Segment = CurDAG->getRegister(0, MVT::i16);
      SmallVector<SDValue, 6> Ops = {Base, Scale, Index,
                                     Disp, Segment, Load->getChain()};
      CurDAG->SelectNodeTo(N, X86::MOV16rm, MVT::i16, MVT::Other, Ops);
      return;
    }
    case ISD::STORE: {
      auto *Store = cast<StoreSDNode>(N);
      if (Store->getMemoryVT() != MVT::i16)
        break;
      auto *FI = dyn_cast<FrameIndexSDNode>(Store->getBasePtr());
      if (!FI)
        break;
      SDValue Base = CurDAG->getTargetFrameIndex(FI->getIndex(), MVT::i16);
      SDValue Scale = CurDAG->getTargetConstant(1, DL, MVT::i8);
      SDValue Index = CurDAG->getRegister(0, MVT::i16);
      SDValue Disp = CurDAG->getSignedTargetConstant(0, DL, MVT::i32);
      SDValue Segment = CurDAG->getRegister(0, MVT::i16);
      SmallVector<SDValue, 7> Ops = {Base,
                                     Scale,
                                     Index,
                                     Disp,
                                     Segment,
                                     Store->getValue(),
                                     Store->getChain()};
      CurDAG->SelectNodeTo(N, X86::MOV16mr, MVT::Other, Ops);
      return;
    }
    case IA16ISD::RET_GLUE: {
      SmallVector<SDValue, 4> Ops;
      for (const SDUse &Op : N->ops())
        Ops.push_back(Op);
      CurDAG->SelectNodeTo(N, X86::RET16, MVT::Other, Ops);
      return;
    }
    default:
      break;
    }

    std::string Message;
    raw_string_ostream OS(Message);
    OS << "cannot select IA-16 node: ";
    N->print(OS, CurDAG);
    report_fatal_error(StringRef(OS.str()));
  }
};

class IA16DAGToDAGISelLegacy final : public SelectionDAGISelLegacy {
public:
  static char ID;
  IA16DAGToDAGISelLegacy(IA16TargetMachine &TM, CodeGenOptLevel OptLevel)
      : SelectionDAGISelLegacy(
            ID, std::make_unique<IA16DAGToDAGISel>(TM, OptLevel)) {}
};

} // end anonymous namespace

char IA16DAGToDAGISelLegacy::ID;

namespace llvm {
void initializeIA16DAGToDAGISelLegacyPass(PassRegistry &);
}

INITIALIZE_PASS(IA16DAGToDAGISelLegacy, DEBUG_TYPE, PASS_NAME, false, false)

FunctionPass *llvm::createIA16ISelDag(IA16TargetMachine &TM,
                                      CodeGenOptLevel OptLevel) {
  return new IA16DAGToDAGISelLegacy(TM, OptLevel);
}
