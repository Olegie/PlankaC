# Type Rules

Type markers attach a family, width, and scale to references and literals.

```text
[:32.16]
[:1.1]
[:L32.0]
[:Q32.0]
[:VEC32.16]
```

## Families

| Family | Accepted Marker Forms |
| --- | --- |
| numeric | `[:bits.scale]` |
| boolean | `[:1.1]` |
| complex | `[:Cbits.scale]`, `[:COMPLEXbits.scale]` |
| list | `[:Lbits.scale]`, `[:LISTbits.scale]` |
| set | `[:Sbits.scale]`, `[:SETbits.scale]` |
| pair | `[:Pbits.scale]`, `[:PAIRbits.scale]` |
| record | `[:Qbits.scale]`, `[:RECbits.scale]`, `[:RECORDbits.scale]` |
| vector | `[:VECbits.scale]`, `[:VEC3bits.scale]`, `[:VECTORbits.scale]` |
| matrix | `[:MATbits.scale]`, `[:MAT4bits.scale]`, `[:MATRIXbits.scale]` |

Two markers are compatible when their families match and any known bit/scale
values match. Empty markers are treated as unknown and do not reject a program.

## Procedure Contracts

A procedure header defines the input and result contract. A call is checked
against the callee:

```text
P50 square (V0[:32.16]) => R0[:32.16]
square(V0[:32.16]) => Z1[:32.16]
```

Argument count and result count must match at runtime. When both sides carry a
type marker, the analyzer rejects incompatible families, widths, or scales.

## Structural Schemas

The analyzer tracks schemas for compound families:

| Shape | Rule |
| --- | --- |
| list/set | pushed element markers must stay compatible |
| pair | left and right sides form a stable pair schema |
| relation | domain/range are inferred from pair lists and checked when pairs are pushed into a relation list |
| record | each field key keeps a stable value family |
| vector/matrix | operations require vector/matrix family markers |

Schema checks are interprocedural where procedure signatures expose enough
marker information. Runtime checks still guard malformed handle values.
When a relation list receives pair handles, the analyzer carries the pair
left/right families into the relation domain/range schema. A later pair with
an incompatible domain or range is rejected during source loading.

## Contracts And Assertions

`REQUIRE`, `ENSURE`, `ASSERT`, and `STOPIF` evaluate boolean expressions.
They are represented in typed IR as contract/control nodes, not as ordinary
arithmetic assignments.
