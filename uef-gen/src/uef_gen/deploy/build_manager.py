"""Toolchain selection and firmware build orchestration.

The public model follows Part XIX of the uef-gen architecture specification.
Command construction is intentionally unimplemented: build lists and flags must
be tokenized and validated before any process is launched.
"""

from __future__ import annotations

import shutil
from pathlib import Path

from uef_gen.deploy.models import BuildConfig, BuildResult, Toolchain, ToolchainFamily


class ToolchainNotFoundError(RuntimeError):
    """Raised when no supported compiler installation can be resolved."""


class UnsupportedArchitectureError(ValueError):
    """Raised when the architecture database has no build mapping."""


class BuildManager:
    """Compile, link, convert, and report one generated firmware project."""

    def build(self, config: BuildConfig) -> BuildResult:
        """Run a build only after all paths and toolchain inputs pass validation."""
        # Do not interpret a build-list line with shell splitting. Paths and flags require
        # platform-aware parsing before subprocess calls are safe on Windows.
        if not config.project_dir.is_dir():
            raise ValueError(f"project directory does not exist: {config.project_dir}")
        if not config.linker_script.is_file():
            raise ValueError(f"linker script does not exist: {config.linker_script}")
        required_lists = (
            config.sources_file,
            config.includes_file,
            config.defines_file,
            config.cflags_file,
        )
        missing = [str(path) for path in required_lists if not path.is_file()]
        if missing:
            raise ValueError("required generated build lists are missing: " + ", ".join(missing))
        raise NotImplementedError(
            "TODO(build-pipeline): parse canonical .txt lists as data, resolve paths beneath "
            "project_dir, validate compiler and linker executables, compile each source to "
            "a deterministic object path, link with the selected script, emit ELF/HEX/BIN/map, "
            "parse the size report, enforce verified flash/RAM budgets, and return diagnostics "
            "without invoking a shell."
        )

    def _compile_all(
        self,
        sources: list[Path],
        includes: list[Path],
        defines: list[str],
        cflags: list[str],
        toolchain: Toolchain,
        output_dir: Path,
    ) -> list[Path]:
        """Compile sources separately so failures identify the responsible translation unit."""
        raise NotImplementedError(
            "TODO(build-compile): validate every source/include path, construct argv arrays "
            "for the selected compiler, capture bounded stdout/stderr, preserve per-source "
            "diagnostics, and stop before linking when any object fails."
        )

    def _link(
        self,
        objects: list[Path],
        elf: Path,
        linker_script: Path,
        toolchain: Toolchain,
    ) -> Path:
        """Link objects with the verified target memory map and emit a map file."""
        raise NotImplementedError(
            "TODO(build-link): verify linker-script provenance and memory-region addresses, "
            "pass an explicit map-file destination, preserve section diagnostics, and reject "
            "link output that exceeds the resolved target's memory layout."
        )

    def _objcopy(self, elf: Path, output: Path, image_format: str, toolchain: Toolchain) -> Path:
        """Convert a validated ELF into the requested programming image format."""
        raise NotImplementedError(
            "TODO(build-objcopy): allow only supported output formats, invoke objcopy without "
            "a shell, verify the output exists and is non-empty, and retain ELF section/address "
            "metadata needed by the artifact safety checks."
        )

    def _read_size(self, elf: Path, toolchain: Toolchain) -> dict[str, int]:
        """Return normalized text/data/bss/total byte counts from the toolchain."""
        raise NotImplementedError(
            "TODO(build-size): parse the toolchain's documented size output, reject malformed "
            "or ambiguous sections, and calculate flash/RAM use using the target's section map."
        )


def detect_toolchain(family: ToolchainFamily) -> Toolchain:
    """Resolve required executable paths for a selected toolchain family."""
    command_by_family = {
        ToolchainFamily.ARM_NONE_EABI: "arm-none-eabi-gcc",
        ToolchainFamily.TI_CGT_C2000: "cl2000",
        ToolchainFamily.RISCV_NONE_ELF: "riscv32-none-elf-gcc",
        ToolchainFamily.HOST_GCC: "gcc",
    }
    command = command_by_family.get(family)
    if command is None:
        raise ToolchainNotFoundError(f"no toolchain mapping is registered for {family!r}")
    compiler = shutil.which(command)
    if compiler is None:
        raise ToolchainNotFoundError(
            f"{command} was not found on PATH; configure a verified toolchain installation."
        )
    raise NotImplementedError(
        "TODO(toolchain-discovery): resolve cc/cxx/as/ld/objcopy/size/gdb as one coherent "
        f"installation rooted at {Path(compiler).parent}; reject mixed installations and "
        f"return actionable install guidance. Compiler found: {compiler}"
    )


def toolchain_for_arch(arch_name: str) -> ToolchainFamily:
    """Map architecture metadata to a toolchain family without guessing by chip name."""
    name = arch_name.casefold()
    if "cortex-m" in name:
        return ToolchainFamily.ARM_NONE_EABI
    if "c2000" in name or "c28x" in name:
        return ToolchainFamily.TI_CGT_C2000
    if "riscv" in name or "rv32" in name:
        return ToolchainFamily.RISCV_NONE_ELF
    if "x86" in name or "host" in name:
        return ToolchainFamily.HOST_GCC
    raise UnsupportedArchitectureError(
        f"no toolchain family is defined for architecture {arch_name!r}"
    )
