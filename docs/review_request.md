# External Review Request

PlankaC would benefit from review by people who work with programming language
implementation, computing history, compiler backends, or old platform targets.

Suggested issue/discussion text:

```text
I am looking for technical review of PlankaC, a substantial executable
Plankalkuel-profile implementation in C.

The project includes a parser/interpreter, typed IR, bytecode, generated C,
generated x86-64 ASM, 8086/DOS-oriented ASM emission, an embedding API, a
conformance corpus, and source-oriented documentation. It is based on the
published Plankalkuel literature and on the Plankalkuel 2000 implementation
report, but it deliberately avoids claiming to be a complete historical
reconstruction.

Review areas where feedback would be useful:

- fidelity of the supported Plankalkuel-profile notation;
- type/value model boundaries;
- conformance corpus structure;
- generated C/ASM backend contract;
- Win16/DOS platform-build notes;
- documentation clarity for people who want to write `.plk` programs.

The main documentation starts at docs/index.md, and the technical report is in
docs/technical_report.md.
```

The goal is a useful outside comment, not praise. A precise criticism from a
language or history person is more valuable than a generic star.
