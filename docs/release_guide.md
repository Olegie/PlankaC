# Release Guide

This project keeps release artifacts out of Git. A release is made from a clean
tagged commit, and binaries are attached to GitHub Releases by CI.

## Preflight

Run the local verification path before tagging:

```text
build.bat
build\plankac.exe check
build\plankac_conformance.exe
```

On Linux or inside GitHub Actions, the equivalent path is:

```text
make ci
```

The Linux Makefile checks generated ASM emission structurally. The executable
x86-64 ASM smoke runner is verified by the Windows `build.bat` path because
the current generated runner uses the Windows/MinGW x64 ABI.

Before publishing, check:

- `VERSION` matches the intended tag without the leading `v`;
- `CHANGELOG.md` has an entry for the release date;
- `docs/plankalkuel_coverage.md`, `docs/conformance.md`, and
  `docs/infographic.md` match the current tree;
- generated executables and compiler outputs are not staged in Git;
- Windows and Linux CI are green.

## Tagging

Use semantic version tags:

```text
git tag v1.1.0
git push origin v1.1.0
```

The `Release` workflow builds the release bundles, creates a source archive,
generates `SHA256SUMS`, and publishes or updates the GitHub Release for that
tag.

## Binary Policy

Modern Windows and Linux binaries are release assets. Do not commit them.

Win16 and DOS builds are attached only when explicitly built with Open Watcom.
Use the label `experimental platform build` for those assets because the
source tree supports them, but the regular CI matrix validates modern Windows
and Linux.

## Release Notes

Use `docs/release_notes_template.md` as the starting point. Keep the notes
specific: what changed, which commands are verified, which platform artifacts
are included, and which language boundaries remain intentional.
