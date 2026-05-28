# Backend Contract

The compiler route has four stable layers:

```text
.plk source -> procedure AST -> expression AST -> typed IR -> bytecode/C/ASM/8086 artifact
```

## Procedure AST

The source loader stores each procedure as:

```text
number, name, input contract, result contract, ordered statements
```

This layer still preserves source line numbers for diagnostics.
The implementation materializes that layer in `c/ir/plankac_ast.c`.
The AST classifies statements before typed IR construction: plain evaluation,
calls, guarded evaluation, guarded calls, loops, contracts, `STOPIF`, and
`CONST`.

Each statement also carries expression AST summaries for guard, value, and
target-list text. The expression layer recognizes literals, references, calls,
unary operators, binary operators, predicate forms, grouped expressions, and
target lists. The readable artifact is emitted with:

```text
build\plankac.exe ast build\plankac.ast
```

Backends therefore consume a stream built from the AST boundary instead of each
backend independently classifying source lines.

## Typed IR

The typed IR records one node per statement:

```text
procedure, line, op, guard family, value family, target family,
callee, arity, result count, lowering class, expression node counts
```

Current operation classes:

```text
EVAL
CALL
GUARD_EVAL
GUARD_CALL
LOOP
ASSERT
REQUIRE
ENSURE
STOPIF
CONST
```

The CLI emits the readable IR with:

```text
build\plankac.exe ir build\plankac.ir
```

The CLI emits the backend lowering plan with:

```text
build\plankac.exe lowering build\plankac.lowering
```

The lowering report records the C, x86-64 ASM, and 8086 path selected for each
typed IR node. Scalar expressions use direct paths; compound values use ABI or
runtime entry points with the same procedure contract.

## Bytecode

The bytecode backend serializes the procedure table from AST statement nodes
into a loadable text format. The bytecode runner reloads that artifact and
executes the same procedure contracts as source loading.

## Generated C And x86-64 ASM

The C and x86-64 ASM emitters generate native entry points backed by the
PlankaC runtime library. Scalar procedures lower directly where possible.
Compound procedures use the runtime value and handle API so list, set, record,
relation, chess, vector, and matrix behavior remains identical to the
interpreter.

## 8086 ASM

The 8086 backend emits MASM/TASM-style source with:

| Symbol | Role |
| --- | --- |
| `plankac_8086_bytecode_image` | embedded bytecode image |
| `plankac_8086_compound_proc_table` | procedure table for compound procedures |
| `plankac_8086_list_heap` | 16-bit list heap storage |
| `plankac_8086_pair_heap` | 16-bit pair heap storage |
| `plankac_8086_record_key_heap` | record key heap |
| `plankac_8086_record_value_heap` | record value heap |
| `plankac_8086_board_heap` | chess board heap |
| `plankac_8086_list_new` | list ABI entry |
| `plankac_8086_list_push` | list append ABI entry |
| `plankac_8086_set_add` | set insert ABI entry |
| `plankac_8086_pair_make` | pair ABI entry |
| `plankac_8086_record_set` | record write ABI entry |
| `plankac_8086_relation_select` | relation selection ABI entry |
| `plankac_8086_predicate_where` | predicate comparison ABI entry |
| `plankac_8086_board_piece` | board read ABI entry |
| `plankac_8086_dispatch_compound` | compound dispatch ABI entry |

Direct arithmetic procedures return `AX` as `R0` and `DX` as status. Compound
procedures expose a 16-bit ABI surface and an embedded bytecode image so a DOS
runtime can dispatch the full profile through a narrow, documented boundary.
