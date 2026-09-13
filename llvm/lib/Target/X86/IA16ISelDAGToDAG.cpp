//===-- IA16ISelDAGToDAG.cpp - IA-16 DAG instruction selector -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "IA16ISelLowering.h"
#include "IA16TargetMachine.h"
#include "MCTargetDesc/X86BaseInfo.h"
#include "MCTargetDesc/X86MCTargetDesc.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

#define DEBUG_TYPE "ia16-isel"
#define PASS_NAME "IA-16 DAG Instruction Selection"

namespace {

static std::optional<X86::CondCode> getIA16CondCode(ISD::CondCode CC) {
  switch (CC) {
  case ISD::SETEQ:
    return X86::COND_E;
  case ISD::SETNE:
    return X86::COND_NE;
  case ISD::SETLT:
    return X86::COND_L;
  case ISD::SETLE:
    return X86::COND_LE;
  case ISD::SETGT:
    return X86::COND_G;
  case ISD::SETGE:
    return X86::COND_GE;
  case ISD::SETULT:
    return X86::COND_B;
  case ISD::SETULE:
    return X86::COND_BE;
  case ISD::SETUGT:
    return X86::COND_A;
  case ISD::SETUGE:
    return X86::COND_AE;
  default:
    return std::nullopt;
  }
}

class IA16DAGToDAGISel final : public SelectionDAGISel {
  bool selectAddress(SDValue Ptr, const SDLoc &DL,
                     SmallVectorImpl<SDValue> &Ops) {
    SDValue Base;
    SDValue Disp;
    if (auto *FI = dyn_cast<FrameIndexSDNode>(Ptr)) {
      Base = CurDAG->getTargetFrameIndex(FI->getIndex(), MVT::i16);
      Disp = CurDAG->getSignedTargetConstant(0, DL, MVT::i32);
    } else if (auto *GA = dyn_cast<GlobalAddressSDNode>(Ptr)) {
      Base = CurDAG->getRegister(0, MVT::i16);
      Disp = CurDAG->getTargetGlobalAddress(
          GA->getGlobal(), DL, MVT::i16, GA->getOffset());
    } else {
      Base = Ptr;
      Disp = CurDAG->getSignedTargetConstant(0, DL, MVT::i32);
    }
    Ops.push_back(Base);
    Ops.push_back(CurDAG->getTargetConstant(1, DL, MVT::i8));
    Ops.push_back(CurDAG->getRegister(0, MVT::i16));
    Ops.push_back(Disp);
    Ops.push_back(CurDAG->getRegister(0, MVT::i16));
    return true;
  }

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
    case ISD::BasicBlock:
    case ISD::Register:
    case ISD::RegisterMask:
    case ISD::TargetConstant:
    case ISD::TargetGlobalAddress:
    case ISD::TargetExternalSymbol:
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
    case ISD::ZERO_EXTEND:
      if (N->getValueType(0) == MVT::i16 &&
          N->getOperand(0).getValueType() == MVT::i8) {
        CurDAG->SelectNodeTo(N, X86::IA16_ZEXT8_16, MVT::i16,
                             N->getOperand(0));
        return;
      }
      break;
    case ISD::SETCC: {
      if (N->getValueType(0) != MVT::i16 ||
          N->getOperand(0).getValueType() != MVT::i16 ||
          N->getOperand(1).getValueType() != MVT::i16)
        break;
      auto *CCNode = cast<CondCodeSDNode>(N->getOperand(2));
      std::optional<X86::CondCode> CC = getIA16CondCode(CCNode->get());
      if (!CC)
        break;
      SDValue Ops[] = {N->getOperand(0), N->getOperand(1),
                       CurDAG->getTargetConstant(*CC, DL, MVT::i8)};
      CurDAG->SelectNodeTo(N, X86::IA16_SETCC16, MVT::i16, Ops);
      return;
    }
    case ISD::SELECT_CC: {
      if (N->getValueType(0) != MVT::i16 ||
          N->getOperand(0).getValueType() != MVT::i16 ||
          N->getOperand(1).getValueType() != MVT::i16 ||
          N->getOperand(2).getValueType() != MVT::i16 ||
          N->getOperand(3).getValueType() != MVT::i16)
        break;
      auto *CCNode = cast<CondCodeSDNode>(N->getOperand(4));
      std::optional<X86::CondCode> CC = getIA16CondCode(CCNode->get());
      if (!CC)
        break;
      SDValue Ops[] = {N->getOperand(0), N->getOperand(1), N->getOperand(2),
                       N->getOperand(3),
                       CurDAG->getTargetConstant(*CC, DL, MVT::i8)};
      CurDAG->SelectNodeTo(N, X86::IA16_SELECTCC16, MVT::i16, Ops);
      return;
    }
    case ISD::ADD:
    case ISD::SUB:
    case ISD::AND:
    case ISD::OR:
    case ISD::XOR:
      if (N->getValueType(0) == MVT::i8 ||
          N->getValueType(0) == MVT::i16) {
        bool IsByte = N->getValueType(0) == MVT::i8;
        unsigned Opc = 0;
        switch (N->getOpcode()) {
        case ISD::ADD:
          Opc = IsByte ? X86::ADD8rr : X86::ADD16rr;
          break;
        case ISD::SUB:
          Opc = IsByte ? X86::SUB8rr : X86::SUB16rr;
          break;
        case ISD::AND:
          Opc = IsByte ? X86::AND8rr : X86::AND16rr;
          break;
        case ISD::OR:
          Opc = IsByte ? X86::OR8rr : X86::OR16rr;
          break;
        case ISD::XOR:
          Opc = IsByte ? X86::XOR8rr : X86::XOR16rr;
          break;
        default:
          llvm_unreachable("handled IA-16 arithmetic opcode");
        }
        CurDAG->SelectNodeTo(N, Opc, N->getValueType(0), N->getOperand(0),
                             N->getOperand(1));
        return;
      }
      break;
    case ISD::SHL:
    case ISD::SRL:
    case ISD::SRA:
      if (N->getValueType(0) == MVT::i8 ||
          N->getValueType(0) == MVT::i16) {
        MVT VT = N->getSimpleValueType(0);
        auto *Count = dyn_cast<ConstantSDNode>(N->getOperand(1));
        if (!Count) {
          SDValue CountCopy = CurDAG->getCopyToReg(
              CurDAG->getEntryNode(), DL, X86::CL, N->getOperand(1),
              SDValue());
          unsigned Opc = 0;
          if (N->getOpcode() == ISD::SHL)
            Opc = VT == MVT::i8 ? X86::SHL8rCL : X86::SHL16rCL;
          else if (N->getOpcode() == ISD::SRL)
            Opc = VT == MVT::i8 ? X86::SHR8rCL : X86::SHR16rCL;
          else
            Opc = VT == MVT::i8 ? X86::SAR8rCL : X86::SAR16rCL;
          SDValue Ops[] = {N->getOperand(0), CountCopy.getValue(1)};
          CurDAG->SelectNodeTo(N, Opc, VT, Ops);
          return;
        }
        unsigned Amount = Count->getZExtValue();
        unsigned Width = VT.getSizeInBits();
        if (Amount == 0) {
          ReplaceNode(N, N->getOperand(0).getNode());
          return;
        }
        if (Amount >= Width)
          break;
        unsigned Opc = 0;
        if (N->getOpcode() == ISD::SHL)
          Opc = VT == MVT::i8 ? X86::SHL8r1 : X86::SHL16r1;
        else if (N->getOpcode() == ISD::SRL)
          Opc = VT == MVT::i8 ? X86::SHR8r1 : X86::SHR16r1;
        else
          Opc = VT == MVT::i8 ? X86::SAR8r1 : X86::SAR16r1;
        SDNode *Result = N->getOperand(0).getNode();
        SDValue Value = N->getOperand(0);
        for (unsigned I = 0; I != Amount; ++I) {
          Result = CurDAG->getMachineNode(Opc, DL, VT, Value);
          Value = SDValue(Result, 0);
        }
        ReplaceNode(N, Result);
        return;
      }
      break;
    case ISD::BR: {
      SDValue Ops[] = {N->getOperand(1), N->getOperand(0)};
      CurDAG->SelectNodeTo(N, X86::JMP_2, MVT::Other, Ops);
      return;
    }
    case ISD::BR_CC: {
      auto *CCNode = cast<CondCodeSDNode>(N->getOperand(1));
      std::optional<X86::CondCode> CC = getIA16CondCode(CCNode->get());
      if (!CC)
        break;
      SDValue LHS = N->getOperand(2);
      SDValue RHS = N->getOperand(3);
      if (LHS.getValueType() != MVT::i16 || RHS.getValueType() != MVT::i16)
        break;

      SDNode *Cmp =
          CurDAG->getMachineNode(X86::CMP16rr, DL, MVT::Glue, LHS, RHS);
      SDValue Ops[] = {
          N->getOperand(4),
          CurDAG->getTargetConstant(*CC, DL, MVT::i8),
          N->getOperand(0), SDValue(Cmp, 0)};
      CurDAG->SelectNodeTo(N, X86::JCC_1, MVT::Other, Ops);
      return;
    }
    case ISD::LOAD: {
      auto *Load = cast<LoadSDNode>(N);
      if (Load->getMemoryVT() != MVT::i8 &&
          Load->getMemoryVT() != MVT::i16)
        break;
      SmallVector<SDValue, 6> Ops;
      if (!selectAddress(Load->getBasePtr(), DL, Ops))
        break;
      Ops.push_back(Load->getChain());
      unsigned Opc = Load->getMemoryVT() == MVT::i8 ? X86::MOV8rm
                                                    : X86::MOV16rm;
      MVT VT = Load->getMemoryVT() == MVT::i8 ? MVT::i8 : MVT::i16;
      CurDAG->SelectNodeTo(N, Opc, VT, MVT::Other, Ops);
      return;
    }
    case ISD::STORE: {
      auto *Store = cast<StoreSDNode>(N);
      if (Store->getMemoryVT() != MVT::i8 &&
          Store->getMemoryVT() != MVT::i16)
        break;
      SmallVector<SDValue, 7> Ops;
      if (!selectAddress(Store->getBasePtr(), DL, Ops))
        break;
      Ops.push_back(Store->getValue());
      Ops.push_back(Store->getChain());
      unsigned Opc = Store->getMemoryVT() == MVT::i8 ? X86::MOV8mr
                                                     : X86::MOV16mr;
      CurDAG->SelectNodeTo(N, Opc, MVT::Other, Ops);
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
