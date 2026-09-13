//===-- IA16ISelLowering.cpp - IA-16 DAG lowering -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "IA16ISelLowering.h"
#include "IA16Subtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

IA16TargetLowering::IA16TargetLowering(const TargetMachine &TM,
                                       const IA16Subtarget &STI)
    : TargetLowering(TM, STI) {
  addRegisterClass(MVT::i8, &X86::IA16_GR8RegClass);
  addRegisterClass(MVT::i16, &X86::IA16_GR16RegClass);
  computeRegisterProperties(STI.getRegisterInfo());

  setStackPointerRegisterToSaveRestore(X86::SP);
  setBooleanContents(ZeroOrOneBooleanContent);
  setBooleanVectorContents(ZeroOrOneBooleanContent);
  setMinFunctionAlignment(Align(1));
  setPrefFunctionAlignment(Align(1));
  setMaxAtomicSizeInBitsSupported(0);

  setOperationAction(ISD::ROTL, MVT::i8, Expand);
  setOperationAction(ISD::ROTR, MVT::i8, Expand);
  setOperationAction(ISD::ROTL, MVT::i16, Expand);
  setOperationAction(ISD::ROTR, MVT::i16, Expand);
  setOperationAction(ISD::BR_JT, MVT::Other, Expand);
  setOperationAction(ISD::BRCOND, MVT::Other, Expand);
  setOperationAction(ISD::SELECT, MVT::i8, Expand);
  setOperationAction(ISD::SELECT, MVT::i16, Expand);
  setOperationAction(ISD::DYNAMIC_STACKALLOC, MVT::i16, Expand);
  setOperationAction(ISD::STACKSAVE, MVT::Other, Expand);
  setOperationAction(ISD::STACKRESTORE, MVT::Other, Expand);
  setOperationAction(ISD::VASTART, MVT::Other, Expand);
  setOperationAction(ISD::VAARG, MVT::Other, Expand);
  setOperationAction(ISD::VAEND, MVT::Other, Expand);
  setOperationAction(ISD::VACOPY, MVT::Other, Expand);

  // These will become compiler-rt libcalls.  Keeping them explicitly out of
  // the legal set prevents accidental selection of post-286 instructions.
  for (MVT VT : {MVT::i8, MVT::i16}) {
    setOperationAction(ISD::CTPOP, VT, Expand);
    setOperationAction(ISD::CTLZ, VT, Expand);
    setOperationAction(ISD::CTTZ, VT, Expand);
  }
}

const char *IA16TargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch (Opcode) {
  case IA16ISD::RET_GLUE:
    return "IA16ISD::RET_GLUE";
  default:
    return nullptr;
  }
}

SDValue IA16TargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {
  if (CallConv != CallingConv::C)
    report_fatal_error("unsupported IA-16 calling convention");
  if (IsVarArg)
    report_fatal_error("IA-16 variadic argument lowering is not implemented");

  MachineFrameInfo &MFI = DAG.getMachineFunction().getFrameInfo();
  int64_t Offset = 2; // Near return IP occupies the first entry-stack word.
  for (const ISD::InputArg &Arg : Ins) {
    EVT VT = Arg.VT;
    if (VT != MVT::i8 && VT != MVT::i16)
      report_fatal_error("unsupported IA-16 argument type");

    int FI = MFI.CreateFixedObject(2, Offset, true);
    SDValue Addr = DAG.getFrameIndex(FI, MVT::i16);
    SDValue Value = DAG.getLoad(MVT::i16, DL, Chain, Addr,
                                MachinePointerInfo::getFixedStack(
                                    DAG.getMachineFunction(), FI));
    if (VT == MVT::i8)
      Value = DAG.getNode(ISD::TRUNCATE, DL, MVT::i8, Value);
    InVals.push_back(Value);
    Offset += 2;
  }
  return Chain;
}

SDValue IA16TargetLowering::LowerCall(
    CallLoweringInfo &, SmallVectorImpl<SDValue> &) const {
  report_fatal_error("IA-16 call lowering is not implemented");
}

bool IA16TargetLowering::CanLowerReturn(
    CallingConv::ID CallConv, MachineFunction &, bool,
    const SmallVectorImpl<ISD::OutputArg> &Outs, LLVMContext &,
    const Type *) const {
  if (CallConv != CallingConv::C || Outs.size() > 1)
    return false;
  return Outs.empty() || Outs.front().VT == MVT::i8 ||
         Outs.front().VT == MVT::i16;
}

SDValue IA16TargetLowering::LowerReturn(
    SDValue Chain, CallingConv::ID CallConv, bool,
    const SmallVectorImpl<ISD::OutputArg> &Outs,
    const SmallVectorImpl<SDValue> &OutVals, const SDLoc &DL,
    SelectionDAG &DAG) const {
  if (!CanLowerReturn(CallConv, DAG.getMachineFunction(), false, Outs,
                      *DAG.getContext(), nullptr))
    report_fatal_error("unsupported IA-16 return type");

  SmallVector<SDValue, 2> RetOps;
  if (!Outs.empty()) {
    MVT VT = Outs.front().VT;
    MCRegister Reg = VT == MVT::i8 ? X86::AL : X86::AX;
    Chain = DAG.getCopyToReg(Chain, DL, Reg, OutVals.front());
    RetOps.push_back(DAG.getRegister(Reg, VT));
  }
  RetOps.push_back(Chain);
  assert(Chain.getNode() && "IA-16 return chain must be valid");
  return SDValue(DAG.getMachineNode(X86::RET16, DL, MVT::Other, RetOps), 0);
}
