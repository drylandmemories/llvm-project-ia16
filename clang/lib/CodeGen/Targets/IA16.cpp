//===- IA16.cpp - IA-16 ABI implementation -------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "ABIInfoImpl.h"
#include "TargetInfo.h"
#include "llvm/Support/MathExtras.h"

using namespace clang;
using namespace clang::CodeGen;

namespace {

class IA16ABIInfo : public DefaultABIInfo {
public:
  explicit IA16ABIInfo(CodeGenTypes &CGT) : DefaultABIInfo(CGT) {}

  ABIArgInfo classifyReturnType(QualType Ty) const {
    if (Ty->isVoidType())
      return ABIArgInfo::getIgnore();

    uint64_t Size = getContext().getTypeSize(Ty);
    if (isAggregateTypeForABI(Ty)) {
      if (Size == 0)
        return ABIArgInfo::getIgnore();
      if (Size <= 16)
        return ABIArgInfo::getDirect(
            llvm::IntegerType::get(getVMContext(), 16), 0, nullptr,
            /*CanBeFlattened=*/false);
      if (Size <= 32)
        return ABIArgInfo::getDirect(
            llvm::IntegerType::get(getVMContext(), 32), 0, nullptr,
            /*CanBeFlattened=*/false);
      return getNaturalAlignIndirect(Ty,
                                     getDataLayout().getAllocaAddrSpace(),
                                     /*ByVal=*/false);
    }

    // IA-16 returns floating values and integer scalars wider than DX:AX
    // through a caller-provided result object.
    if (Ty->isRealFloatingType() || Ty->isAnyComplexType() || Size > 32)
      return getNaturalAlignIndirect(Ty,
                                     getDataLayout().getAllocaAddrSpace(),
                                     /*ByVal=*/false);

    if (isPromotableIntegerTypeForABI(Ty))
      return ABIArgInfo::getExtend(Ty);
    return ABIArgInfo::getDirect();
  }

  ABIArgInfo classifyArgumentType(QualType Ty) const {
    Ty = useFirstFieldIfTransparentUnion(Ty);

    // Aggregates occupy an even-sized copy in the outgoing stack area.  A
    // byval pointer is the LLVM IR representation of that copy; the IA-16
    // call-lowering layer assigns its final SS-relative location.
    if (isAggregateTypeForABI(Ty)) {
      if (getContext().getTypeSize(Ty) == 0)
        return ABIArgInfo::getIgnore();
      return getNaturalAlignIndirect(Ty,
                                     getDataLayout().getAllocaAddrSpace(),
                                     /*ByVal=*/true);
    }

    if (isPromotableIntegerTypeForABI(Ty))
      return ABIArgInfo::getExtend(Ty);
    return ABIArgInfo::getDirect();
  }

  void computeInfo(CGFunctionInfo &FI) const override {
    if (!getCXXABI().classifyReturnType(FI))
      FI.getReturnInfo() = classifyReturnType(FI.getReturnType());
    for (auto &Arg : FI.arguments())
      Arg.info = classifyArgumentType(Arg.type);
  }

  RValue EmitVAArg(CodeGenFunction &CGF, Address VAListAddr, QualType Ty,
                   AggValueSlot Slot) const override {
    TypeInfoChars TypeInfo = getContext().getTypeInfoInChars(Ty);
    // The ABI caps stack alignment at one word and rounds each argument to an
    // even byte count. Aggregates are present inline in the variadic area.
    TypeInfo.Align = std::min(TypeInfo.Align, CharUnits::fromQuantity(2));
    return emitVoidPtrVAArg(CGF, VAListAddr, Ty, /*IsIndirect=*/false,
                            TypeInfo, CharUnits::fromQuantity(2),
                            /*AllowHigherAlign=*/false, Slot);
  }
};

class IA16TargetCodeGenInfo : public TargetCodeGenInfo {
public:
  explicit IA16TargetCodeGenInfo(CodeGenTypes &CGT)
      : TargetCodeGenInfo(std::make_unique<IA16ABIInfo>(CGT)) {}
};

} // namespace

std::unique_ptr<TargetCodeGenInfo>
CodeGen::createIA16TargetCodeGenInfo(CodeGenModule &CGM) {
  return std::make_unique<IA16TargetCodeGenInfo>(CGM.getTypes());
}
