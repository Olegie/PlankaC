# PlankaC Language Specification

This directory is the normative specification for the PlankaC language profile
implemented in this repository. It is written for people who want to read or
write `.plk` files without reverse-engineering the C implementation.

## Documents

| Document | Scope |
| --- | --- |
| `grammar.md` | lexical form, procedure headers, statements, expressions, PAGE documents, and predicate forms |
| `ast.md` | statement AST, bounded expression trees, operation classes, and backend AST boundary |
| `value_model.md` | value banks, tagged values, fixed-point values, handles, compound values, and chess boards |
| `type_rules.md` | type markers, type families, structural schemas, call compatibility, and contracts |
| `execution_rules.md` | procedure calls, frames, assignment, guards, loops, constants, PAGE expansion, and host ABI |
| `errors.md` | parser, analyzer, runtime, conformance, and backend error classes |
| `backend_contract.md` | AST to typed IR, bytecode, generated C, generated x86-64 ASM, 8086 ABI surface, DOS COM bootstrap, and PlankaC DOS runner route |

The public language route is:

```text
.plk source -> source loader -> AST/procedure table -> expression tree -> typed IR -> bytecode/C/ASM backends
```

The interpreter, bytecode writer, typed IR, and generated backends use the same
procedure table, AST statement classes, serialized expression tree shapes, and type markers.
The conformance suite checks valid programs, invalid programs, runtime
behavior, backend equivalence fixtures, and Zuse-style table examples.
