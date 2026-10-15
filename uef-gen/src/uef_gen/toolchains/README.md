# UETD-owned host tool selection and provisioning

UETD is the version/source-of-truth for tool IDs, versions, package URLs,
checksums, host support, executable names, license handling, and dependencies.
This package reads those records; it must not embed target compiler versions or
URLs in Python, project YAML, or generated build files.

## File map

| File | Owner boundary | Function headings |
| --- | --- | --- |
| `catalog.py` | Read and validate the paths declared under `api.json -> uhal_api.toolchain_catalogue`. | `_declared_path()` prevents registry path escape; `load_toolchain_catalogue()` validates index/record schemas, IDs, dependency references, and cycles. |
| `profiles.py` | Match one board's `build` or `flash` profile to UETD package records and local install receipts. | `resolve_board_tool_selection()` chooses the board/backend/package for a host; `resolve_executable_map()` validates role-to-path mapping; `validate_build_flash_pair()` checks build artifact and flash input compatibility. |
| `bootstrap.py` | Convert UETD package pins into a safe local package plan and eventually install/verify it. | `detect_host_platform()` normalizes the host; `ToolchainBootstrapper.plan()` dependency-orders exact package selections; `install()`, `verify()`, `_download()`, and `_extract_archive()` contain the detailed safety TODOs. |
| `cli.py` | Human/JSON command surface for inspecting or provisioning packages. | `add_tool_commands()` declares CLI inputs; `handle_tool_command()` lists records or prints a plan. Install is unavailable until the bootstrap implementation and package pins are ready. |

## Make a package usable

1. Complete the matching UETD `manifest/<id>.json` package record for each host
   with an exact upstream version, asset URL, SHA-256, archive/installer kind,
   extraction layout, executable-role map, dependencies, license, and
   interactive-acceptance policy.
2. Keep `status` as `scaffold_only` until the source and integrity metadata have
   been independently checked. A tool listed by `uef-gen tools list` is not
   necessarily installable.
3. Implement safe install/receipt/version verification in `bootstrap.py` and
   board-profile selection in `profiles.py`; keep install paths local to the
   user-selected tools root.
4. Connect the resolved executable map to `../deploy/build_manager.py` and the
   selected flash backend. Record package ID/version and executable identity
   in build/artifact provenance.

`python -m pip install` is appropriate only for packages deliberately modeled
as `python_wheel`. CMake, compiler suites, vendor SDKs, probe drivers, and
flash utilities need their own UETD package records and host-specific install
policy. The registry currently contains scaffold-only records, so it is an
inventory and implementation guide rather than a working tool installer.
