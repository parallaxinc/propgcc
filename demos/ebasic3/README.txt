Editor commands:

NEW
NEW filename
LOAD
LOAD filename
SAVE
SAVE filename
LIST
RUN

Language syntax:

REM comment

DEF const = expr

DEF function-name
DEF function-name ( arg [ , arg ]... )

END DEF

RETURN expr

dim-statement:

    DIM variable-def [ , variable-def ]...
    
variable-def:

    var [ scalar-initializer ]
    var '[' size ']' [ array-initializer ]
    
scalar-initializer:

    = constant-expr
    
array-initializer:

    = { constant-expr [ , constant-expr ]... }

[LET] var = expr

IF expr

ELSE IF expr

ELSE

END IF

STOP

END

FOR var = start TO end [ STEP inc ]

NEXT var

DO
DO WHILE expr
DO UNTIL expr

LOOP
LOOP WHILE expr
LOOP UNTIL expr

label:

GOTO label

PRINT expr [ ;|, expr ]... [ ; ]

expr AND expr
expr OR expr

expr ^ expr
expr | expr
expr & expr

expr = expr
expr <> expr

expr < expr
expr <= expr
expr >= expr
expr > expr

expr << expr
expr >> expr

expr + expr
expr - expr
expr * expr
expr / expr
expr MOD expr

- expr
~ expr
NOT expr

function ( arg [, arg ]... )
array [ index ]

(expr)
var
integer
"string"

Built-in variables:

clkfreq
par
cnt
ina
inb
outa
outb
dira
dirb
ctra
ctrb
frqa
frqb
phsa
phsb
vcfg
vscl

Built-in Functions:

waitcnt(target)
waitpeq(state, mask)
waitpne(state, mask)


