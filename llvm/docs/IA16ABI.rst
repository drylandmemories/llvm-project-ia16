====================================
IA-16 C and Segmented ELF ABI, v0.2
====================================

:Status: Gate 1 frozen; incompatible changes require a version bump
:Applies to: ``ia16-unknown-none-elf`` and ``ia16-pc-dos-elf``
:Baseline CPU: Intel 8086
:Object format: ELFCLASS32, little endian, ``EM_386``, static linking

This document is the normative ABI contract for the experimental IA-16 LLVM
target.  The target is distinct from X86 ``-m16``: it has a 16-bit C data
model, 16-bit stack ABI, segmented pointers, and an instruction set that is
valid on the selected pre-386 processor.

Version identification
======================

The compiler defines ``__IA16_ABI_VERSION__`` as ``0x000200`` for this ABI.
Compiler-generated objects add an ELF ``.note.ia16.abi`` note with owner
``IA16``, type ``1``, and the NUL-ended ASCII descriptor
``IA16-ABI:0.2``.  LLD retains these notes and diagnoses a link that combines
different recorded IA-16 ABI versions.  An object without this note, such as
handwritten startup assembly, remains linkable and does not establish a
version.

Scalar data model
=================

All quantities are little endian.  A byte is eight bits.  The size and ABI
alignment, in bits, are:

==================  ====  =========
C type              Size  Alignment
==================  ====  =========
``_Bool``              8          8
``char``                8          8
``short``              16         16
``int``                16         16
``long``               32         16
``long long``          64         16
``float``              32         16
``double``             64         16
``long double``        64         16
==================  ====  =========

``char`` is signed by default.  Plain enums use the smallest 8- or 16-bit
integer type that represents all enumerators; ``-fno-short-enums`` forces a
16-bit underlying type.  IEEE binary32 and binary64 operations are implemented
in software unless a later ABI supplement selects an 8087/80287 convention.

``size_t``, ``ptrdiff_t``, ``intptr_t``, and ``uintptr_t`` are 16 bits in the
tiny, small, medium, compact, and large models.  They are 32 bits in the huge
model.  ``wchar_t`` and ``wint_t`` are unsigned 16-bit types.

Struct members use their natural ABI alignment from the table.  Aggregate
alignment is at most 16 bits.  There is no tail-padding reuse across base
objects.  Bit-fields are allocated from the low bit upward in 8- or 16-bit
containers and never straddle a container boundary.  Zero-width bit-fields
force the next 16-bit boundary.

LLVM address spaces and pointers
================================

Address-space numbers are ABI and IR-linking interfaces; they must not be
renumbered.

==  ===========  ==============  ===========  =============================
AS  Source name  Representation  GEP index    Meaning
==  ===========  ==============  ===========  =============================
 0  ``__near``          16 bits      16 bits  DS-relative data
 1  ``__far``           32 bits      16 bits  segment/selector plus offset
 2  ``__huge``          32 bits      32 bits  normalized real-mode data
 3  ``__seg_ss``        16 bits      16 bits  SS-relative data
 4  near code           16 bits      16 bits  CS-relative function
 5  far code            32 bits      16 bits  segment/selector plus offset
==  ===========  ==============  ===========  =============================

The canonical DataLayout is::

  e-m:e-p:16:16-p1:32:16:16:16-p2:32:16:16:32-p3:16:16-p4:16:16-p5:32:16:16:16-i32:16-i64:16-f32:16-f64:16-a:0:16-n8:16-S16

A far or huge representation stores the offset in bits 0--15 and the segment
or selector in bits 16--31.  The null value in every address space is all zero
bits.  Near-to-far data conversions obtain the segment from DS, except that
SS pointers obtain it from SS; near-to-far code conversions obtain it from CS.
Far-to-near conversion discards the segment only when it matches the expected
implicit segment.  A mismatch is undefined behavior unless a checked target
builtin is used.

In real mode, far arithmetic changes only the 16-bit offset.  Arithmetic that
would carry out of that offset, or an access that crosses the 64 KiB segment
limit, is undefined behavior.  Huge pointers denote a 20-bit linear address
and are normalized after construction and arithmetic so that the offset is in
the range 0--15 and ``linear = segment * 16 + offset``.  Huge arithmetic may
cross segment boundaries but must remain within the C object.

In 80286 protected mode, the upper word of a far pointer is a selector.
Arithmetic may change only an offset while remaining within that selector's
descriptor limit.  ``__huge`` construction, conversion, and arithmetic that
could require a selector change are rejected at compile time.  There is no
protected-mode huge-pointer ABI in version 0.1.

Memory models
=============

=======  ==============  ============  ====================================
Model    Code pointers   Data pointers Required segment invariant
=======  ==============  ============  ====================================
tiny     near, 16-bit    near, 16-bit  CS = DS = SS; total image <= 64 KiB
small    near, 16-bit    near, 16-bit  one code and one data/stack segment
medium   far, 32-bit     near, 16-bit  multiple code segments
compact  near, 16-bit    far, 32-bit   multiple data segments
large    far, 32-bit     far, 32-bit   multiple code and data segments
huge     far, 32-bit     huge, 32-bit  normalized real-mode data
=======  ==============  ============  ====================================

Explicit qualifiers override a model's default pointer kind.  Object layout
still follows the destination address space and section.  No single non-huge
object may exceed 65,535 bytes.  A huge object may exceed that size, subject to
the linker layout and 20-bit real-mode address limit.

Registers and call frame
========================

SP is always reserved.  BP is a frame pointer when required.  AX, CX, DX, ES,
and FLAGS are call-clobbered.  BX, SI, DI, BP, and SS are call-preserved.  CS
is changed only by far control transfer.  In the default compatibility mode DS
is call-used, but a function assumes DS names its near-data segment on entry
and restores that value before return.  ``restore_ds`` requires preservation
even when ``no_assume_ds_data`` removes the entry assumption.

The stack grows downward and is aligned to two bytes at every public call
boundary.  Arguments are assigned from right to left.  Each scalar stack slot
is rounded up to an even byte count.  Aggregates are copied by value with an
even-byte round-up.  A near call leaves return IP at ``[SS:SP]`` on callee
entry.  A far call leaves return IP at ``[SS:SP]`` and return CS at
``[SS:SP+2]``.  A frame pointer, when used, is established after saving the
caller's BP; the first stack argument is then at BP+4 for near functions and
BP+6 for far functions.

``cdecl`` uses caller cleanup.  ``stdcall`` uses callee cleanup for fixed
prototypes and caller cleanup for variadic functions.  ``regparmcall`` assigns
successive complete 16-bit argument words to AX, DX, and CX.  A 32-bit scalar
uses the next two available registers only when the pair is AX:DX, with the low
word in AX; otherwise it and all remaining words are passed on the stack.
Stack-resident fixed arguments are callee-popped.  Variadic arguments are all
stack-resident and caller-popped.  Aggregate register splitting is forbidden.

Return values are placed as follows:

* 8- and 16-bit integers and near pointers: AX (8-bit values in AL).
* 32-bit integers, ``float``, far/huge pointers, and far function pointers:
  DX:AX, with the low or offset word in AX.
* 64-bit integers, ``double``, ``long double``, complex values, and aggregates
  larger than four bytes: a caller-provided near or far result pointer matching
  the result object's address space, passed as a hidden first argument.
* Aggregates of one or two bytes: AX.  Aggregates of three or four bytes:
  DX:AX, with memory byte order preserved from low to high.

The hidden result pointer is also returned in DX:AX for far/huge results or AX
for near results.  An interrupt function takes no ordinary arguments, saves
all general and segment registers it uses, restores the incoming segment
state, and returns with IRET.  The hardware interrupt frame is IP, CS, FLAGS at
successive words starting at the entry SP.

Variadic access advances through even-sized stack slots.  Default argument
promotions use 16-bit ``int``/``unsigned int`` and 64-bit ``double``.  A
variadic function may not use callee cleanup or register arguments.

Segmented ELF contract
======================

Objects use ELFCLASS32, ELFDATA2LSB, ``EM_386``, REL relocations, and static
linking.  ELF symbol values are 32-bit linear link addresses.  These relocation
numbers and calculations are frozen, where ``S`` is symbol value, ``A`` the
implicit addend, and ``P`` the relocation place:

=================  ======  ====  =====================
Relocation         Number  Bits  Calculation
=================  ======  ====  =====================
``R_386_16``           20    16  ``S + A``
``R_386_PC16``         21    16  ``S + A - P``
``R_386_SEG16``        45    16  ``(S + A) >> 4``
``R_386_SUB16``        46    16  ``A - S``
``R_386_SUB32``        47    32  ``A - S``
``R_386_HUGE8``        49     8  ``(S + A) & 15``
=================  ======  ====  =====================

Number 44 remains reserved by the Intel386 psABI.  Number 48 is left available
for the existing binutils-ia16 ``R_386_SEGRELATIVE`` convention, which this
static-only ABI does not emit.  ``R_386_HUGE8`` is the LLVM IA-16 extension at
number 49; its calculation follows the published SEGELF normalization design.

Relocations at the same offset are processed in section order and are
cumulative: after the first relocation, ``A`` is the value written by the
previous relocation.  Overflow is diagnosed except for the conventional adjacent
``R_386_16``/``R_386_SUB16`` pair at one offset.  Segment relocations are
invalid in protected-mode output.

Program-entry contracts
=======================

For DOS COM, the linker emits a relocation-free image loaded at PSP offset
0x100.  The initial invariant is CS = DS = ES = SS.  The combined load image,
PSP, and required stack must fit in one 64 KiB segment.

For DOS MZ, load segments are paragraph aligned.  The header records entry
CS:IP, initial SS:SP, and every word requiring loader base relocation.  Startup
code establishes the memory-model invariants before calling C ``main``.

The freestanding 80286 protected-mode runtime owns GDT construction and mode
entry.  C entry receives valid code, near-data, and stack selectors already
loaded.  The ABI does not prescribe DOS services once protected mode is active.

Excluded from v0.2
==================

C++, exceptions, RTTI, atomics, TLS, dynamic linking, PIC, sanitizers, JIT,
OMF, hardware floating point, and cross-selector huge-pointer arithmetic have
no ABI in this version.  Clang defines ``__STDC_NO_ATOMICS__`` and diagnoses
attempts to use unsupported target facilities.
