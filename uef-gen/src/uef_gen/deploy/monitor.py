"""Scaffold for wired V1 firmware output monitoring.

The board's UETD ``debug_console`` record owns framing and transport defaults.
The host chooses the concrete COM/tty path at runtime. Monitoring is read-only:
it must not flash, erase, reset, or toggle DTR/RTS unless a board record and an
explicit future command define that behavior.
"""

from __future__ import annotations
from dataclasses import dataclass
from datetime import datetime, timezone
from typing import Any, Callable

class SerialMonitorError(RuntimeError):
    """Raised for missing console metadata, unavailable serial support, or I/O errors."""

@dataclass(frozen=True)
class ConsoleProfile:
    """Validated serial framing and transport information from one UETD board record."""

    board_id: str
    transport: str
    baud_rate: int
    data_bits: int
    parity: str
    stop_bits: int
    encoding: str
    line_ending: str = "lf"
    dtr: bool = False
    rts: bool = False

    @classmethod
    def from_board_record(cls, record: dict[str, Any]) -> "ConsoleProfile":
        """Read the console block and reject incomplete/unsupported framing.

        TODO(console-profile-validation): validate the board schema version,
        require a sourced console interface, reject unsupported baud/framing
        combinations, and carry the source reference into diagnostics. Do not
        guess defaults for a board whose console record is absent.
        """
        block = record.get("debug_console")
        if not isinstance(block, dict):
            raise SerialMonitorError(f"UETD board {record.get('id', '<unknown>')!r} has no debug_console record")
        required = ("transport", "baud_rate", "data_bits", "parity", "stop_bits", "encoding")
        missing = [key for key in required if key not in block]
        if missing:
            raise SerialMonitorError(f"UETD board {record.get('id', '<unknown>')!r} debug_console is missing: " + ", ".join(missing))
        return cls(
            board_id=str(record["id"]),
            transport=str(block["transport"]),
            baud_rate=int(block["baud_rate"]),
            data_bits=int(block["data_bits"]),
            parity=str(block["parity"]),
            stop_bits=int(block["stop_bits"]),
            encoding=str(block["encoding"]),
            line_ending=str(block.get("line_ending", "lf")),
            dtr=bool(block.get("dtr", False)),
            rts=bool(block.get("rts", False)),
        )

@dataclass(frozen=True)
class ConsoleLine:
    """One decoded line plus the UTC time it was received by the host."""

    received_at: datetime
    text: str
    raw: bytes


LineCallback = Callable[[ConsoleLine], None]

class SerialMonitor:
    """Read a wired board console using the board's UETD metadata."""

    def __init__(self, profile: ConsoleProfile):
        self.profile = profile
        self._serial: Any | None = None

    @staticmethod
    def enumerate_ports() -> list[dict[str, str]]:
        """List host serial endpoints without opening them or changing board state.

        TODO(console-enumerate): import pyserial lazily; return stable port,
        VID/PID, serial number, and description fields where the host exposes
        them; tolerate inaccessible devices; and never claim that a port's USB
        descriptor proves the MCU identity.
        """
        raise NotImplementedError("TODO(console-enumerate): serial endpoint listing is not implemented.")

    def connect(self, port: str) -> None:
        """Open only the selected wired endpoint with board-declared framing.

        TODO(console-connect): require a non-empty user-selected port; lazily
        require pyserial; apply baud/data/parity/stop settings from the profile;
        set a finite read timeout; prevent implicit reset by keeping DTR/RTS at
        the board-declared safe values; and close partial handles on every
        exception. Reject wireless transports in V1.
        """
        raise NotImplementedError("TODO(console-connect): serial connection is not implemented.")

    def read_lines(self, callback: LineCallback, *, max_lines: int = 256) -> int:
        """Read a bounded batch, decode it, and deliver lines to the caller.

        TODO(console-read-lines): cap bytes and lines per call; preserve partial
        UTF-8 sequences between calls; honor declared line endings; replace or
        report malformed bytes without dropping the raw payload; and ensure a
        slow callback cannot create an unbounded queue or block device safety
        operations.
        """
        raise NotImplementedError("TODO(console-read-lines): bounded serial reads are not implemented.")

    def run(self, port: str, callback: LineCallback, stop_requested: Callable[[], bool]) -> None:
        """Connect, stream until cancellation, and always close the serial handle.

        TODO(console-run): report connection/read/disconnect diagnostics as
        structured events; poll the stop condition between bounded reads; treat
        unplug as a terminal error; and do not reconnect/reset/flash implicitly.
        """
        raise NotImplementedError("TODO(console-run): monitor lifecycle is not implemented.")

    def disconnect(self) -> None:
        """Close only a serial handle opened by this instance; make cleanup idempotent."""
        raise NotImplementedError("TODO(console-disconnect): cancel pending read, close owned handle, and preserve the original I/O diagnostic.")

    @staticmethod
    def _line(text: str, raw: bytes) -> ConsoleLine:
        """Attach a timezone-aware receive timestamp to one already-decoded line."""
        return ConsoleLine(datetime.now(timezone.utc), text, raw)
