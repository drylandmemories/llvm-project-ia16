//===-- llvm-ia16-mz.cpp - IA-16 DOS executable packer ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Packs a statically linked IA-16 ELF32 image as a DOS .COM or MZ executable.
// MZ loader fixups are reconstructed from retained R_386_SEG16 relocations
// (link with --emit-relocs). Other SEGELF relocations must already be resolved
// by the linker.
//
//===----------------------------------------------------------------------===//

#include "llvm/BinaryFormat/ELF.h"
#include "llvm/Object/ELFObjectFile.h"
#include "llvm/Object/ObjectFile.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Endian.h"
#include "llvm/Support/FileOutputBuffer.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/WithColor.h"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <limits>
#include <optional>
#include <tuple>
#include <vector>

using namespace llvm;
using namespace llvm::object;

namespace {

enum class OutputKind { COM, MZ };

cl::opt<std::string> InputFilename(cl::Positional, cl::Required,
                                   cl::desc("<linked IA-16 ELF>"));
cl::opt<std::string> OutputFilename("o", cl::Required,
                                    cl::desc("Output executable"));
cl::opt<OutputKind> Format(
    "format", cl::Required, cl::desc("DOS output format"),
    cl::values(clEnumValN(OutputKind::COM, "com", "DOS .COM image"),
               clEnumValN(OutputKind::MZ, "mz", "DOS MZ executable")));
cl::opt<unsigned> InitialSS(
    "ss", cl::init(0),
    cl::desc("Initial MZ SS, relative to the load module (default: 0)"));
cl::opt<unsigned> InitialSP(
    "sp", cl::init(0xfffe),
    cl::desc("Initial MZ SP (default: 0xfffe)"));
cl::opt<unsigned> MinAlloc(
    "min-alloc", cl::init(0),
    cl::desc("Minimum extra MZ allocation in paragraphs (default: 0)"));
cl::opt<unsigned> MaxAlloc(
    "max-alloc", cl::init(0xffff),
    cl::desc("Maximum extra MZ allocation in paragraphs (default: 65535)"));

struct ImageSection {
  SectionRef Section;
  uint64_t Address;
  uint64_t Size;
  bool IsBSS;
};

struct DOSRelocation {
  uint16_t Offset;
  uint16_t Segment;
};

[[noreturn]] void fail(const Twine &Message) {
  WithColor::error(errs(), "llvm-ia16-mz") << Message << '\n';
  exit(1);
}

template <typename T> T unwrap(Expected<T> Value, const Twine &Context) {
  if (!Value)
    fail(Context + ": " + toString(Value.takeError()));
  return std::move(*Value);
}

static void set16(MutableArrayRef<char> Bytes, size_t Offset, uint16_t Value) {
  support::endian::write16le(Bytes.data() + Offset, Value);
}

static uint16_t checked16(uint64_t Value, StringRef What) {
  if (Value > std::numeric_limits<uint16_t>::max())
    fail(What + " does not fit in 16 bits");
  return static_cast<uint16_t>(Value);
}

static void writeOutput(ArrayRef<char> Bytes) {
  Expected<std::unique_ptr<FileOutputBuffer>> BufferOrErr =
      FileOutputBuffer::create(OutputFilename, Bytes.size());
  if (!BufferOrErr)
    fail("cannot create output: " + toString(BufferOrErr.takeError()));
  std::unique_ptr<FileOutputBuffer> Buffer = std::move(*BufferOrErr);
  std::memcpy(Buffer->getBufferStart(), Bytes.data(), Bytes.size());
  if (Error E = Buffer->commit())
    fail("cannot write output: " + toString(std::move(E)));
}

static bool isAllocSection(SectionRef Section) {
  return (ELFSectionRef(Section).getFlags() & ELF::SHF_ALLOC) != 0;
}

static std::vector<ImageSection> collectImageSections(const ObjectFile &Obj) {
  std::vector<ImageSection> Sections;
  for (SectionRef Section : Obj.sections()) {
    if (!isAllocSection(Section) || Section.getSize() == 0)
      continue;
    Sections.push_back(
        {Section, Section.getAddress(), Section.getSize(), Section.isBSS()});
  }
  if (Sections.empty())
    fail("input has no non-empty SHF_ALLOC sections");
  llvm::sort(Sections, [](const ImageSection &L, const ImageSection &R) {
    return L.Address < R.Address;
  });
  for (size_t I = 1; I != Sections.size(); ++I)
    if (Sections[I - 1].Address + Sections[I - 1].Size > Sections[I].Address)
      fail("overlapping allocated ELF sections are unsupported");
  return Sections;
}

static SmallVector<char, 0>
buildLoadImage(ArrayRef<ImageSection> Sections, uint64_t ImageBase,
               uint64_t FileEnd) {
  if (FileEnd < ImageBase || FileEnd - ImageBase > UINT32_MAX)
    fail("linked load image is too large");
  SmallVector<char, 0> Image(FileEnd - ImageBase, 0);
  for (const ImageSection &IS : Sections) {
    if (IS.IsBSS)
      continue;
    StringRef Contents = unwrap(IS.Section.getContents(), "cannot read section");
    if (Contents.size() != IS.Size)
      fail("ELF section contents do not match its declared size");
    std::memcpy(Image.data() + IS.Address - ImageBase, Contents.data(),
                Contents.size());
  }
  return Image;
}

static std::vector<DOSRelocation>
collectMZRelocations(const ObjectFile &Obj, uint64_t ImageBase,
                     uint64_t MemoryEnd) {
  std::vector<DOSRelocation> Result;
  bool IsRelocatable =
      cast<ELFObjectFileBase>(Obj).getEType() == ELF::ET_REL;
  for (SectionRef RelocationSection : Obj.sections()) {
    Expected<section_iterator> RelocatedOrErr =
        RelocationSection.getRelocatedSection();
    if (!RelocatedOrErr) {
      consumeError(RelocatedOrErr.takeError());
      continue;
    }
    if (*RelocatedOrErr == Obj.section_end())
      continue;
    SectionRef Section = **RelocatedOrErr;
    if (!isAllocSection(Section))
      continue;
    for (RelocationRef Relocation : RelocationSection.relocations()) {
      if (Relocation.getType() != ELF::R_386_SEG16)
        continue;
      uint64_t Address = Relocation.getOffset();
      if (IsRelocatable)
        Address += Section.getAddress();
      if (Address < ImageBase || Address + 2 > MemoryEnd)
        fail("R_386_SEG16 relocation lies outside the load image");
      uint64_t LinearOffset = Address - ImageBase;
      Result.push_back({static_cast<uint16_t>(LinearOffset & 0xf),
                        checked16(LinearOffset >> 4,
                                  "MZ relocation segment")});
    }
  }
  llvm::sort(Result, [](const DOSRelocation &L, const DOSRelocation &R) {
    return std::tie(L.Segment, L.Offset) < std::tie(R.Segment, R.Offset);
  });
  Result.erase(std::unique(Result.begin(), Result.end(),
                           [](const DOSRelocation &L,
                              const DOSRelocation &R) {
                             return L.Offset == R.Offset &&
                                    L.Segment == R.Segment;
                           }),
               Result.end());
  return Result;
}

static void packCOM(const ObjectFile &Obj, ArrayRef<ImageSection> Sections,
                    uint64_t Entry) {
  constexpr uint64_t COMOrigin = 0x100;
  uint64_t FileEnd = COMOrigin;
  uint64_t MemoryEnd = COMOrigin;
  for (const ImageSection &IS : Sections) {
    if (IS.Address < COMOrigin)
      fail(".COM allocated sections must start at or above PSP offset 0x100");
    MemoryEnd = std::max(MemoryEnd, IS.Address + IS.Size);
    if (!IS.IsBSS)
      FileEnd = std::max(FileEnd, IS.Address + IS.Size);
  }
  if (Entry != COMOrigin)
    fail(".COM entry point must be 0x100");
  if (MemoryEnd > 0x10000)
    fail(".COM load image and BSS exceed the 65280-byte limit");
  if (!collectMZRelocations(Obj, COMOrigin, FileEnd).empty())
    fail(".COM output cannot contain segment loader relocations");
  writeOutput(buildLoadImage(Sections, COMOrigin, FileEnd));
}

static void packMZ(const ObjectFile &Obj, ArrayRef<ImageSection> Sections,
                   uint64_t Entry) {
  uint64_t ImageBase = Sections.front().Address;
  if (ImageBase & 0xf)
    fail("MZ load image base must be paragraph aligned");

  uint64_t MemoryEnd = ImageBase;
  uint64_t FileEnd = ImageBase;
  for (const ImageSection &IS : Sections) {
    MemoryEnd = std::max(MemoryEnd, IS.Address + IS.Size);
    if (!IS.IsBSS)
      FileEnd = std::max(FileEnd, IS.Address + IS.Size);
  }
  if (Entry < ImageBase || Entry >= MemoryEnd)
    fail("ELF entry point lies outside the load image");

  std::vector<DOSRelocation> Relocations =
      collectMZRelocations(Obj, ImageBase, MemoryEnd);
  SmallVector<char, 0> Image = buildLoadImage(Sections, ImageBase, FileEnd);

  constexpr uint16_t RelocTableOffset = 0x1c;
  uint64_t HeaderBytes = alignTo(RelocTableOffset + Relocations.size() * 4, 16);
  uint16_t HeaderParagraphs = checked16(HeaderBytes / 16, "MZ header size");
  uint64_t TotalBytes = HeaderBytes + Image.size();
  uint16_t Pages = checked16(divideCeil(TotalBytes, uint64_t(512)),
                             "MZ executable page count");
  uint16_t LastPageBytes = static_cast<uint16_t>(TotalBytes % 512);
  uint64_t RelativeEntry = Entry - ImageBase;
  uint64_t BSSParagraphs = divideCeil(MemoryEnd - FileEnd, uint64_t(16));
  uint16_t RequiredMinAlloc =
      checked16(BSSParagraphs + MinAlloc, "minimum allocation including BSS");
  if (MaxAlloc < RequiredMinAlloc)
    fail("maximum allocation is smaller than the required minimum");

  SmallVector<char, 0> Output(HeaderBytes, 0);
  set16(Output, 0x00, 0x5a4d);
  set16(Output, 0x02, LastPageBytes);
  set16(Output, 0x04, Pages);
  set16(Output, 0x06, checked16(Relocations.size(), "MZ relocation count"));
  set16(Output, 0x08, HeaderParagraphs);
  set16(Output, 0x0a, RequiredMinAlloc);
  set16(Output, 0x0c, checked16(MaxAlloc, "maximum allocation"));
  set16(Output, 0x0e, checked16(InitialSS, "initial SS"));
  set16(Output, 0x10, checked16(InitialSP, "initial SP"));
  set16(Output, 0x12, 0);
  set16(Output, 0x14, static_cast<uint16_t>(RelativeEntry & 0xf));
  set16(Output, 0x16, checked16(RelativeEntry >> 4, "entry CS"));
  set16(Output, 0x18, RelocTableOffset);
  set16(Output, 0x1a, 0);

  size_t RelocOffset = RelocTableOffset;
  for (const DOSRelocation &Relocation : Relocations) {
    set16(Output, RelocOffset, Relocation.Offset);
    set16(Output, RelocOffset + 2, Relocation.Segment);
    RelocOffset += 4;
  }
  Output.append(Image.begin(), Image.end());
  writeOutput(Output);
}

} // end anonymous namespace

int main(int Argc, char **Argv) {
  InitLLVM X(Argc, Argv);
  cl::ParseCommandLineOptions(Argc, Argv,
                              "LLVM IA-16 DOS executable packer\n");

  ErrorOr<std::unique_ptr<MemoryBuffer>> BufferOrErr =
      MemoryBuffer::getFileOrSTDIN(InputFilename);
  if (!BufferOrErr)
    fail("cannot read input: " + BufferOrErr.getError().message());
  Expected<std::unique_ptr<ObjectFile>> ObjOrErr =
      ObjectFile::createObjectFile((*BufferOrErr)->getMemBufferRef());
  if (!ObjOrErr)
    fail("input is not an object file: " + toString(ObjOrErr.takeError()));
  std::unique_ptr<ObjectFile> Obj = std::move(*ObjOrErr);

  // IA-16 deliberately retains EM_386 interoperability, so an object reader
  // classifies the container as x86. The explicit packer invocation and the
  // IA-16 linker script establish the narrower ABI contract.
  if (!isa<ELF32LEObjectFile>(*Obj) ||
      cast<ELFObjectFileBase>(*Obj).getEMachine() != ELF::EM_386)
    fail("input must be little-endian ELF32 with EM_386");
  if (cast<ELFObjectFileBase>(*Obj).getEType() != ELF::ET_EXEC)
    fail("input must be a statically linked ET_EXEC image");

  Expected<uint64_t> EntryOrErr = Obj->getStartAddress();
  if (!EntryOrErr)
    fail("cannot read ELF entry point: " + toString(EntryOrErr.takeError()));
  std::vector<ImageSection> Sections = collectImageSections(*Obj);
  if (Format == OutputKind::COM)
    packCOM(*Obj, Sections, *EntryOrErr);
  else
    packMZ(*Obj, Sections, *EntryOrErr);
  return 0;
}
