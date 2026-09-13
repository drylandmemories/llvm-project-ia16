#!/usr/bin/env python3
"""Run the IA-16 per-CPU MC generation and rejection matrix.

This harness exercises an existing build.  It does not repair the assembler or
linker.  Positive fixtures are linked only to give opcode scans explicit load
regions; the result is MC/test/opcode-scan evidence, not linker integration.
"""

from __future__ import annotations

import argparse
import datetime
import hashlib
import json
import pathlib
import re
import shutil
import subprocess
import sys
import time
from typing import Any, Iterable


CPUS = {
    "i8086": 0,
    "i8088": 0,
    "i80186": 1,
    "i80188": 1,
    "i80286": 2,
}
CPU_BY_RANK = {0: "i8086", 1: "i80186", 2: "i80286", 3: "post-i80286"}
PREFIXES_8086 = {0x26, 0x2E, 0x36, 0x3E, 0xF0, 0xF2, 0xF3}
PREFIXES_POST_286 = {0x64, 0x65, 0x66, 0x67}
OPCODES_80186 = {
    0x60, 0x61, 0x62, 0x68, 0x69, 0x6A, 0x6B, 0x6C,
    0x6D, 0x6E, 0x6F, 0xC0, 0xC1, 0xC8, 0xC9,
}
OPCODES_80286 = {0x63}
DISASSEMBLY_LINE = re.compile(
    r"^\s*([0-9a-fA-F]+):\s+((?:[0-9a-fA-F]{2}(?:\s+|$))+)(.*)$"
)

BASE_SOURCE = """.text
.globl _start
_start:
  nop
  pushw %ax
  shlw $1, %ax
  imulw %bx
  movw (%bx,%si), %ax
  retw
"""

SOURCE_186 = """  pushw $7
  pushw $4660
  shlw $2, %ax
  pushaw
  popaw
  enter $4, $0
  leave
  imulw $3, %ax, %bx
  imulw $4660, %ax, %bx
"""

SOURCE_286 = """  arpl %ax, %bx
  clts
  larw %ax, %bx
  lgdtw (%bx)
  lidtw (%bx)
  lldtw %ax
  lmsww %ax
  lslw %ax, %bx
  ltrw %ax
  sgdtw (%bx)
  sidtw (%bx)
  sldtw %ax
  smsww %ax
  strw %ax
  verr %ax
  verw %ax
"""

NEGATIVE_CASES = {
    "i8086": {
        "push-immediate": ("pushw $7\n", "immediate push requires an 80186"),
        "shift-immediate": ("shlw $2, %ax\n", "multi-bit immediate shift requires an 80186"),
        "pusha": ("pusha\n", "instruction is not available"),
        "popa": ("popa\n", "instruction is not available"),
        "enter": ("enter $4, $0\n", "instruction is not available"),
        "leave": ("leave\n", "instruction is not available"),
        "imul-immediate": ("imulw $3, %ax, %bx\n", "immediate imul requires an 80186"),
    },
    "i80186": {
        "arpl": ("arpl %ax, %bx\n", "instruction is not available"),
        "clts": ("clts\n", "instruction is not available"),
        "lgdt": ("lgdtw (%bx)\n", "instruction is not available"),
    },
    "i80286": {},
}

POST_286_CASES = {
    "register-eax": ("movl %eax, %ebx\n", "instruction is not available"),
    "address-eax": ("movw (%eax), %bx\n", "register is not available"),
    "segment-fs": ("movw %fs:(%bx), %ax\n", "register is not available"),
    "segment-gs": ("movw %gs:(%bx), %ax\n", "register is not available"),
    "bsf": ("bsfw %ax, %bx\n", "instruction is not available"),
    "shld": ("shldw $2, %ax, %bx\n", "instruction is not available"),
}


class AcceptanceError(RuntimeError):
    pass


def utc_now() -> str:
    return datetime.datetime.now(datetime.timezone.utc).isoformat()


def sha256_file(path: pathlib.Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def write_text(path: pathlib.Path, value: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8", newline="\n") as stream:
        stream.write(value)


def write_json(path: pathlib.Path, value: Any) -> None:
    write_text(path, json.dumps(value, indent=2, sort_keys=True) + "\n")


def read_cache(path: pathlib.Path) -> dict[str, str]:
    result = {}
    for line in path.read_text(encoding="utf-8").splitlines():
        if not line or line.startswith(("#", "//")) or "=" not in line:
            continue
        key_and_type, value = line.split("=", 1)
        result[key_and_type.split(":", 1)[0]] = value
    return result


class Recorder:
    def __init__(self, output: pathlib.Path) -> None:
        self.output = output
        self.logs = output / "logs"
        self.logs.mkdir()
        self.commands: list[dict[str, Any]] = []

    def run(self, name: str, argv: Iterable[object], *, cwd: pathlib.Path,
            input_text: str | None = None) -> subprocess.CompletedProcess[str]:
        command = [str(item) for item in argv]
        sequence = len(self.commands) + 1
        stem = f"{sequence:03d}-{name}"
        started = utc_now()
        start = time.monotonic()
        completed = subprocess.run(
            command, cwd=cwd, input=input_text, stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT, text=True, errors="replace", check=False
        )
        log = self.logs / f"{stem}.log"
        write_text(log, completed.stdout or "")
        record = {
            "argv": command,
            "cwd": str(cwd),
            "duration_seconds": round(time.monotonic() - start, 6),
            "ended_at": utc_now(),
            "exit_code": completed.returncode,
            "input_sha256": hashlib.sha256((input_text or "").encode()).hexdigest(),
            "log": str(log.relative_to(self.output)),
            "log_sha256": sha256_file(log),
            "name": name,
            "started_at": started,
        }
        self.commands.append(record)
        write_json(self.output / "commands.json", self.commands)
        return completed


def classify_instruction(raw: list[int], selected_cpu: str) -> dict[str, Any]:
    if not raw:
        raise AcceptanceError("decoded instruction has no bytes")
    index = 0
    while index < len(raw) and raw[index] in PREFIXES_8086:
        index += 1
    rank = 0
    reason = "8086 base opcode"
    if index == len(raw):
        rank, reason = 3, "prefix-only executable bytes"
    elif raw[index] in PREFIXES_POST_286:
        rank, reason = 3, "FS/GS or operand/address-size prefix"
    elif raw[index] in OPCODES_80186:
        rank, reason = 1, "80186 opcode"
    elif raw[index] in OPCODES_80286:
        rank, reason = 2, "80286 opcode"
    elif raw[index] == 0x0F:
        if index + 1 >= len(raw):
            rank, reason = 3, "truncated 0F escape"
        elif raw[index + 1] in {0x00, 0x01}:
            if index + 2 >= len(raw):
                rank, reason = 3, "truncated 80286 0F group"
            else:
                extension = (raw[index + 2] >> 3) & 7
                allowed = (
                    extension <= 5
                    if raw[index + 1] == 0x00
                    else extension in {0, 1, 2, 3, 4, 6}
                )
                rank, reason = (
                    (2, "80286 0F group opcode")
                    if allowed
                    else (3, "post-286 or unassigned 0F group extension")
                )
        elif raw[index + 1] in {0x02, 0x03} and index + 2 < len(raw):
            rank, reason = 2, "80286 0F opcode"
        elif raw[index + 1] == 0x06:
            rank, reason = 2, "80286 0F opcode"
        else:
            rank, reason = 3, "post-286 0F opcode"
    return {
        "allowed": rank <= CPUS[selected_cpu],
        "reason": reason,
        "required_cpu": CPU_BY_RANK[rank],
        "selected_cpu": selected_cpu,
    }


def parse_disassembly(output: str, *, address: int, size: int,
                      cpu: str) -> list[dict[str, Any]]:
    instructions = []
    for line in output.splitlines():
        match = DISASSEMBLY_LINE.match(line)
        if not match:
            continue
        raw = [int(byte, 16) for byte in match.group(2).split()]
        instructions.append({
            "address": int(match.group(1), 16),
            "bytes": "".join(f"{byte:02x}" for byte in raw),
            "length": len(raw),
            "text": match.group(3).strip(),
            **classify_instruction(raw, cpu),
        })
    expected = address
    for instruction in instructions:
        if instruction["address"] != expected:
            raise AcceptanceError(f"unaccounted executable byte at 0x{expected:x}")
        expected += instruction["length"]
    if not instructions or expected != address + size:
        raise AcceptanceError(f"decoded {expected - address} of {size} bytes")
    if any("<unknown>" in item["text"] or not item["allowed"] for item in instructions):
        raise AcceptanceError("opcode policy rejected an executable instruction")
    return instructions


def elf_layout(output: str) -> tuple[dict[str, Any], list[dict[str, Any]]]:
    payload = json.loads(output)
    if not isinstance(payload, list) or len(payload) != 1:
        raise AcceptanceError("unexpected llvm-readobj JSON")
    wrapped = payload[0]
    executable_sections = []
    for item in wrapped.get("Sections", []):
        section = item["Section"]
        flags = [flag["Name"] for flag in section["Flags"].get("Flags", [])]
        if "SHF_ALLOC" in flags and "SHF_EXECINSTR" in flags and section["Size"]:
            executable_sections.append(section)
    if len(executable_sections) != 1 or executable_sections[0]["Name"]["Name"] != ".text":
        raise AcceptanceError("expected exactly one executable .text section")
    loads = []
    for item in wrapped.get("ProgramHeaders", []):
        segment = item["ProgramHeader"]
        if segment["Type"]["Name"] != "PT_LOAD":
            continue
        flags = [flag["Name"] for flag in segment["Flags"].get("Flags", [])]
        loads.append({
            "address": segment["VirtualAddress"],
            "file_size": segment["FileSize"],
            "flags": flags,
            "memory_size": segment["MemSize"],
            "offset": segment["Offset"],
        })
    executable_loads = [load for load in loads if "PF_X" in load["flags"]]
    text = executable_sections[0]
    if len(executable_loads) != 1:
        raise AcceptanceError("expected exactly one executable PT_LOAD region")
    load = executable_loads[0]
    if load["address"] != text["Address"] or load["file_size"] != text["Size"]:
        raise AcceptanceError("executable load region is not exactly covered by .text")
    return text, loads


def positive_source(cpu: str) -> str:
    result = BASE_SOURCE
    if CPUS[cpu] >= 1:
        result = result.replace("  retw\n", SOURCE_186 + "  retw\n")
    if CPUS[cpu] >= 2:
        result = result.replace("  retw\n", SOURCE_286 + "  retw\n")
    return result


def self_test() -> None:
    assert classify_instruction([0x90], "i8086")["allowed"]
    assert not classify_instruction([0xC1, 0xE0, 0x02], "i8086")["allowed"]
    assert classify_instruction([0xC1, 0xE0, 0x02], "i80186")["allowed"]
    assert not classify_instruction([0x0F, 0x01, 0x17], "i80186")["allowed"]
    assert classify_instruction([0x0F, 0x01, 0x17], "i80286")["allowed"]
    assert not classify_instruction([0x0F, 0x00, 0xF0], "i80286")["allowed"]
    assert not classify_instruction([0x0F, 0x01, 0xE8], "i80286")["allowed"]
    assert not classify_instruction([0x0F, 0x01, 0x38], "i80286")["allowed"]
    for prefix in PREFIXES_POST_286:
        assert not classify_instruction([prefix, 0x90], "i80286")["allowed"]
    sample = "100: 90 nop\n101: c3 retw\n"
    assert len(parse_disassembly(sample, address=0x100, size=2, cpu="i8086")) == 2
    try:
        parse_disassembly(sample, address=0x100, size=3, cpu="i8086")
    except AcceptanceError:
        pass
    else:
        raise AssertionError("unaccounted executable bytes did not fail closed")
    print("IA16-MC-FAIL-CLOSED-SELF-TEST: PASS")
    print("IA16-MC-MATRIX-SELF-TEST: PASS")


def manifest_files(output: pathlib.Path) -> None:
    lines = []
    for path in sorted(output.rglob("*")):
        if path.is_file() and path.name != "MANIFEST.sha256":
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
    result: dict[str, Any] = {"started_at": utc_now(), "status": "running"}
    try:
        cache = read_cache(build / "CMakeCache.txt")
        if pathlib.Path(cache.get("CMAKE_HOME_DIRECTORY", "")).resolve() != repo / "llvm":
            raise AcceptanceError("build cache belongs to a different source tree")
        head = recorder.run("source-head", ["git", "rev-parse", "HEAD"], cwd=repo)
        status = recorder.run("source-status", ["git", "status", "--porcelain=v1"], cwd=repo)
        if head.returncode or status.returncode or status.stdout:
            raise AcceptanceError("acceptance requires a clean full source commit")
        source = (head.stdout or "").strip()
        bin_dir = build / "bin"
        tools = {name: bin_dir / name for name in (
            "ld.lld", "llvm-lit", "llvm-mc", "llvm-objdump", "llvm-readobj"
        )}
        if any(not path.is_file() for path in tools.values()):
            raise AcceptanceError("one or more required build tools are missing")
        write_json(output / "source-identity.json", {
            "build_cache_sha256": sha256_file(build / "CMakeCache.txt"),
            "cmake_home_directory": cache["CMAKE_HOME_DIRECTORY"],
            "harness": str(script.relative_to(repo)),
            "harness_sha256": sha256_file(script),
            "source": source,
        })
        build_result = recorder.run(
            "build-tools",
            [shutil.which("cmake") or "cmake", "--build", build, "--target",
             "llvm-mc", "llvm-objdump", "llvm-readobj", "lld", "--parallel", "2"],
            cwd=repo,
        )
        if build_result.returncode:
            raise AcceptanceError("required tool build failed")
        lit = recorder.run(
            "selected-tests", [tools["llvm-lit"], "-j", "2", "-sv",
                               repo / "llvm/test/MC/IA16",
                               repo / "llvm/test/Acceptance/IA16/mc-generation-harness.test"],
            cwd=repo,
        )
        if lit.returncode:
            raise AcceptanceError("selected IA-16 MC tests failed")

        matrix = []
        for cpu in CPUS:
            cpu_dir = output / "matrix" / cpu
            cpu_dir.mkdir(parents=True)
            source_text = positive_source(cpu)
            source_path = cpu_dir / "POSITIVE.S"
            write_text(source_path, source_text)
            obj = cpu_dir / "POSITIVE.O"
            elf = cpu_dir / "POSITIVE.ELF"
            assembled = recorder.run(
                f"assemble-{cpu}", [tools["llvm-mc"], "-triple=ia16-pc-dos-elf",
                                    f"-mcpu={cpu}", "-filetype=obj", "-o", obj],
                cwd=repo, input_text=source_text,
            )
            if assembled.returncode:
                raise AcceptanceError(f"positive assembly failed for {cpu}")
            linked = recorder.run(
                f"link-fixture-{cpu}", [tools["ld.lld"], "-m", "elf_i386",
                                        "--image-base=0", "-Ttext=0x100",
                                        "--entry=_start", "-o", elf, obj], cwd=repo,
            )
            if linked.returncode:
                raise AcceptanceError(f"fixture link failed for {cpu}")
            layout = recorder.run(
                f"layout-{cpu}", [tools["llvm-readobj"], "--elf-output-style=JSON",
                                  "--sections", "--segments", elf], cwd=repo,
            )
            disassembly = recorder.run(
                f"disassemble-{cpu}", [tools["llvm-objdump"], "-d",
                                       "--triple=ia16-pc-dos-elf", f"--mcpu={cpu}", elf],
                cwd=repo,
            )
            if layout.returncode or disassembly.returncode:
                raise AcceptanceError(f"scan inputs failed for {cpu}")
            text, loads = elf_layout(layout.stdout or "")
            instructions = parse_disassembly(
                disassembly.stdout or "", address=text["Address"], size=text["Size"], cpu=cpu
            )
            record = {
                "cpu": cpu,
                "executable_bytes": text["Size"],
                "instructions": instructions,
                "load_regions": loads,
                "object_sha256": sha256_file(obj),
                "elf_sha256": sha256_file(elf),
                "status": "pass",
            }
            write_json(cpu_dir / "scan.json", record)
            matrix.append(record)

        rejections = []
        for cpu in CPUS:
            family = "i8086" if CPUS[cpu] == 0 else "i80186" if CPUS[cpu] == 1 else "i80286"
            cases = {**NEGATIVE_CASES[family], **POST_286_CASES}
            for name, (source_text, diagnostic) in cases.items():
                completed = recorder.run(
                    f"reject-{cpu}-{name}", [tools["llvm-mc"], "-triple=ia16",
                                             f"-mcpu={cpu}", "-o", "/dev/null"],
                    cwd=repo, input_text=source_text,
                )
                passed = completed.returncode != 0 and diagnostic in (completed.stdout or "")
                rejections.append({"case": name, "cpu": cpu, "diagnostic": diagnostic,
                                   "status": "pass" if passed else "fail"})
                if not passed:
                    raise AcceptanceError(f"negative case {name} did not reject for {cpu}")

        injected = []
        for prefix in sorted(PREFIXES_POST_286):
            policy = classify_instruction([prefix, 0x90], "i80286")
            injected.append({"bytes": f"{prefix:02x}90", **policy})
            if policy["allowed"]:
                raise AcceptanceError(f"post-286 prefix {prefix:02x} was admitted")
        write_json(output / "matrix-summary.json", matrix)
        write_json(output / "rejection-summary.json", rejections)
        write_json(output / "injected-prefix-scan.json", injected)
        result.update({
            "finished_at": utc_now(),
            "source": source,
            "status": "pass",
            "summary": {
                "cpus": len(matrix),
                "executable_bytes": sum(item["executable_bytes"] for item in matrix),
                "instructions": sum(len(item["instructions"]) for item in matrix),
                "load_regions": sum(len(item["load_regions"]) for item in matrix),
                "negative_cases": len(rejections),
                "injected_prefix_cases": len(injected),
            },
        })
        write_json(output / "result.json", result)
        manifest_files(output)
        print("IA16-MC-MATRIX: PASS")
        print(json.dumps(result["summary"], sort_keys=True))
        return 0
    except (AcceptanceError, OSError, subprocess.SubprocessError) as error:
        result.update({"error": str(error), "finished_at": utc_now(), "status": "fail"})
        write_json(output / "result.json", result)
        manifest_files(output)
        print(f"error: {error}", file=sys.stderr)
        return 1


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--self-test", action="store_true")
    parser.add_argument("--build-dir", type=pathlib.Path)
    parser.add_argument("--output-dir", type=pathlib.Path)
    args = parser.parse_args(argv)
    if not args.self_test and (args.build_dir is None or args.output_dir is None):
        parser.error("--build-dir and --output-dir are required without --self-test")
    return args


def main(argv: list[str]) -> int:
    args = parse_args(argv)
    if args.self_test:
        self_test()
        return 0
    try:
        return run_acceptance(args)
    except AcceptanceError as error:
        print(f"error: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
