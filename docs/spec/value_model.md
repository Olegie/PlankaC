# Value Model

PlankaC evaluates procedures inside a frame. A frame contains four named banks:

| Bank | Meaning |
| --- | --- |
| `V` | procedure inputs |
| `C` | constants declared with `CONST` |
| `Z` | local intermediate values |
| `R` | procedure results |

Every bank supports scalar slots, indexed slots, two-dimensional indexed slots,
and field paths. Scalar `V`, `C`, `Z`, and `R` slots have active tagged
`PLC_VALUE` storage; the numeric bank arrays remain as the compatibility ABI.
Indexed and field-path values keep the same tagged shadow table used by earlier
profiles. Compound values are represented by typed handles into frame heaps.
Hosts that need the value model use the typed result API, which returns the
same tag, raw fixed value, handle, bit width, scale, family, and marker text
used internally.

## Tagged Values

The internal value form is:

```text
tag, family, bits, scale, raw, handle, number
```

`raw` is a wide integer storage field so fixed-point values such as `[:32.16]`
can cross procedure boundaries without being truncated by a host platform where
C `long` is only 32 bits.

| Tag | Use |
| --- | --- |
| `numeric` | unscaled numeric value |
| `bit` | Ja/Nein value, normalized to `0` or `1` |
| `fixed` | raw fixed-point integer plus scale |
| `handle` | reference to list, set, pair, record, complex, vector, matrix, or board storage |
| `exception` | reserved for arithmetic/status values |

Procedure arguments are boxed into tagged `V` slots using the header type
contracts before statement execution begins. Result extraction reads tagged
`R` slots first, then falls back to the numeric ABI mirror only when a result
was never written as a tagged value.

The interpreter writes the ordinary numeric bank value and a tagged `PLC_VALUE`
entry at the same time. Scalar bank reads prefer the bank-local tagged slot;
indexed and field reads prefer their tagged shadow entries. Both routes convert
back through the ABI number when a host asks for ordinary numeric results.
`plankac_context_run_typed` and
`plankac_run_typed` expose that value shape through the public C API as
`PLANKAC_TYPED_RESULT`.

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
