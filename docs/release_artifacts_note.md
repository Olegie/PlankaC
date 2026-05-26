# Release Artifacts

The Git repository stores source, tests, documentation, and build recipes.
Generated executables, generated compiler output, object files, and toolchain
dumps stay outside Git.

Release assets are the right place for binaries:

- `PlankaC-<tag>-windows-x64.zip` contains the modern Windows command-line
  tools, GUI hosts, graphics exporters, and API demos built by CI.
- `PlankaC-<tag>-linux-x64.tar.gz` contains the Linux command-line tools,
  exporters, and API demos built by CI.
- `PlankaC-<tag>-source.tar.gz` is a clean source archive generated from the
  tagged commit.
- `SHA256SUMS` records checksums for every release asset.

The Win16 and DOS targets are platform builds. They are produced with Open
Watcom using `build-win16.bat` and `build-dos.bat`. When these binaries are
attached to a release, label them explicitly as experimental platform builds:

```text
PlankaC-<tag>-win16-experimental.zip
PlankaC-<tag>-dos-experimental.zip
```

That wording is deliberate. It tells users that the 16-bit targets are real
build targets, while the primary CI contract remains the modern Windows and
Linux toolchain.
