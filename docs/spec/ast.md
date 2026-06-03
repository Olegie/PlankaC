# AST Model

PlankaC materializes two AST levels before typed IR construction.

```text
.plk source -> procedure AST -> bounded expression AST trees -> typed IR
```

The procedure AST is the stable statement layer. It preserves procedure
number, procedure name, source line, operation class, guard text, value text,
target text, call shape, and result shape.

The expression AST layer parses the expressions inside those statements into a
bounded tree. The tree is intentionally small and textual enough to inspect,
but it carries real parent/child structure for calls, references, literals,
operators, predicate forms, groups, and target lists. It recognizes:

| Kind | Meaning |
| --- | --- |
| `LITERAL` | numeric or typed numeric literal |
| `REF` | `V`, `C`, `Z`, or `R` bank reference, including indexes, fields, and type markers |
| `CALL` | procedure or native function call |
| `UNARY` | unary `-` or `!` |
| `BINARY` | arithmetic, comparison, or boolean binary operation |
| `PREDICATE` | `SELECT`, `COUNT`, `EXISTS`, `FORALL`, `NOT`, set, domain, or range predicate form |
| `GROUP` | parenthesized expression |
| `TARGET_LIST` | assignment target list |
| `UNKNOWN` | accepted source fragment that is intentionally kept for later diagnostics |

The readable AST artifact records the root kind, node count, maximum depth,
call count, reference count, literal count, predicate count, operator count,
unknown count, and a serialized `tree ...` shape for every guard, value, and
target list.

```text
build\plankac.exe ast build\plankac.ast
```

The AST artifact is intentionally textual. It is not the runtime bytecode.
Its job is to make the compiler boundary inspectable and to keep bytecode,
typed IR, and lowering reports tied to the same parsed statement model.

## Runtime Use

Scalar expression execution uses the bounded expression AST as the first route.
Literals, `V`/`C`/`Z`/`R` references, grouping, unary and binary operators,
procedure calls, predicate nodes, and predicate negation are evaluated from the
tree. The older recursive text parser remains as a compatibility fallback for
source fragments outside the AST execution subset.

The AST artifact remains textual and inspectable; runtime execution uses the
same tree model instead of a separate string classifier.

## Statement Classes

The procedure AST supports these operation classes:

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

`CALL` and `GUARD_CALL` carry a resolved callee name, argument count, and result
count when the target procedure or registered native function is known.

## Backend Contract

The bytecode writer serializes from the AST statement layer. Generated C,
native x86-64 ASM, and 8086 ASM continue to use the same bytecode/procedure
contracts, while typed IR includes expression-node counts, depth, lowering
class, and serialized expression shape for backend inspection. This keeps
backend behavior tied to one parse boundary instead of several independent
string classifiers.
