# Language Reference

PlankaC reads a compact executable profile of linear Plankalkuel notation.
The profile is source-first: procedures are written in `.plk` files, loaded
into a procedure table, analyzed, and executed by name or by `P` number.
For the normative grammar, value model, type rules, execution rules, and error
classes, see `docs/spec/index.md`.

The current loaded profile is:

```text
29 source files
148 procedures
```

## Source Files

A source file contains numbered plans. Lines beginning with `#` are comments.
Blank lines are ignored.

```text
P10 add (V0[:32.16], V1[:32.16]) => R0[:32.16]
(V0[:32.16] + V1[:32.16]) => R0[:32.16]
END
```

## Procedure Header

```text
P<number> name (arguments) => results
```

Example:

```text
P14 divide_checked (V0[:32.16], V1[:32.16]) => R0[:32.16], R1[:1.1]
```

`P<number>` is the plan number. `name` is the callable procedure name.
Arguments use `V` variables. Results use `R` variables. Each reference carries
a structure marker.

Procedures are non-recursive. PlankaC rejects direct and indirect recursion
while loading source.

## Variable Classes

| Class | Meaning |
| --- | --- |
| `V` | input value |
| `C` | constant value |
| `Z` | intermediate value |
| `R` | result value |

The runtime stores `V`, `C`, `Z`, and `R` banks directly. `C` values are
written only through `CONST`; normal assignment to `C` is rejected.

## Structure Markers

Structure markers are written as `[:...]` after every typed reference or
literal.

```text
V0[:32.16]
1[:1.1]
Z5[:L32.0]
R0[:C32.16]
```

Implemented marker families:

| Marker shape | Family |
| --- | --- |
| `[:1.1]` | Boolean / Ja-Nein value |
| `[:32.0]` | integer-like numeric value |
| `[:32.16]` | fixed-point-style numeric value |
| `[:C32.16]` | complex value handle |
| `[:L32.0]` | list handle |
| `[:S32.0]` | set handle |
| `[:P32.0]` | pair handle |
| `[:Q32.0]`, `[:REC32.0]` | record handle |
| `[:VEC32.16]` | 3D vector handle |
| `[:MAT32.16]` | 4x4 matrix handle |

The parser records width and scale pieces and checks type-family
compatibility. Fixed-point helpers use raw scaled integer operations before
returning host-visible numeric results. Assigned values are stored through
active tagged PLC entries for bit, fixed, numeric, and handle families.
Compound values live in typed handle tables for lists, sets, pairs, records,
complex numbers, vectors and matrices.

## Expressions

Supported arithmetic:

```text
+  addition
-  subtraction
*   multiplication
/   division
%   modulo
^   power
```

Supported comparisons:

```text
=   equality
!=  inequality
<   less than
<=  less or equal
>   greater than
>=  greater or equal
```

Supported Boolean operators:

```text
&   AND
|   OR
!   NOT
```

Parentheses may be used for grouping.

Predicate forms can be used as assignment values:

```text
SELECT Z0[:L32.0] > 4[:32.0] => Z1[:L32.0]
COUNT Z0[:L32.0] = 5[:32.0] => Z2[:32.0]
EXISTS Z0[:L32.0] = 9[:32.0] => Z3[:1.1]
FORALL Z1[:L32.0] > 4[:32.0] => Z4[:1.1]
```

Relations also expose library-level predicate and schema helpers:

```text
relation_select_range(Z0[:L32.0], 7[:32.0]) => Z5[:L32.0]
relation_exists_range_equal(Z0[:L32.0], 10[:32.0]) => Z6[:1.1]
relation_forall_domain_greater(Z0[:L32.0], 2[:32.0]) => Z7[:1.1]
relation_signature(Z0[:L32.0]) => Z8[:32.0]
```

## Assignment

```text
expression => target
```

Example:

```text
(V0[:32.16] + V1[:32.16]) => R0[:32.16]
```

## Constants

```text
CONST C0[:32.16] = 41[:32.16]
C0[:32.16] + 1[:32.16] => R0[:32.16]
```

The `C` bank can also be indexed:

```text
CONST C2[1,2][:32.16] = 17[:32.16]
C2[1][2][:32.16] => R0[:32.16]
```

## Guarded Equation

```text
condition => expression => target
```

Example:

```text
(V1[:32.16] != 0[:32.16]) => (V0[:32.16] / V1[:32.16]) => R0[:32.16]
(V1[:32.16] = 0[:32.16]) => 1[:1.1] => R1[:1.1]
```

There is no `else` form. A guarded equation writes only when its condition is
true.

## Procedure Calls

```text
name(arg0, arg1) => target
P73(arg0, arg1) => target
```

Calls can return multiple values:

```text
divide_checked(V0[:32.16], V1[:32.16]) => Z0[:32.16], Z1[:1.1]
```

## Host Native Calls

Registered C functions share the same call syntax as source procedures:

```text
host_mad(V0[:32.16], V1[:32.16]) => R0[:32.16], R1[:1.1]
```

The host registers the function through the C ABI before execution. Arity,
result count, and optional marker strings are part of the registered
signature. See `abi.md`.

## Loops

```text
LOOP count => expression => target
```

The expression is evaluated and assigned repeatedly.

Example:

```text
LOOP V1[:32.0] => (R0[:32.16] + V0[:32.16]) => R0[:32.16]
```

## Assertions

```text
ASSERT (predicate)
REQUIRE (predicate)
ENSURE (predicate)
```

An assertion or contract predicate fails execution when it evaluates to `0`.

## Stop Criteria

```text
STOPIF (predicate)
```

`STOPIF` stops the current procedure when the predicate is true and leaves
already written `R` values intact.

## Indexed And Structured References

Indexed references:

```text
Z10[0][:32.16]
Z10[1][:32.16]
Z10[1,2][:32.16]
Z10[1][2][:32.16]
```

Nested field references:

```text
Z40.white.king.file[:32.0]
Z40.white.king.rank[:32.0]
```

Handle-backed records are available through the standard library:

```text
record_new() => Z1[:Q32.0]
record_set(Z1[:Q32.0], 100[:32.0], 7[:32.0]) => Z1[:Q32.0]
record_get(Z1[:Q32.0], 100[:32.0]) => R0[:32.0]
record_set_path2(Z1[:Q32.0], 10[:32.0], 20[:32.0], 42[:32.0]) => Z1[:Q32.0]
record_get_path2(Z1[:Q32.0], 10[:32.0], 20[:32.0]) => R0[:32.0]
```

## Executable Two-Dimensional Rows And Pages

PlankaC supports executable rows with expression, index, and type rows:

```text
 | Z + 1[:32.16] => R
V| 4                0
S| 32.16            32.16
```

The notation layer first builds a row/cell document model with row and column
coordinates. The row is then expanded to the same linear assignment model.
Aligned executable rows, swapped `V|`/`S|` order, and spaced cell alignment are
accepted.

Several executable rows can also be grouped into a page/table block. Each
expression row is bound to the nearest `V|` and `S|` rows in the block:

```text
PAGE
V|1   2    0
S|32.16 32.16 32.16
|A + B => R
|C * D => R
S|32.16 32.16 32.16
V|3   4    0
ENDPAGE
```

## Application Profiles

An application `.plk` file can be loaded on top of the standard profile:

```text
build\plankac.exe runfile graphics\src\plankagui.plk app_canvas
build\PlankaHost.exe graphics\src\plankacube.plk
```

Graphical application profiles expose:

```text
app_kind
app_canvas
app_checksum
app_timer_step
```

See `plk_application_model.md` and `plankahost_api.md`.
