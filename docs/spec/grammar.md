# Grammar

This grammar describes the accepted `.plk` source profile. Whitespace separates
tokens except inside identifiers, type markers, and numeric literals.

```ebnf
program        = { procedure } ;
procedure      = header , { statement | page } , "END" ;
header         = "P" , number , identifier , "(" , [ arg-list ] , ")" ,
                 "=>" , result-list ;

arg-list       = ref , { "," , ref } ;
result-list    = ref , { "," , ref } ;

statement      = assignment
               | guarded-assignment
               | loop-assignment
               | const-decl
               | assertion
               | requirement
               | ensure
               | stopif
               | predicate-statement ;

assignment     = expression , "=>" , target-list ;
guarded-assignment
               = guard , "=>" , expression , "=>" , target-list ;
loop-assignment
               = "LOOP" , expression , "=>" , expression , "=>" , target-list ;
const-decl     = "CONST" , c-ref , "=" , expression ;
assertion      = "ASSERT" , guard ;
requirement    = "REQUIRE" , guard ;
ensure         = "ENSURE" , guard ;
stopif         = "STOPIF" , guard ;

predicate-statement
               = predicate , "=>" , target-list ;

predicate      = predicate-core
               | "NOT" , predicate ;
predicate-core = ("SELECT" | "COUNT" | "EXISTS" | "FORALL"
                 | "SETSELECT" | "SETCOUNT" | "SETEXISTS" | "SETFORALL"
                 | "DOMAINSELECT" | "DOMAINCOUNT"
                 | "DOMAINEXISTS" | "DOMAINFORALL"
                 | "RANGESELECT" | "RANGECOUNT"
                 | "RANGEEXISTS" | "RANGEFORALL") ,
                 expression , predicate-op , expression ;

page           = "PAGE" , { page-row } , "ENDPAGE" ;
page-row       = [ page-coord ] ,
                 ( expr-row | index-row | type-row | separator-row )
               | blank-row ;
page-coord     = "[" , number , "," , number , "]" ;
expr-row       = "|" , spatial-expression ;
index-row      = "V|" , spatial-index-cells ;
type-row       = "S|" , spatial-type-cells ;
separator-row  = "--" , { "-" } ;

target-list    = ref , { "," , ref } ;
guard          = "(" , expression , ")" | expression ;
expression     = primary , { binary-op , primary } ;
primary        = number | ref | call | "(" , expression , ")" ;
call           = identifier , "(" , [ expression , { "," , expression } ] , ")" ;

ref            = bank , number , { subscript } , { field } , [ type-marker ] ;
c-ref          = "C" , number , { subscript } , { field } , type-marker ;
bank           = "V" | "C" | "Z" | "R" ;
subscript      = "[" , number , [ "," , number ] , "]" ;
field          = "." , identifier ;
type-marker    = "[:" , [ family ] , bits , "." , scale , "]" ;

binary-op      = "+" | "-" | "*" | "/" | "%" | "=" | "!="
               | "<" | "<=" | ">" | ">=" | "&" | "|" ;
predicate-op   = "=" | "!=" | "<" | "<=" | ">" | ">=" ;
identifier     = letter , { letter | digit | "_" } ;
number         = digit , { digit } , [ "." , digit , { digit } ] ;
```

## Procedure Headers

A procedure starts with `P<number>`, a symbolic name, zero or more `V` inputs,
and one or more `R` outputs:

```text
P10 add (V0[:32.16], V1[:32.16]) => R0[:32.16]
```

Procedure numbers and names are both unique within a loaded program. Calls may
use the name or the numbered form.

## PAGE Documents

`PAGE` documents accept several executable expression rows with nearby `V|`
and `S|` rows. The document loader keeps row and column diagnostics before the
rows are expanded into linear statements. A page may contain more than one
table block; blank rows and `--` separator rows split blocks. An expression row
binds to overlapping index/type rows in the same block by coordinate, not
merely to the first rows in the page.

```text
PAGE
V|1   2   0
S|32.16 32.16 32.16
|A + B => R
ENDPAGE
```

Rows may also start with an explicit page coordinate. This lets a fixture or
source generator describe a logical page position without relying on physical
file indentation:

```text
PAGE
[10,20] V|1       2       0
[11,20] S|32.16   32.16   32.16
[12,20] |A + B => R
ENDPAGE
```

The loader first builds a page model with row/column cells, then expands the
expression row through that model. Each single-letter symbol in the expression
row is bound to the nearest index cell and type cell by logical column
position. Symbols before `=>` map to `Z` unless the symbol is already `V`,
`Z`, or `R`. Symbols after `=>` map to `R` unless the symbol is already an
explicit bank symbol.
