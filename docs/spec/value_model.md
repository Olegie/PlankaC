# Value Model

PlankaC evaluates procedures inside a frame. A frame contains four named banks:

| Bank | Meaning |
| --- | --- |
| `V` | procedure inputs |
| `C` | constants declared with `CONST` |
| `Z` | local intermediate values |
| `R` | procedure results |

Every bank supports scalar slots, indexed slots, two-dimensional indexed slots,
and field paths. Compound values are represented by typed handles into frame
heaps. The public result ABI still transports numeric values, while the runtime
stores assigned values through active tagged PLC entries beside bank storage.

## Tagged Values

The internal value form is:

```text
tag, family, bits, scale, raw, handle, number
```

| Tag | Use |
| --- | --- |
| `numeric` | unscaled numeric value |
| `bit` | Ja/Nein value, normalized to `0` or `1` |
| `fixed` | raw fixed-point integer plus scale |
| `handle` | reference to list, set, pair, record, complex, vector, matrix, or board storage |
| `exception` | reserved for arithmetic/status values |

The interpreter writes the ordinary numeric bank value and a tagged PLC entry
at the same time. Reads prefer the tagged entry when it exists, then convert
back through the ABI number. That keeps the C ABI simple while giving the
compiler, IR, and analyzers a bit-aware value surface.

## Fixed-Point Values

The marker `[:32.16]` denotes a numeric family with bit width and scale. Raw
fixed-point helpers convert with:

```text
raw = round(value * 2^scale)
value = raw / 2^scale
```

`fixed_add`, `fixed_mul`, and `fixed_div_checked` operate through the raw
helpers before returning the ABI number.

## Compound Values

Compound values are frame-local handles:

| Family | Marker Prefix | Storage |
| --- | --- | --- |
| list | `L` or `LIST` | ordered array of values |
| set | `S` or `SET` | list with uniqueness semantics |
| pair | `P` or `PAIR` | left and right handle/value |
| record | `Q`, `REC`, or `RECORD` | key/value field table |
| complex | `C` or `COMPLEX` | real and imaginary slots |
| vector | `VEC`, `VEC3`, or `VECTOR` | three numeric components |
| matrix | `MAT`, `MAT4`, or `MATRIX` | sixteen numeric components |

Records support nested paths through child records. Lists and sets are used as
the concrete representation for relations: a relation is a list of pair
handles.

## Chess Values

Chess boards use record handles. Board fields are square numbers, and values
are piece codes:

```text
square = file * 10 + rank
piece  = kind * 10 + side
```

The model exposes board placement, legal move checks, legal move counts,
castling path checks, promotion checks, en passant checks, check, mate,
stalemate, material score, capture search, position signatures, and FEN-style
state signatures.
