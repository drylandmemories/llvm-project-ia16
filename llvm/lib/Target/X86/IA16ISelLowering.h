//===-- IA16ISelLowering.h - IA-16 DAG lowering ---------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_X86_IA16ISELLOWERING_H
#define LLVM_LIB_TARGET_X86_IA16ISELLOWERING_H

#include "llvm/CodeGen/TargetLowering.h"

namespace llvm {

class IA16Subtarget;

namespace IA16ISD {
enum NodeType : unsigned {
  FIRST_NUMBER = ISD::BUILTIN_OP_END,
  RET_GLUE,
};
} // end namespace IA16ISD

class IA16TargetLowering final : public TargetLowering {
public:
  IA16TargetLowering(const TargetMachine &TM, const IA16Subtarget &STI);

  bool areJTsAllowed(const Function *) const override { return false; }
  const char *getTargetNodeName(unsigned Opcode) const override;
  MVT getScalarShiftAmountTy(const DataLayout &, EVT) const override {
    return MVT::i8;
  }
  MVT::SimpleValueType getCmpLibcallReturnType() const override {
    return MVT::i16;
  }

  MachineBasicBlock *
  EmitInstrWithCustomInserter(MachineInstr &MI,
                              MachineBasicBlock *MBB) const override;

private:
  SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const override;
  SDValue LowerFormalArguments(
      SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
      const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
      SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const override;
  SDValue LowerCall(CallLoweringInfo &CLI,
                    SmallVectorImpl<SDValue> &InVals) const override;
  bool CanLowerReturn(CallingConv::ID CallConv, MachineFunction &MF,
                      bool IsVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &Outs,
                      LLVMContext &Context,
                      const Type *RetTy) const override;
  SDValue LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &Outs,
                      const SmallVectorImpl<SDValue> &OutVals, const SDLoc &DL,
                      SelectionDAG &DAG) const override;
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_X86_IA16ISELLOWERING_H
