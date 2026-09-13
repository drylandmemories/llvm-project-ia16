//===-- IA16ISelLowering.cpp - IA-16 DAG lowering -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "IA16ISelLowering.h"
#include "IA16MachineFunctionInfo.h"
#include "IA16Subtarget.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include <iterator>

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
  setOperationAction(ISD::BR_CC, MVT::i16, Legal);
  setOperationAction(ISD::SELECT, MVT::i8, Expand);
  setOperationAction(ISD::SELECT, MVT::i16, Expand);
  setOperationAction(ISD::SHL_PARTS, MVT::i16, Expand);
  setOperationAction(ISD::SRL_PARTS, MVT::i16, Expand);
  setOperationAction(ISD::SRA_PARTS, MVT::i16, Expand);
  setOperationAction(ISD::DYNAMIC_STACKALLOC, MVT::i16, Expand);
  setOperationAction(ISD::STACKSAVE, MVT::Other, Expand);
  setOperationAction(ISD::STACKRESTORE, MVT::Other, Expand);
  setOperationAction(ISD::VASTART, MVT::Other, Custom);
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

SDValue IA16TargetLowering::LowerOperation(SDValue Op,
                                           SelectionDAG &DAG) const {
  if (Op.getOpcode() != ISD::VASTART)
    report_fatal_error("unexpected custom IA-16 DAG operation");

  MachineFunction &MF = DAG.getMachineFunction();
  const auto *FuncInfo = MF.getInfo<IA16MachineFunctionInfo>();
  SDValue FrameIndex = DAG.getFrameIndex(FuncInfo->getVarArgsFrameIndex(),
                                         getPointerTy(DAG.getDataLayout()));
  const Value *SrcValue = cast<SrcValueSDNode>(Op.getOperand(2))->getValue();
  return DAG.getStore(Op.getOperand(0), SDLoc(Op), FrameIndex,
                      Op.getOperand(1), MachinePointerInfo(SrcValue));
}

MachineBasicBlock *IA16TargetLowering::EmitInstrWithCustomInserter(
    MachineInstr &MI, MachineBasicBlock *MBB) const {
  bool IsSetCC = MI.getOpcode() == X86::IA16_SETCC16;
  if (!IsSetCC && MI.getOpcode() != X86::IA16_SELECTCC16)
    report_fatal_error("unexpected IA-16 custom-inserter instruction");

  MachineFunction &MF = *MBB->getParent();
  MachineRegisterInfo &MRI = MF.getRegInfo();
  const auto &TII = *MF.getSubtarget<IA16Subtarget>().getInstrInfo();
  const BasicBlock *IRBB = MBB->getBasicBlock();
  MachineFunction::iterator InsertAt = std::next(MBB->getIterator());
  MachineBasicBlock *FalseMBB = MF.CreateMachineBasicBlock(IRBB);
  MachineBasicBlock *SinkMBB = MF.CreateMachineBasicBlock(IRBB);
  MF.insert(InsertAt, FalseMBB);
  MF.insert(InsertAt, SinkMBB);

  SinkMBB->splice(SinkMBB->begin(), MBB, std::next(MI.getIterator()),
                  MBB->end());
  SinkMBB->transferSuccessorsAndUpdatePHIs(MBB);
  MBB->addSuccessor(FalseMBB);
  MBB->addSuccessor(SinkMBB);
  FalseMBB->addSuccessor(SinkMBB);

  const DebugLoc &DL = MI.getDebugLoc();
  Register TrueValue;
  Register FalseValue;
  unsigned CCOperand;
  if (IsSetCC) {
    TrueValue = MRI.createVirtualRegister(&X86::IA16_GR16RegClass);
    FalseValue = MRI.createVirtualRegister(&X86::IA16_GR16RegClass);
    CCOperand = 3;
    BuildMI(*MBB, MI, DL, TII.get(X86::MOV16ri), TrueValue).addImm(1);
  } else {
    TrueValue = MI.getOperand(3).getReg();
    FalseValue = MI.getOperand(4).getReg();
    CCOperand = 5;
  }
  BuildMI(*MBB, MI, DL, TII.get(X86::CMP16rr))
      .addReg(MI.getOperand(1).getReg())
      .addReg(MI.getOperand(2).getReg());
  BuildMI(*MBB, MI, DL, TII.get(X86::JCC_1))
      .addMBB(SinkMBB)
      .addImm(MI.getOperand(CCOperand).getImm());
  if (IsSetCC)
    BuildMI(*FalseMBB, FalseMBB->end(), DL, TII.get(X86::MOV16ri),
            FalseValue)
        .addImm(0);
  BuildMI(*SinkMBB, SinkMBB->begin(), DL, TII.get(TargetOpcode::PHI),
          MI.getOperand(0).getReg())
      .addReg(FalseValue)
      .addMBB(FalseMBB)
      .addReg(TrueValue)
      .addMBB(MBB);

  MI.eraseFromParent();
  return SinkMBB;
}

SDValue IA16TargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {
  if (CallConv != CallingConv::C)
    report_fatal_error("unsupported IA-16 calling convention");
  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  int64_t Offset = 2; // Near return IP occupies the first entry-stack word.
  for (const ISD::InputArg &Arg : Ins) {
    EVT VT = Arg.VT;
    if (Arg.Flags.isByVal()) {
      if (VT != MVT::i16)
        report_fatal_error("unsupported IA-16 byval pointer type");
      unsigned Size = Arg.Flags.getByValSize();
      int FI = MFI.CreateFixedObject(Size, Offset, true);
      InVals.push_back(DAG.getFrameIndex(FI, MVT::i16));
      Offset += alignTo(Size, 2u);
      continue;
    }
    if (VT != MVT::i8 && VT != MVT::i16)
      report_fatal_error("unsupported IA-16 argument type");

    int FI = MFI.CreateFixedObject(2, Offset, true);
    SDValue Addr = DAG.getFrameIndex(FI, MVT::i16);
    SDValue Value = DAG.getLoad(VT, DL, Chain, Addr,
                                MachinePointerInfo::getFixedStack(
                                    DAG.getMachineFunction(), FI));
    InVals.push_back(Value);
    if (Arg.Flags.isSRet()) {
      auto *FuncInfo = MF.getInfo<IA16MachineFunctionInfo>();
      if (FuncInfo->getSRetReturnReg())
        report_fatal_error("multiple IA-16 sret arguments");
      Register Reg =
          MF.getRegInfo().createVirtualRegister(getRegClassFor(MVT::i16));
      FuncInfo->setSRetReturnReg(Reg);
      SDValue Copy = DAG.getCopyToReg(DAG.getEntryNode(), DL, Reg, Value);
      Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, Copy, Chain);
    }
    Offset += 2;
  }
  if (IsVarArg) {
    int FI = MFI.CreateFixedObject(2, Offset, true);
    MF.getInfo<IA16MachineFunctionInfo>()->setVarArgsFrameIndex(FI);
  }
  return Chain;
}

SDValue IA16TargetLowering::LowerCall(
    CallLoweringInfo &CLI, SmallVectorImpl<SDValue> &InVals) const {
  if (CLI.CallConv != CallingConv::C)
    report_fatal_error("unsupported IA-16 call convention");
  CLI.IsTailCall = false;

  SelectionDAG &DAG = CLI.DAG;
  SDLoc &DL = CLI.DL;
  unsigned StackBytes = 0;
  for (const ISD::OutputArg &Out : CLI.Outs) {
    if (Out.Flags.isByVal()) {
      if (Out.VT != MVT::i16)
        report_fatal_error("unsupported IA-16 byval pointer type");
      StackBytes += alignTo(Out.Flags.getByValSize(), 2u);
      continue;
    }
    if (Out.VT != MVT::i16)
      report_fatal_error("unsupported IA-16 call argument type");
    StackBytes += 2;
  }
  SDValue Chain = DAG.getCALLSEQ_START(CLI.Chain, StackBytes, 0, DL);

  // cdecl arguments are pushed right-to-left in complete 16-bit words.
  for (int I = static_cast<int>(CLI.Outs.size()) - 1; I >= 0; --I) {
    const ISD::ArgFlagsTy &Flags = CLI.Outs[I].Flags;
    if (Flags.isByVal()) {
      unsigned Size = Flags.getByValSize();
      unsigned RoundedSize = alignTo(Size, 2u);
      SDValue Base = CLI.OutVals[I];
      for (unsigned End = RoundedSize; End != 0; End -= 2) {
        unsigned Offset = End - 2;
        SDValue Address = Base;
        if (Offset)
          Address = DAG.getNode(ISD::ADD, DL, MVT::i16, Base,
                                DAG.getConstant(Offset, DL, MVT::i16));
        MVT LoadVT = Offset + 2 <= Size ? MVT::i16 : MVT::i8;
        SDValue Load =
            DAG.getLoad(LoadVT, DL, Chain, Address, MachinePointerInfo());
        SDValue Value = Load;
        Chain = Load.getValue(1);
        if (LoadVT == MVT::i8)
          Value = DAG.getNode(ISD::ZERO_EXTEND, DL, MVT::i16, Value);
        SDValue Ops[] = {Value, Chain};
        Chain = SDValue(
            DAG.getMachineNode(X86::IA16_PUSH16r, DL, MVT::Other, Ops), 0);
      }
      continue;
    }
    SDValue Ops[] = {CLI.OutVals[I], Chain};
    Chain =
        SDValue(DAG.getMachineNode(X86::IA16_PUSH16r, DL, MVT::Other, Ops), 0);
  }

  SDValue Callee = CLI.Callee;
  unsigned CallOpcode = X86::IA16_CALLpcrel16;
  if (auto *GA = dyn_cast<GlobalAddressSDNode>(Callee))
    Callee = DAG.getTargetGlobalAddress(GA->getGlobal(), DL, MVT::i16,
                                        GA->getOffset());
  else if (auto *ES = dyn_cast<ExternalSymbolSDNode>(Callee))
    Callee = DAG.getTargetExternalSymbol(ES->getSymbol(), MVT::i16);
  else {
    if (Callee.getValueType() != MVT::i16)
      report_fatal_error("unsupported IA-16 indirect-call pointer type");
    CallOpcode = X86::IA16_CALL16r;
  }

  const uint32_t *Mask =
      DAG.getMachineFunction().getSubtarget().getRegisterInfo()->
          getCallPreservedMask(DAG.getMachineFunction(), CLI.CallConv);
  SDValue CallOps[] = {Callee, DAG.getRegisterMask(Mask), Chain};
  SDVTList CallVTs = DAG.getVTList(MVT::Other, MVT::Glue);
  SDNode *Call = DAG.getMachineNode(CallOpcode, DL, CallVTs, CallOps);
  Chain = SDValue(Call, 0);
  SDValue Glue(Call, 1);

  // Keep the call-frame destroy between the call and its result copies. This
  // makes even otherwise-pure legalization libcalls retain and serialize the
  // caller cleanup through the returned value dependency.
  Chain = DAG.getCALLSEQ_END(Chain, StackBytes, 0, Glue, DL);
  Glue = Chain.getValue(1);

  for (auto [Index, In] : llvm::enumerate(CLI.Ins)) {
    if (In.VT != MVT::i16)
      report_fatal_error("unsupported IA-16 call result type");
    if (Index > 1)
      report_fatal_error("unsupported IA-16 multiword call result");
    MCRegister ResultReg = Index == 0 ? X86::AX : X86::DX;
    SDValue Result =
        DAG.getCopyFromReg(Chain, DL, ResultReg, MVT::i16, Glue);
    InVals.push_back(Result);
    Chain = Result.getValue(1);
    Glue = Result.getValue(2);
  }
  return Chain;
}

bool IA16TargetLowering::CanLowerReturn(
    CallingConv::ID CallConv, MachineFunction &, bool,
    const SmallVectorImpl<ISD::OutputArg> &Outs, LLVMContext &,
    const Type *) const {
  if (CallConv != CallingConv::C || Outs.size() > 2)
    return false;
  if (Outs.empty())
    return true;
  if (Outs.size() == 1)
    return Outs.front().VT == MVT::i8 || Outs.front().VT == MVT::i16;
  return Outs[0].VT == MVT::i16 && Outs[1].VT == MVT::i16;
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
  for (auto [Index, Out] : llvm::enumerate(Outs)) {
    MVT VT = Out.VT;
    MCRegister Reg = VT == MVT::i8 ? X86::AL
                                   : Index == 0 ? X86::AX : X86::DX;
    Chain = DAG.getCopyToReg(Chain, DL, Reg, OutVals[Index]);
    RetOps.push_back(DAG.getRegister(Reg, VT));
  }

  if (Register SRetReg = DAG.getMachineFunction()
                             .getInfo<IA16MachineFunctionInfo>()
                             ->getSRetReturnReg()) {
    SDValue Value = DAG.getCopyFromReg(Chain, DL, SRetReg, MVT::i16);
    Chain = Value.getValue(1);
    Chain = DAG.getCopyToReg(Chain, DL, X86::AX, Value);
    RetOps.push_back(DAG.getRegister(X86::AX, MVT::i16));
  }
  RetOps.push_back(Chain);
  assert(Chain.getNode() && "IA-16 return chain must be valid");
  return SDValue(DAG.getMachineNode(X86::RET16, DL, MVT::Other, RetOps), 0);
}
