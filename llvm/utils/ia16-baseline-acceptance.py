#!/usr/bin/env python3
"""Run the reproducible IA-16 baseline acceptance suite.

This is intentionally an acceptance harness, not an implementation repair
tool.  It uses an existing LLVM build, writes only to a new evidence directory,
and records source, build, test, opcode-policy, and emulator evidence as
separate layers.
"""

from __future__ import annotations

import argparse
import datetime
import hashlib
import json
import os
import pathlib
import re
import shutil
import subprocess
import sys
import tempfile
import time
from typing import Any, Iterable


OPT_LEVELS = ("O0", "O2", "Os")
CPUS = {
    "i8086": {"rank": 0, "dosbox": "8086"},
    "i80186": {"rank": 1, "dosbox": "80186"},
    "i80286": {"rank": 2, "dosbox": "286"},
}
CPU_NAME_BY_RANK = {0: "i8086", 1: "i80186", 2: "i80286", 3: "post-i80286"}
DOS_NAME_83 = re.compile(r"^[A-Z0-9_]{1,8}(?:\.[A-Z0-9_]{1,3})?$")
REQUIRED_TEST_PATHS = {
    "lld/test/ELF/ia16-segelf-relocs.s",
    "llvm/test/Acceptance/IA16/baseline-harness.test",
}
REQUIRED_TARGET_PARSER_TESTS = (
    "IA16TargetParserTest.CPUParsingAndAliases",
    "IA16TargetParserTest.GenerationFeatures",
    "IA16TargetParserTest.ValidCPUList",
    "TripleTest.ParsedIDs",
    "TripleTest.BitWidthChecks",
    "TripleTest.FileFormat",
    "DataLayoutTest.IA16SegmentedPointers",
)

# These opcode bytes are the generation boundaries relevant to code emitted by
# this fixture.  Every decoded instruction is recorded.  Prefixes and the 0x0f
# escape fail closed so a post-286 instruction cannot be admitted by a mnemonic
# spelling that happens to look old.
PREFIXES_8086 = {0x26, 0x2E, 0x36, 0x3E, 0xF0, 0xF2, 0xF3}
PREFIXES_POST_286 = {0x64, 0x65, 0x66, 0x67}
OPCODES_80186 = {
    0x60,
    0x61,
    0x62,
    0x68,
    0x69,
    0x6A,
    0x6B,
    0x6C,
    0x6D,
    0x6E,
    0x6F,
    0xC0,
    0xC1,
    0xC8,
    0xC9,
}
OPCODES_80286 = {0x63}

PROBE_C = r"""volatile int signed_left;
volatile int signed_right;
volatile int signed_result;
volatile unsigned unsigned_left;
volatile unsigned unsigned_right;
volatile unsigned unsigned_result;

__attribute__((noinline)) unsigned swap_word_runtime(unsigned value) {
  return __builtin_bswap16(value);
}

static int verify_integer_arithmetic(void) {
  unsigned_left = 0x6a5aU;
  unsigned_right = 0x1234U;
  unsigned_result = unsigned_left + unsigned_right;
  if (unsigned_result != 0x7c8eU)
    return 1;
  unsigned_result = unsigned_left - unsigned_right;
  if (unsigned_result != 0x5826U)
    return 2;
  unsigned_left = 123U;
  unsigned_right = 211U;
  unsigned_result = unsigned_left * unsigned_right;
  if (unsigned_result != 25953U)
    return 3;
  unsigned_left = 0x1234U;
  unsigned_right = 3U;
  unsigned_result = (unsigned_left << unsigned_right) ^
                    (unsigned_left >> unsigned_right);
  if (unsigned_result != 0x93e6U)
    return 4;
  return 0;
}

static int verify_division(void) {
  signed_left = 30000;
  signed_right = 97;
  signed_result = signed_left / signed_right;
  if (signed_result != 309)
    return 5;
  signed_result = signed_left % signed_right;
  if (signed_result != 27)
    return 6;

  signed_left = -30000;
  signed_right = 97;
  signed_result = signed_left / signed_right;
  if (signed_result != -309)
    return 7;
  signed_result = signed_left % signed_right;
  if (signed_result != -27)
    return 8;

  signed_left = 30000;
  signed_right = -97;
  signed_result = signed_left / signed_right;
  if (signed_result != -309)
    return 9;
  signed_result = signed_left % signed_right;
  if (signed_result != 27)
    return 10;

  signed_left = -32768;
  signed_right = 1;
  signed_result = signed_left / signed_right;
  if (signed_result != -32768)
    return 11;
  signed_result = signed_left % signed_right;
  if (signed_result != 0)
    return 12;

  signed_left = 32767;
  signed_right = -1;
  signed_result = signed_left / signed_right;
  if (signed_result != -32767)
    return 13;

  unsigned_left = 65535U;
  unsigned_right = 251U;
  unsigned_result = unsigned_left / unsigned_right;
  if (unsigned_result != 261U)
    return 14;
  unsigned_result = unsigned_left % unsigned_right;
  if (unsigned_result != 24U)
    return 15;

  unsigned_left = 32768U;
  unsigned_right = 3U;
  unsigned_result = unsigned_left / unsigned_right;
  if (unsigned_result != 10922U)
    return 16;
  unsigned_result = unsigned_left % unsigned_right;
  if (unsigned_result != 2U)
    return 17;

  unsigned_left = 65535U;
  unsigned_right = 65535U;
  unsigned_result = unsigned_left / unsigned_right;
  if (unsigned_result != 1U)
    return 18;
  return 0;
}

static int verify_byte_swaps(void) {
  unsigned_left = 0x1234U;
  unsigned_result = swap_word_runtime(unsigned_left);
  if (unsigned_result != 0x3412U)
    return 19;
  unsigned_left = 0x00ffU;
  unsigned_result = swap_word_runtime(unsigned_left);
  if (unsigned_result != 0xff00U)
    return 20;
  unsigned_left = 0xa55aU;
  unsigned_result = swap_word_runtime(unsigned_left);
  if (unsigned_result != 0x5aa5U)
    return 21;
  return 0;
}

int verify_probe(void) {
  int result = verify_integer_arithmetic();
  if (result)
    return result;
  result = verify_division();
  if (result)
    return result;
  return verify_byte_swaps();
}
"""

START_S = r"""	.text
	.globl	_start
	.type	_start,@function
_start:
	callw	verify_probe
	testw	%ax, %ax
	jne	.Lfail

	movw	$.Lsuccess_name, %dx
	xorw	%cx, %cx
	movw	$0x3c00, %ax
	int	$0x21
	jc	.Lfile_fail
	movw	%ax, %bx
	movw	$.Lsuccess_text, %dx
	movw	$2, %cx
	movw	$0x4000, %ax
	int	$0x21
	movw	$0x3e00, %ax
	int	$0x21
	movw	$0x4c00, %ax
	int	$0x21

.Lfail:
	movb	%al, .Lfail_code
	movw	$.Lfail_name, %dx
	xorw	%cx, %cx
	movw	$0x3c00, %ax
	int	$0x21
	jc	.Lfile_fail
	movw	%ax, %bx
	movw	$.Lfail_code, %dx
	movw	$1, %cx
	movw	$0x4000, %ax
	int	$0x21
	movw	$0x3e00, %ax
	int	$0x21
	movb	.Lfail_code, %al
	movb	$0x4c, %ah
	int	$0x21

.Lfile_fail:
	movw	$0x4c19, %ax
	int	$0x21
	.size	_start, .-_start

	.section .rodata,"a",@progbits
.Lsuccess_name:
	.asciz	"RESULT.OK"
.Lsuccess_text:
	.ascii	"OK"
.Lfail_name:
	.asciz	"FAIL.BIN"

	.data
.Lfail_code:
	.byte	0
"""

DOSBOX_CONFIG = """[sdl]
fullscreen=false
autolock=false

[dosbox]
machine=ega

[cpu]
cputype={dosbox_cpu}
core=normal
cycles=fixed 3000

[mixer]
nosound=true

[speaker]
pcspeaker=false
tandy=off
disney=false
"""


class AcceptanceError(RuntimeError):
    pass


def utc_now() -> str:
    return datetime.datetime.now(datetime.timezone.utc).isoformat()


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def sha256_file(path: pathlib.Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def is_dos_83_name(name: str) -> bool:
    return DOS_NAME_83.fullmatch(name) is not None


def write_text(path: pathlib.Path, value: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8", newline="\n") as stream:
        stream.write(value)


def write_json(path: pathlib.Path, value: Any) -> None:
    write_text(path, json.dumps(value, indent=2, sort_keys=True) + "\n")


def read_cache(path: pathlib.Path) -> dict[str, str]:
    result: dict[str, str] = {}
    for line in path.read_text(encoding="utf-8").splitlines():
        if not line or line.startswith("#") or line.startswith("//") or "=" not in line:
            continue
        key_and_type, value = line.split("=", 1)
        key = key_and_type.split(":", 1)[0]
        result[key] = value
    return result


class Recorder:
    def __init__(self, output: pathlib.Path) -> None:
        self.output = output
        self.logs = output / "logs"
        self.logs.mkdir()
        self.sequence = 0
        self.commands: list[dict[str, Any]] = []

    def run(
        self,
        name: str,
        argv: Iterable[object],
        *,
        cwd: pathlib.Path,
        timeout: int | None = None,
        env: dict[str, str] | None = None,
    ) -> subprocess.CompletedProcess[str]:
        self.sequence += 1
        command = [str(item) for item in argv]
        stem = f"{self.sequence:03d}-{name}"
        log_path = self.logs / f"{stem}.log"
        command_path = self.logs / f"{stem}.command.json"
        started = utc_now()
        started_monotonic = time.monotonic()
        timed_out = False
        try:
            completed = subprocess.run(
                command,
                cwd=cwd,
                env=env,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                errors="replace",
                timeout=timeout,
                check=False,
            )
        except subprocess.TimeoutExpired as error:
            timed_out = True
            output = error.stdout or ""
            if isinstance(output, bytes):
                output = output.decode("utf-8", errors="replace")
            completed = subprocess.CompletedProcess(command, 124, output)
        ended = utc_now()
        duration = time.monotonic() - started_monotonic
        write_text(log_path, completed.stdout or "")
        record = {
            "argv": command,
            "cwd": str(cwd),
            "duration_seconds": round(duration, 6),
            "ended_at": ended,
            "exit_code": completed.returncode,
            "log": str(log_path.relative_to(self.output)),
            "log_sha256": sha256_file(log_path),
            "name": name,
            "started_at": started,
            "timed_out": timed_out,
        }
        write_json(command_path, record)
        self.commands.append(record)
        write_json(self.output / "commands.json", self.commands)
        return completed


def classify_instruction(raw: list[int], selected_cpu: str) -> dict[str, Any]:
    if not raw:
        raise AcceptanceError("decoded instruction has no bytes")
    index = 0
    required_rank = 0
    reason = "8086 base opcode"
    while index < len(raw) and raw[index] in PREFIXES_8086:
        index += 1
    if index < len(raw) and raw[index] in PREFIXES_POST_286:
        required_rank = 3
        reason = "FS/GS or 32-bit operand/address prefix requires a post-286 CPU"
    elif index == len(raw):
        required_rank = 3
        reason = "prefix-only decoding is invalid"
    else:
        opcode = raw[index]
        if opcode == 0x0F:
            required_rank = 3
            reason = "0x0f escape is outside this real-mode fixture policy"
        elif opcode in OPCODES_80286:
            required_rank = 2
            reason = "80286 opcode"
        elif opcode in OPCODES_80186:
            required_rank = 1
            reason = "80186 opcode"
    selected_rank = CPUS[selected_cpu]["rank"]
    return {
        "allowed": required_rank <= selected_rank,
        "reason": reason,
        "required_cpu": CPU_NAME_BY_RANK[required_rank],
        "selected_cpu": selected_cpu,
    }


DISASSEMBLY_LINE = re.compile(
    r"^\s*([0-9a-fA-F]+):\s+((?:[0-9a-fA-F]{2}(?:\s+|$))+)(.*)$"
)


def parse_disassembly(
    output: str, *, text_address: int, text_size: int, selected_cpu: str
) -> list[dict[str, Any]]:
    instructions: list[dict[str, Any]] = []
    for line in output.splitlines():
        match = DISASSEMBLY_LINE.match(line)
        if not match:
            continue
        address = int(match.group(1), 16)
        raw = [int(byte, 16) for byte in match.group(2).split()]
        text = match.group(3).strip()
        policy = classify_instruction(raw, selected_cpu)
        instructions.append(
            {
                "address": address,
                "bytes": "".join(f"{byte:02x}" for byte in raw),
                "length": len(raw),
                "text": text,
                **policy,
            }
        )
    if not instructions:
        raise AcceptanceError("disassembler produced no instruction records")
    expected = text_address
    for instruction in instructions:
        if instruction["address"] != expected:
            raise AcceptanceError(
                f"unaccounted executable bytes at 0x{expected:x}; "
                f"next decode is 0x{instruction['address']:x}"
            )
        expected += instruction["length"]
    if expected != text_address + text_size:
        raise AcceptanceError(
            f"decoded {expected - text_address} of {text_size} executable bytes"
        )
    if any("<unknown>" in item["text"] or not item["allowed"] for item in instructions):
        raise AcceptanceError("opcode policy rejected one or more decoded instructions")
    return instructions


def sections_from_readobj(output: str) -> list[dict[str, Any]]:
    payload = json.loads(output)
    if not isinstance(payload, list) or len(payload) != 1:
        raise AcceptanceError("unexpected llvm-readobj JSON structure")
    result = []
    for wrapped in payload[0].get("Sections", []):
        section = wrapped["Section"]
        result.append(
            {
                "address": section["Address"],
                "flags": [item["Name"] for item in section["Flags"].get("Flags", [])],
                "name": section["Name"]["Name"],
                "offset": section["Offset"],
                "size": section["Size"],
                "type": section["Type"]["Name"],
            }
        )
    return result


def select_executable_section(sections: list[dict[str, Any]]) -> dict[str, Any]:
    executable = [
        section
        for section in sections
        if "SHF_ALLOC" in section["flags"]
        and "SHF_EXECINSTR" in section["flags"]
        and section["size"] != 0
    ]
    if len(executable) != 1 or executable[0]["name"] != ".text":
        names = [section["name"] for section in executable]
        raise AcceptanceError(
            "linked probe must contain exactly one executable .text section; "
            f"found {names}"
        )
    return executable[0]


def account_load_image(
    elf: pathlib.Path, com: pathlib.Path, sections: list[dict[str, Any]]
) -> list[dict[str, Any]]:
    elf_bytes = elf.read_bytes()
    com_bytes = com.read_bytes()
    ranges: list[dict[str, Any]] = []
    occupied: list[tuple[int, int]] = []
    for section in sections:
        if "SHF_ALLOC" not in section["flags"] or section["size"] == 0:
            continue
        record = dict(section)
        if section["type"] == "SHT_NOBITS":
            record["accounting"] = "zero-initialized memory; absent from COM bytes"
        else:
            start = section["address"] - 0x100
            end = start + section["size"]
            if start < 0 or end > len(com_bytes):
                raise AcceptanceError(f"allocated section {section['name']} is outside COM")
            elf_start = section["offset"]
            elf_end = elf_start + section["size"]
            if elf_bytes[elf_start:elf_end] != com_bytes[start:end]:
                raise AcceptanceError(f"COM bytes differ from ELF section {section['name']}")
            record["accounting"] = (
                "decoded executable section"
                if "SHF_EXECINSTR" in section["flags"]
                else "non-executable data section"
            )
            record["com_offset"] = start
            record["sha256"] = sha256_bytes(com_bytes[start:end])
            occupied.append((start, end))
        ranges.append(record)
    occupied.sort()
    cursor = 0
    for start, end in occupied:
        if start < cursor:
            raise AcceptanceError("overlapping allocated COM regions")
        if start > cursor:
            ranges.append(
                {
                    "accounting": "linker padding outside executable sections",
                    "com_offset": cursor,
                    "name": "<padding>",
                    "sha256": sha256_bytes(com_bytes[cursor:start]),
                    "size": start - cursor,
                }
            )
        cursor = end
    if cursor < len(com_bytes):
        ranges.append(
            {
                "accounting": "linker padding outside executable sections",
                "com_offset": cursor,
                "name": "<padding>",
                "sha256": sha256_bytes(com_bytes[cursor:]),
                "size": len(com_bytes) - cursor,
            }
        )
    return ranges


def self_test() -> None:
    with tempfile.TemporaryDirectory() as directory:
        output = pathlib.Path(directory) / "nested" / "line-endings.txt"
        write_text(output, "first\nsecond\n")
        assert output.read_bytes() == b"first\nsecond\n"
    for name in ("PROBE.COM", "RESULT.OK", "FAIL.BIN"):
        assert is_dos_83_name(name)
    for name in ("probe-result.ok", "TOO-LONG.COM", "RESULT.LONG"):
        assert not is_dos_83_name(name)
    print("IA16-DOS-NAMES-8.3: PASS")
    sample = """
00000100 <_start>:
 100: 55             pushw %bp
 101: c1 e0 02       shlw $2, %ax
"""
    records = parse_disassembly(
        sample, text_address=0x100, text_size=4, selected_cpu="i80186"
    )
    assert len(records) == 2
    assert records[0]["required_cpu"] == "i8086"
    assert records[1]["required_cpu"] == "i80186"
    assert not classify_instruction([0xC1, 0xE0, 0x02], "i8086")["allowed"]
    assert not classify_instruction([0x66, 0x90], "i80286")["allowed"]
    try:
        parse_disassembly(sample, text_address=0x100, text_size=5, selected_cpu="i80186")
    except AcceptanceError:
        pass
    else:
        raise AssertionError("unaccounted executable bytes were not rejected")
    select_executable_section(
        [{"flags": ["SHF_ALLOC", "SHF_EXECINSTR"], "name": ".text", "size": 1}]
    )
    try:
        select_executable_section(
            [
                {
                    "flags": ["SHF_ALLOC", "SHF_EXECINSTR"],
                    "name": ".text",
                    "size": 1,
                },
                {
                    "flags": ["SHF_ALLOC", "SHF_EXECINSTR"],
                    "name": ".init",
                    "size": 1,
                },
            ]
        )
    except AcceptanceError:
        pass
    else:
        raise AssertionError("additional executable sections were not rejected")
    observed = validate_target_parser_output(
        "\n".join(f"[ RUN      ] {name}" for name in REQUIRED_TARGET_PARSER_TESTS)
    )
    assert observed == list(REQUIRED_TARGET_PARSER_TESTS)
    try:
        validate_target_parser_output("[ RUN      ] TripleTest.ParsedIDs")
    except AcceptanceError:
        pass
    else:
        raise AssertionError("a shrunken TargetParser selection was not rejected")
    print("IA16-FAIL-CLOSED-SELF-TEST: PASS")
    print("IA16-BASELINE-SELF-TEST: PASS")


def discover_test_paths(repo: pathlib.Path) -> dict[str, list[pathlib.Path]]:
    llvm_tests = [
        *sorted((repo / "llvm/test/CodeGen/IA16").glob("*")),
        *sorted((repo / "llvm/test/MC/IA16").glob("*")),
        repo / "llvm/test/tools/llvm-ia16-mz/basic.test",
        repo / "llvm/test/Acceptance/IA16/baseline-harness.test",
    ]
    clang_tests = sorted((repo / "clang/test").glob("**/*ia16*"))
    lld_tests = sorted((repo / "lld/test/ELF").glob("ia16*.s"))
    groups = {"llvm": llvm_tests, "clang": clang_tests, "lld": lld_tests}
    for name, paths in groups.items():
        if not paths or any(not path.exists() for path in paths):
            raise AcceptanceError(f"required {name} IA-16 tests are missing")
    selected = {
        str(path.relative_to(repo)) for paths in groups.values() for path in paths
    }
    missing = REQUIRED_TEST_PATHS - selected
    if missing:
        raise AcceptanceError(
            "required exact IA-16 tests are missing: " + ", ".join(sorted(missing))
        )
    return groups


def validate_target_parser_output(output: str) -> list[str]:
    observed = re.findall(r"^\[ RUN\s+\] (\S+)$", output, flags=re.MULTILINE)
    expected = list(REQUIRED_TARGET_PARSER_TESTS)
    if observed != expected:
        raise AcceptanceError(
            "TargetParser test selection changed; "
            f"expected {expected}, observed {observed}"
        )
    return observed


def tool_identity(
    recorder: Recorder, path: pathlib.Path, version_args: list[str]
) -> dict[str, Any]:
    completed = recorder.run(
        f"identity-{path.name}", [path, *version_args], cwd=recorder.output
    )
    return {
        "exit_code": completed.returncode,
        "path": str(path.resolve()),
        "sha256": sha256_file(path.resolve()),
        "version_log": recorder.commands[-1]["log"],
    }


def manifest_files(output: pathlib.Path) -> None:
    lines = []
    for path in sorted(output.rglob("*")):
        if not path.is_file() or path.name == "MANIFEST.sha256":
            continue
        lines.append(f"{sha256_file(path)}  {path.relative_to(output)}")
    write_text(output / "MANIFEST.sha256", "\n".join(lines) + "\n")


def run_acceptance(args: argparse.Namespace) -> int:
    script = pathlib.Path(__file__).resolve()
    repo = script.parents[2]
    build = args.build_dir.resolve()
    output = args.output_dir.resolve()
    if output.exists():
        raise AcceptanceError(f"output directory already exists: {output}")
    output.mkdir(parents=True)
    recorder = Recorder(output)
    result: dict[str, Any] = {
        "evidence_layers": {},
        "finished_at": None,
        "overall_status": "running",
        "started_at": utc_now(),
    }
    exit_code = 1
    try:
        cache_path = build / "CMakeCache.txt"
        if not cache_path.is_file():
            raise AcceptanceError(f"missing build cache: {cache_path}")
        cache = read_cache(cache_path)
        if pathlib.Path(cache.get("CMAKE_HOME_DIRECTORY", "")).resolve() != repo / "llvm":
            raise AcceptanceError("build cache belongs to a different source tree")

        git_head = recorder.run("source-head", ["git", "rev-parse", "HEAD"], cwd=repo)
        source = (git_head.stdout or "").strip()
        status = recorder.run(
            "source-status", ["git", "status", "--porcelain=v1"], cwd=repo
        )
        if git_head.returncode or status.returncode or status.stdout:
            raise AcceptanceError("acceptance requires a clean full source commit")

        source_identity = {
            "build_cache": {
                "cmake_cache_sha256": sha256_file(cache_path),
                "cmake_home_directory": cache["CMAKE_HOME_DIRECTORY"],
                "configuration": cache.get("CMAKE_BUILD_TYPE"),
                "enabled_projects": cache.get("LLVM_ENABLE_PROJECTS"),
                "targets": cache.get("LLVM_TARGETS_TO_BUILD"),
            },
            "harness": {"path": str(script.relative_to(repo)), "sha256": sha256_file(script)},
            "repository": str(repo),
            "source": source,
        }
        write_json(output / "source-identity.json", source_identity)

        self_test_result = recorder.run(
            "harness-self-test", [sys.executable, script, "--self-test"], cwd=repo
        )
        result["evidence_layers"]["source"] = {
            "status": "pass" if self_test_result.returncode == 0 else "fail",
            "evidence": ["source-identity.json", recorder.commands[-1]["log"]],
        }

        build_result = recorder.run(
            "build-admitted-tools",
            [
                shutil.which("cmake") or "cmake",
                "--build",
                build,
                "--target",
                "clang",
                "lld",
                "llvm-ia16-mz",
                "TargetParserTests",
                "--parallel",
                "2",
            ],
            cwd=repo,
        )
        result["evidence_layers"]["build"] = {
            "status": "pass" if build_result.returncode == 0 else "fail",
            "evidence": [recorder.commands[-1]["log"]],
        }
        if build_result.returncode:
            raise AcceptanceError("admitted tool build failed")

        bin_dir = build / "bin"
        tools = {
            "clang": (bin_dir / "clang", ["--version"]),
            "ld.lld": (bin_dir / "ld.lld", ["--version"]),
            "llvm-ia16-mz": (bin_dir / "llvm-ia16-mz", ["--help"]),
            "llvm-lit": (bin_dir / "llvm-lit", ["--version"]),
            "llvm-objdump": (bin_dir / "llvm-objdump", ["--version"]),
            "llvm-readobj": (bin_dir / "llvm-readobj", ["--version"]),
            "dosbox-x": (args.dosbox.resolve(), ["-version"]),
            "TargetParserTests": (
                build / "unittests/TargetParser/TargetParserTests",
                ["--gtest_list_tests"],
            ),
        }
        identities = {}
        for name, (path, version_args) in tools.items():
            if not path.is_file():
                raise AcceptanceError(f"required tool is missing: {path}")
            identities[name] = tool_identity(recorder, path, version_args)
        write_json(output / "tool-identities.json", identities)

        test_groups = discover_test_paths(repo)
        test_manifest = {}
        test_statuses = []
        test_results = {}
        for group, paths in test_groups.items():
            test_manifest[group] = [
                {
                    "path": str(path.relative_to(repo)),
                    "sha256": sha256_file(path),
                }
                for path in paths
            ]
            completed = recorder.run(
                f"tests-{group}",
                [bin_dir / "llvm-lit", "-j", "2", "-sv", *paths],
                cwd=repo,
            )
            test_statuses.append(completed.returncode == 0)
            test_results[group] = "pass" if completed.returncode == 0 else "fail"
        target_parser = recorder.run(
            "tests-target-parser",
            [
                build / "unittests/TargetParser/TargetParserTests",
                "--gtest_filter=" + ":".join(REQUIRED_TARGET_PARSER_TESTS),
            ],
            cwd=repo,
        )
        try:
            observed_target_parser_tests = validate_target_parser_output(
                target_parser.stdout or ""
            )
            target_parser_selection_ok = True
            target_parser_selection_error = None
        except AcceptanceError as error:
            observed_target_parser_tests = []
            target_parser_selection_ok = False
            target_parser_selection_error = str(error)
        target_parser_ok = (
            target_parser.returncode == 0 and target_parser_selection_ok
        )
        test_statuses.append(target_parser_ok)
        test_results["target-parser"] = (
            "pass" if target_parser_ok else "fail"
        )
        write_json(
            output / "target-parser-selection.json",
            {
                "error": target_parser_selection_error,
                "expected": list(REQUIRED_TARGET_PARSER_TESTS),
                "observed": observed_target_parser_tests,
                "status": "pass" if target_parser_ok else "fail",
            },
        )
        write_json(output / "test-selection.json", test_manifest)
        result["evidence_layers"]["test"] = {
            "status": "pass" if all(test_statuses) else "fail",
            "checks": test_results,
            "evidence": [
                item["log"] for item in recorder.commands if item["name"].startswith("tests-")
            ]
            + ["test-selection.json", "target-parser-selection.json"],
            "explicit_omissions_closed": [
                "lld/test/ELF/ia16-segelf-relocs.s",
                "llvm/unittests/TargetParser/IA16TargetParserTest.cpp",
            ],
        }

        fixtures = output / "fixture"
        fixtures.mkdir()
        write_text(fixtures / "probe.c", PROBE_C)
        write_text(fixtures / "start.S", START_S)
        divhi = repo / "compiler-rt/lib/builtins/ia16/divhi3.S"
        modhi = repo / "compiler-rt/lib/builtins/ia16/modhi3.S"
        fixture_identity = {
            "probe.c": sha256_file(fixtures / "probe.c"),
            "start.S": sha256_file(fixtures / "start.S"),
            str(divhi.relative_to(repo)): sha256_file(divhi),
            str(modhi.relative_to(repo)): sha256_file(modhi),
        }
        write_json(output / "fixture-identity.json", fixture_identity)

        opcode_statuses = []
        runtime_statuses = []
        probe_records = []
        for cpu, cpu_info in CPUS.items():
            for opt in OPT_LEVELS:
                probe_dir = output / "probes" / cpu / opt
                probe_dir.mkdir(parents=True)
                temp_dir = probe_dir / "tmp"
                temp_dir.mkdir()
                env = dict(os.environ)
                env["TMPDIR"] = str(temp_dir)
                common = [
                    bin_dir / "clang",
                    "--target=ia16-pc-dos-elf",
                    f"-march={cpu}",
                    "-mcmodel=tiny",
                    "-ffreestanding",
                    "-fno-builtin",
                    "-nostdlib",
                ]
                compile_jobs = [
                    ("probe", fixtures / "probe.c", probe_dir / "probe.o", f"-{opt}"),
                    ("start", fixtures / "start.S", probe_dir / "start.o", None),
                    ("divhi3", divhi, probe_dir / "divhi3.o", None),
                    ("modhi3", modhi, probe_dir / "modhi3.o", None),
                ]
                compile_ok = True
                for label, source_path, object_path, optimization in compile_jobs:
                    argv = [*common]
                    if optimization:
                        argv.append(optimization)
                    argv.extend(["-c", source_path, "-o", object_path])
                    completed = recorder.run(
                        f"compile-{cpu}-{opt}-{label}", argv, cwd=repo, env=env
                    )
                    compile_ok = compile_ok and completed.returncode == 0
                elf = probe_dir / "probe.elf"
                link = recorder.run(
                    f"link-{cpu}-{opt}",
                    [
                        bin_dir / "ld.lld",
                        "-m",
                        "elf_i386",
                        "--image-base=0",
                        "--emit-relocs",
                        "--gc-sections",
                        "-N",
                        "-Ttext=0x100",
                        "--entry=_start",
                        "-o",
                        elf,
                        probe_dir / "start.o",
                        probe_dir / "probe.o",
                        probe_dir / "divhi3.o",
                        probe_dir / "modhi3.o",
                    ],
                    cwd=repo,
                )
                com = probe_dir / "PROBE.COM"
                pack = recorder.run(
                    f"pack-{cpu}-{opt}",
                    [bin_dir / "llvm-ia16-mz", "--format=com", "-o", com, elf],
                    cwd=repo,
                )
                if not compile_ok or link.returncode or pack.returncode:
                    opcode_statuses.append(False)
                    runtime_statuses.append(False)
                    probe_records.append(
                        {"cpu": cpu, "optimization": opt, "status": "build-failed"}
                    )
                    continue

                dos_dir = probe_dir / "dosrun"
                dos_dir.mkdir()
                shutil.copy2(com, dos_dir / "PROBE.COM")

                readobj = recorder.run(
                    f"sections-{cpu}-{opt}",
                    [
                        bin_dir / "llvm-readobj",
                        "--elf-output-style=JSON",
                        "--sections",
                        elf,
                    ],
                    cwd=repo,
                )
                disassembly = recorder.run(
                    f"disassemble-{cpu}-{opt}",
                    [
                        bin_dir / "llvm-objdump",
                        "-d",
                        "--triple=ia16-pc-dos-elf",
                        f"--mcpu={cpu}",
                        elf,
                    ],
                    cwd=repo,
                )
                try:
                    sections = sections_from_readobj(readobj.stdout or "")
                    text_section = select_executable_section(sections)
                    instructions = parse_disassembly(
                        disassembly.stdout or "",
                        text_address=text_section["address"],
                        text_size=text_section["size"],
                        selected_cpu=cpu,
                    )
                    load_regions = account_load_image(elf, com, sections)
                    opcode_ok = readobj.returncode == 0 and disassembly.returncode == 0
                except (AcceptanceError, json.JSONDecodeError) as error:
                    instructions = []
                    load_regions = []
                    opcode_ok = False
                    write_text(probe_dir / "opcode-error.txt", str(error) + "\n")
                write_json(probe_dir / "opcode-accounting.json", instructions)
                write_json(probe_dir / "load-region-accounting.json", load_regions)
                opcode_statuses.append(opcode_ok)

                config = probe_dir / "dosbox.conf"
                write_text(config, DOSBOX_CONFIG.format(dosbox_cpu=cpu_info["dosbox"]))
                emulator = recorder.run(
                    f"emulator-{cpu}-{opt}",
                    [
                        args.dosbox,
                        "-conf",
                        config,
                        "-nogui",
                        "-exit",
                        "-fastlaunch",
                        "-time-limit",
                        str(args.emulator_timeout),
                        "-c",
                        f"mount c {dos_dir}",
                        "-c",
                        "c:",
                        "-c",
                        "PROBE.COM",
                        "-c",
                        "exit",
                    ],
                    cwd=dos_dir,
                    timeout=args.emulator_timeout + 5,
                )
                success_path = dos_dir / "RESULT.OK"
                fail_path = dos_dir / "FAIL.BIN"
                dos_visible_files = sorted(path.name for path in dos_dir.iterdir())
                dos_names_ok = all(is_dos_83_name(name) for name in dos_visible_files)
                runtime_ok = (
                    emulator.returncode == 0
                    and success_path.is_file()
                    and success_path.read_bytes() == b"OK"
                    and not fail_path.exists()
                    and dos_names_ok
                )
                runtime_statuses.append(runtime_ok)
                probe_records.append(
                    {
                        "artifacts": {
                            "com": {
                                "path": str(com.relative_to(output)),
                                "sha256": sha256_file(com),
                            },
                            "elf": {
                                "path": str(elf.relative_to(output)),
                                "sha256": sha256_file(elf),
                            },
                        },
                        "cpu": cpu,
                        "dosbox_cpu": cpu_info["dosbox"],
                        "dos_visible_files": dos_visible_files,
                        "dos_visible_names_8_3": dos_names_ok,
                        "instruction_count": len(instructions),
                        "opcode_status": "pass" if opcode_ok else "fail",
                        "optimization": opt,
                        "runtime_status": "pass" if runtime_ok else "fail",
                    }
                )

        write_json(output / "probe-results.json", probe_records)
        result["evidence_layers"]["opcode-scan"] = {
            "status": "pass" if opcode_statuses and all(opcode_statuses) else "fail",
            "evidence": [
                "probe-results.json",
                "probes/*/*/opcode-accounting.json",
                "probes/*/*/load-region-accounting.json",
            ],
        }
        result["evidence_layers"]["emulator-runtime"] = {
            "status": "pass" if runtime_statuses and all(runtime_statuses) else "fail",
            "evidence": [
                "probe-results.json",
                "probes/*/*/dosrun/RESULT.OK",
                "logs/*-emulator-*.log",
            ],
        }

        all_pass = all(
            layer["status"] == "pass"
            for layer in result["evidence_layers"].values()
        )
        result["overall_status"] = "pass" if all_pass else "partial"
        exit_code = 0 if all_pass else 1
    except (AcceptanceError, OSError, subprocess.SubprocessError) as error:
        result["overall_status"] = "blocked"
        result["error"] = str(error)
        exit_code = 1
    finally:
        result["finished_at"] = utc_now()
        write_json(output / "result.json", result)
        layers = result["evidence_layers"]
        test_checks = layers.get("test", {}).get("checks", {})
        requirements = [
            {
                "requirement": "clean exact source and verified cache",
                "layer": "source/build",
                "status": layers.get("build", {}).get("status", "not-run"),
            },
            {
                "requirement": "LLVM IA-16 lit selection",
                "layer": "test",
                "status": test_checks.get("llvm", "not-run"),
            },
            {
                "requirement": "Clang IA-16 lit selection",
                "layer": "test",
                "status": test_checks.get("clang", "not-run"),
            },
            {
                "requirement": "LLD including SEGELF relocations",
                "layer": "test",
                "status": test_checks.get("lld", "not-run"),
            },
            {
                "requirement": "IA-16 TargetParser unit tests",
                "layer": "test",
                "status": test_checks.get("target-parser", "not-run"),
            },
            {
                "requirement": "arithmetic, division edges, and bswap at O0/O2/Os",
                "layer": "emulator-runtime",
                "status": layers.get("emulator-runtime", {}).get(
                    "status", "not-run"
                ),
            },
            {
                "requirement": "DOS-mounted runtime surface uses only 8.3 names",
                "layer": "emulator-runtime",
                "status": layers.get("emulator-runtime", {}).get(
                    "status", "not-run"
                ),
            },
            {
                "requirement": "every executable byte and load region accounted per CPU",
                "layer": "opcode-scan",
                "status": layers.get("opcode-scan", {}).get(
                    "status", "not-run"
                ),
            },
        ]
        write_json(output / "requirement-matrix.json", requirements)
        report_lines = [
            "# IA-16 reproducible baseline acceptance",
            "",
            f"Overall disposition: **{result['overall_status']}**",
            "",
            "## Evidence layers",
            "",
            "| Layer | Status |",
            "| --- | --- |",
        ]
        for layer, data in result["evidence_layers"].items():
            report_lines.append(f"| {layer} | {data['status']} |")
        if result["overall_status"] == "pass":
            next_gap = (
                "Complete the Gate 1 per-CPU MC generation and rejection matrix "
                "beyond this admitted probe."
            )
        else:
            next_gap = (
                "Resolve the first retained acceptance failure as one bounded "
                "compiler, linker, runtime, or harness defect task."
            )
        report_lines.extend(
            [
                "",
                "Gate disposition: Gate 1 remains incomplete; this baseline does not "
                "cover the full ABI, MC, SEGELF malformed/overflow, or differential matrix.",
                "",
                "Progress disposition: retained evidence replaces the historical narrow "
                "slice only for the requirements listed in the matrix.",
                "",
                f"Exactly one bounded next gap: {next_gap}",
                "",
                "The result is acceptance evidence only. It is not a release, "
                "integration, deployment, or complete gate claim.",
                "",
                "See `requirement-matrix.json`, `commands.json`, "
                "`tool-identities.json`, and `MANIFEST.sha256` for exact retained "
                "evidence.",
            ]
        )
        write_text(output / "report.md", "\n".join(report_lines) + "\n")
        manifest_files(output)
    return exit_code


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--self-test", action="store_true")
    parser.add_argument("--build-dir", type=pathlib.Path)
    parser.add_argument("--output-dir", type=pathlib.Path)
    parser.add_argument(
        "--dosbox", type=pathlib.Path, default=pathlib.Path("/opt/homebrew/bin/dosbox-x")
    )
    parser.add_argument("--emulator-timeout", type=int, default=20)
    args = parser.parse_args(argv)
    if not args.self_test and (args.build_dir is None or args.output_dir is None):
        parser.error("--build-dir and --output-dir are required")
    return args


def main(argv: list[str]) -> int:
    args = parse_args(argv)
    if args.self_test:
        self_test()
        return 0
    return run_acceptance(args)


if __name__ == "__main__":
    try:
        raise SystemExit(main(sys.argv[1:]))
    except AcceptanceError as error:
        print(f"error: {error}", file=sys.stderr)
        raise SystemExit(1)
