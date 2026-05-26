# Error Rules

PlankaC reports errors at the earliest layer that can identify them.

## Source Errors

| Error | Example |
| --- | --- |
| missing procedure header data | `P1 bad` |
| duplicate procedure number or name | two procedures with the same `P` number |
| source outside a procedure | statement before a `P` header |
| missing `END` | file ends inside a procedure |
| stray `END` | `END` without an open procedure |

## Type And Analyzer Errors

| Error | Example |
| --- | --- |
| bad type marker | `[:32]` |
| assignment type mismatch | list assigned to numeric target |
| procedure argument type mismatch | caller marker incompatible with callee marker |
| recursive procedure rejected | direct or indirect recursion |
| list element schema mismatch | pushing incompatible typed elements |
| record field schema mismatch | same field key with incompatible families |

## PAGE Errors

PAGE diagnostics include row and column when the document shape is wrong:

```text
PAGE row 2 column 1: missing V| or S| row
```

The expander also rejects rows where `V|`/`S|` cells do not cover every symbol
in an executable row.

## Runtime Errors

| Error | Trigger |
| --- | --- |
| unknown procedure | call name or number cannot be resolved |
| bad argument count | runtime call arity does not match contract |
| divide by zero | unchecked division by zero |
| assertion failed | `ASSERT` evaluates false |
| contract requirement failed | `REQUIRE` evaluates false |
| bad list/record/pair handle | compound operation receives an invalid handle |
| illegal chess move | checked move application rejects the move |

Checked arithmetic procedures return status values where possible. Fatal runtime
errors stop the current command and return `PLANKAC_ERR`.

## Backend Errors

Backends reject missing output paths, failed writes, failed bytecode reloads,
and native linker failures. The build script treats every backend failure as a
failed build.
