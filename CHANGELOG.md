# Changelog

## [1.1.0] - 2026-05-26

- Added GitHub Actions CI for Windows and Linux.
- Added a Linux Makefile that builds the PlankaC CLI, library, conformance
  runner, API demos, generated C smoke test, generated x86-64 ASM smoke test,
  8086 ASM emission, and graphics exporters.
- Added release workflow packaging for source archives, Windows binaries,
  Linux binaries, and SHA-256 checksums.
- Added release notes and artifact guidance for keeping binaries outside the
  Git tree.
- Added a tutorial for the `.plk -> bytecode -> C/ASM -> native` path.
- Expanded conformance fixtures for PAGE diagnostics, result type checking,
  relation edge cases, chess backend equivalence, and Zuse-style max examples.
- Added an English/German technical report and generated PDF artifact.

## [1.0.0] - 2026-05-20

- Initial public PlankaC release with parser, interpreter, bytecode, generated
  C, x86-64 ASM, 8086 ASM emission, embedding API, Win16/DOS-oriented targets,
  PlankaHost, PlankaGUI, PlankaCube, and documentation set.
