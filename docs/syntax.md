# Linear Plankalkuel Syntax

Konrad Zuse's original notation was two-dimensional. Later implementations and
explanations often use a linear notation so that programs can be written as
plain text.

This project uses a small educational subset of that linear notation.

## Procedure

```text
P10 add (V0[:32.16], V1[:32.16]) => R0[:32.16]
(V0[:32.16] + V1[:32.16]) => R0[:32.16]
END
```

Meaning:

- `P1` is the procedure number.
- `add` is the procedure name.
- `V0` and `V1` are input variables.
- `R0` is the return variable.
- `[:32.16]` is a simplified fixed-point structure marker.
- `=>` is the linear assignment/result arrow.

## Assignment

```text
(V0[:32.16] + V1[:32.16]) => R0[:32.16]
```

The expression on the left is evaluated and stored in the variable on the
right.

## Guarded Equation

```text
(Z1[:32.16] < V1[:32.16]) => V1[:32.16] => Z1[:32.16]
```

The first expression is a condition. If the condition is true, the value is
assigned to the target. This follows the known linear examples of Plankalkuel
program equations.

## Function Call

```text
add(V0[:32.16], V1[:32.16]) => Z1[:32.16]
multiply(Z1[:32.16], 3[:32.16]) => R0[:32.16]
```

Temporary values use `Z` variables. Results use `R` variables.

## Operators

```text
+   addition
-   subtraction
*   multiplication
/   division
%   modulo
^   power
=   equality
!=  inequality
<   less than
<=  less or equal
&   logical AND for Ja-Nein-Werte
|   logical OR for Ja-Nein-Werte
!   logical NOT for Ja-Nein-Werte
```

## Project Type Markers

```text
[:1.1]       boolean Ja-Nein-Wert
[:32.0]      signed integer value
[:32.16]     fixed-point calculator value
```

These markers are a compact linear equivalent of the historical structure rows.

## Indexed And Structured Values

```text
Z10[0][:32.16]
Z20.point.x[:32.16]
Z40.white.king.file[:32.16]
```

PlankaC stores these as frame-local indexed slots and named field paths.

## Executable Two-Dimensional Rows

```text
P134 two_dimensional_fuller () => R0[:32.16]
 | Z + 1[:32.16] => R
V| 4                0
S| 32.16            32.16
END
```

The expression row is expanded into the same linear form as the rest of the
project. The `V|` row supplies the indices, and the `S|` row supplies the
structure markers.

## Historical Example Shape

The following sample mirrors the well-known linear presentation of a
three-value maximum procedure:

```text
P1 max3 (V0[:8.0], V1[:8.0], V2[:8.0]) => R0[:8.0]
max(V0[:8.0], V1[:8.0]) => Z1[:8.0]
max(Z1[:8.0], V2[:8.0]) => R0[:8.0]
END
```

PlankaC uses the same general procedure/call/result shape for the executable
profile in this repository.
