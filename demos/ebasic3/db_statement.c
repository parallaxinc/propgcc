/* db_statement.c - statement parser
 *
 * Copyright (c) 2009 by David Michael Betz.  All rights reserved.
 *
 */

#include <stdlib.h>
#include <string.h>
#include "db_compiler.h"

/* statement handler prototypes */
static void ParseDef(ParseContext *c);
static void ParseEndDef(ParseContext *c);
static void ParseDim(ParseContext *c);
static int ParseVariableDecl(ParseContext *c, char *name, VMVALUE *pSize);
static VMVALUE ParseScalarInitializer(ParseContext *c);
static void ParseArrayInitializers(ParseContext *c, VMVALUE size);
static void ClearArrayInitializers(ParseContext *c, VMVALUE size);
static void ParseImpliedLetOrFunctionCall(ParseContext *c);
static void ParseLet(ParseContext *c);
static void ParseIf(ParseContext *c);
static void ParseElse(ParseContext *c);
static void ParseElseIf(ParseContext *c);
static void ParseEndIf(ParseContext *c);
static void ParseEnd(ParseContext *c);
static void ParseConstantDef(ParseContext *c, char *name);
static void ParseFor(ParseContext *c);
static void ParseNext(ParseContext *c);
static void ParseDo(ParseContext *c);
static void ParseDoWhile(ParseContext *c);
static void ParseDoUntil(ParseContext *c);
static void ParseLoop(ParseContext *c);
static void ParseLoopWhile(ParseContext *c);
static void ParseLoopUntil(ParseContext *c);
static void ParseStop(ParseContext *c);
static void ParseGoto(ParseContext *c);
static void ParseReturn(ParseContext *c);
static void ParsePrint(ParseContext *c);
#ifdef USE_ASM
static void ParseAsm(ParseContext *c);
#endif

/* prototypes */
static void CallHandler(ParseContext *c, int trap, ParseTreeNode *expr);
static void DefineLabel(ParseContext *c, char *name, int offset);
static int ReferenceLabel(ParseContext *c, char *name, int offset);
static void PushBlock(ParseContext *c);
static void PopBlock(ParseContext *c);

/* ParseStatement - parse a statement */
void ParseStatement(ParseContext *c, int tkn)
{
    /* dispatch on the statement keyword */
    switch (tkn) {
    case T_REM:
        /* just a comment so ignore the rest of the line */
        break;
    case T_DEF:
        ParseDef(c);
        break;
    case T_END_DEF:
        ParseEndDef(c);
        break;
    case T_DIM:
        ParseDim(c);
        break;
    case T_LET:
        ParseLet(c);
        break;
    case T_IF:
        ParseIf(c);
        break;
    case T_ELSE:
        ParseElse(c);
        break;
    case T_ELSE_IF:
        ParseElseIf(c);
        break;
    case T_END_IF:
        ParseEndIf(c);
        break;
    case T_END:
        ParseEnd(c);
        break;
    case T_FOR:
        ParseFor(c);
        break;
    case T_NEXT:
        ParseNext(c);
        break;
    case T_DO:
        ParseDo(c);
        break;
    case T_DO_WHILE:
        ParseDoWhile(c);
        break;
    case T_DO_UNTIL:
        ParseDoUntil(c);
        break;
    case T_LOOP:
        ParseLoop(c);
        break;
    case T_LOOP_WHILE:
        ParseLoopWhile(c);
        break;
    case T_LOOP_UNTIL:
        ParseLoopUntil(c);
        break;
    case T_STOP:
        ParseStop(c);
        break;
    case T_GOTO:
        ParseGoto(c);
        break;
    case T_RETURN:
        ParseReturn(c);
        break;
    case T_PRINT:
        ParsePrint(c);
        break;
#ifdef USE_ASM
    case T_ASM:
        ParseAsm(c);
        break;
#endif
    case T_IDENTIFIER:
        if (SkipSpaces(c) == ':') {
            DefineLabel(c, c->token, codeaddr(c));
            break;
        }
        UngetC(c);
    default:
        SaveToken(c, tkn);
        ParseImpliedLetOrFunctionCall(c);
        break;
    }
}

/* ParseDef - parse the 'DEF' statement */
static void ParseDef(ParseContext *c)
{
    char name[MAXTOKEN];
    int tkn;

    /* get the name being defined */
    FRequire(c, T_IDENTIFIER);
    strcpy(name, c->token);

    /* check for a constant definition */
    if ((tkn = GetToken(c)) == '=')
        ParseConstantDef(c, name);

    /* otherwise, assume a function definition */
    else {

        /* save the lookahead token */
        SaveToken(c, tkn);

        /* enter the function name in the global symbol table */
        c->codeSymbol = AddGlobal(c, name, SC_CONSTANT, 0);

        /* start the code under construction */
        StartCode(c, CODE_TYPE_FUNCTION);

        /* get the argument list */
        if ((tkn = GetToken(c)) == '(') {
            if ((tkn = GetToken(c)) != ')') {
                int offset = 0;
                SaveToken(c, tkn);
                do {
                    FRequire(c, T_IDENTIFIER);
                    AddArgument(c, c->token, SC_VARIABLE, offset);
                    ++offset;
                } while ((tkn = GetToken(c)) == ',');
            }
            Require(c, tkn, ')');
        }
        else
            SaveToken(c, tkn);
    }

    FRequire(c, T_EOL);
}

/* ParseEndDef - parse the 'END DEF' statement */
static void ParseEndDef(ParseContext *c)
{
    if (c->codeType != CODE_TYPE_FUNCTION)
        ParseError(c, "not in a function definition");
    c->codeSymbol->value = StoreCode(c);
    fixup(c, c->codeSymbol->fixups, c->codeSymbol->value);
    c->codeSymbol = NULL;
}

/* ParseConstantDef - parse a 'DEF <name> =' statement */
static void ParseConstantDef(ParseContext *c, char *name)
{
    ParseTreeNode *expr;

    /* get the constant value */
    expr = ParseExpr(c);

    /* make sure it's a constant */
    if (!IsIntegerLit(expr))
        ParseError(c, "expecting a constant expression");

    /* add the symbol as a global */
    AddGlobal(c, name, SC_CONSTANT, expr->u.integerLit.value);

    FRequire(c, T_EOL);
}

/* ParseDim - parse the 'DIM' statement */
static void ParseDim(ParseContext *c)
{
    char name[MAXTOKEN];
    VMVALUE value, size = 0;
    int isArray;
    int tkn;

    /* parse variable declarations */
    do {

        /* get variable name */
        isArray = ParseVariableDecl(c, name, &size);

        /* add to the global symbol table if outside a function definition */
        if (c->codeType == CODE_TYPE_MAIN) {
            Symbol *sym;

            /* check for initializers */
            if ((tkn = GetToken(c)) == '=') {
                if (isArray)
                    ParseArrayInitializers(c, size);
                else {
                    VMVALUE *dataPtr = (VMVALUE *)c->codeBuf;
                    if (dataPtr >= (VMVALUE *)c->ctop)
                        ParseError(c, "insufficient image space");
                    *dataPtr = ParseScalarInitializer(c);
                    VM_printf("initializer %d\n", *dataPtr);
                }
            }

            /* no initializers */
            else {
                ClearArrayInitializers(c, isArray ? size : 1);
                SaveToken(c, tkn);
            }

            /* allocate space for the data */
            value = (VMVALUE)c->imageDataFree;
            c->imageDataFree += size;
            
            /* add the symbol to the global symbol table */
            sym = AddGlobal(c, name, SC_VARIABLE, value);
            sym->placed = VMTRUE;
    
            /* reinitialize the code buffer */
            InitCodeBuffer(c);
        }

        /* otherwise, add to the local symbol table */
        else {
            if (isArray)
                ParseError(c, "local arrays are not supported");
            AddLocal(c, name, SC_VARIABLE, c->localOffset);
            ++c->localOffset;
        }

    } while ((tkn = GetToken(c)) == ',');

    Require(c, tkn, T_EOL);
}

/* ParseVariableDecl - parse a variable declaration */
static int ParseVariableDecl(ParseContext *c, char *name, VMVALUE *pSize)
{
    int isArray;
    int tkn;

    /* parse the variable name */
    FRequire(c, T_IDENTIFIER);
    strcpy(name, c->token);

    /* handle arrays */
    if ((tkn = GetToken(c)) == '[') {

        /* check for an array with unspecified size */
        if ((tkn = GetToken(c)) == ']')
            *pSize = 0;

        /* otherwise, parse the array size */
        else {
            ParseTreeNode *expr;

            /* put back the token */
            SaveToken(c, tkn);

            /* get the array size */
            expr = ParseExpr(c);

            /* make sure it's a constant */
            if (!IsIntegerLit(expr) || expr->u.integerLit.value <= 0)
                ParseError(c, "expecting a positive constant expression");
            *pSize = expr->u.integerLit.value;

            /* only one dimension allowed for now */
            FRequire(c, ']');
        }

        /* return an array and its size */
        isArray = VMTRUE;
        return VMTRUE;
    }

    /* not an array */
    else {
        SaveToken(c, tkn);
        isArray = VMFALSE;
        *pSize = 1;
    }

    /* return array indicator */
    return isArray;
}

/* ParseScalarInitializer - parse a scalar initializer */
static VMVALUE ParseScalarInitializer(ParseContext *c)
{
    ParseTreeNode *expr = ParseExpr(c);
    if (!IsIntegerLit(expr))
        ParseError(c, "expecting a constant expression");
    return expr->u.integerLit.value;
}

/* ParseArrayInitializers - parse array initializers */
static void ParseArrayInitializers(ParseContext *c, VMVALUE size)
{
    VMVALUE *dataPtr = (VMVALUE *)c->codeBuf;
    VMVALUE *dataTop = (VMVALUE *)c->ctop;
    int done = VMFALSE;
    int tkn;

    FRequire(c, '{');

    /* handle each line of initializers */
    while (!done) {
        int lineDone = VMFALSE;

        /* look for the first non-blank line */
        while ((tkn = GetToken(c)) == T_EOL) {
            if (!GetLine(c))
                ParseError(c, "unexpected end of file in initializers");
        }

        /* check for the end of the initializers */
        if (tkn == '}')
            break;
        SaveToken(c, tkn);

        /* handle each initializer in the current line */
        while (!lineDone) {
            VMVALUE value;

            /* get a constant expression */
            value = ParseScalarInitializer(c);

            /* check for too many initializers */
            if (--size < 0)
                ParseError(c, "too many initializers");

            /* store the initial value */
            if (dataPtr >= dataTop)
                ParseError(c, "insufficient image space");
            *dataPtr++ = value;

            switch (tkn = GetToken(c)) {
            case T_EOL:
                lineDone = VMTRUE;
                break;
            case '}':
                lineDone = VMTRUE;
                done = VMTRUE;
                break;
            case ',':
                break;
            default:
                ParseError(c, "expecting a comma, right brace or end of line");
                break;
            }

        }
    }
}

/* ClearArrayInitializers - clear the array initializers */
static void ClearArrayInitializers(ParseContext *c, VMVALUE size)
{
    VMVALUE *dataPtr = (VMVALUE *)c->codeBuf;
    VMVALUE *dataTop = (VMVALUE *)c->ctop;
    if (dataPtr + size > dataTop)
        ParseError(c, "insufficient image space");
    memset(dataPtr, 0, size * sizeof(VMVALUE));
}

/* ParseImpliedLetOrFunctionCall - parse an implied let statement or a function call */
static void ParseImpliedLetOrFunctionCall(ParseContext *c)
{
    ParseTreeNode *expr;
    int tkn;
    PVAL pv;
    expr = ParsePrimary(c);
    switch (tkn = GetToken(c)) {
    case '=':
        ParseRValue(c);
        code_lvalue(c, expr, &pv);
        (*pv.fcn)(c, PV_STORE, &pv);
        break;
    default:
        SaveToken(c, tkn);
        code_rvalue(c, expr);
        putcbyte(c, OP_DROP);
        break;
    }
    FRequire(c, T_EOL);
}

/* ParseLet - parse the 'LET' statement */
static void ParseLet(ParseContext *c)
{
    ParseTreeNode *lvalue;
    PVAL pv;
    lvalue = ParsePrimary(c);
    FRequire(c, '=');
    ParseRValue(c);
    code_lvalue(c, lvalue, &pv);
    (*pv.fcn)(c, PV_STORE, &pv);
    FRequire(c, T_EOL);
}

/* ParseIf - parse the 'IF' statement */
static void ParseIf(ParseContext *c)
{
    int tkn;
    ParseRValue(c);
    FRequire(c, T_THEN);
    PushBlock(c);
    c->bptr->type = BLOCK_IF;
    putcbyte(c, OP_BRF);
    c->bptr->u.IfBlock.nxt = putcword(c, 0);
    c->bptr->u.IfBlock.end = 0;
    if ((tkn = GetToken(c)) != T_EOL) {
        ParseStatement(c, tkn);
        fixupbranch(c, c->bptr->u.IfBlock.nxt, codeaddr(c));
        PopBlock(c);
    }
    else
        Require(c, tkn, T_EOL);
}

/* ParseElseIf - parse the 'ELSE IF' statement */
static void ParseElseIf(ParseContext *c)
{
    switch (CurrentBlockType(c)) {
    case BLOCK_IF:
        putcbyte(c, OP_BR);
        c->bptr->u.IfBlock.end = putcword(c, c->bptr->u.IfBlock.end);
        fixupbranch(c, c->bptr->u.IfBlock.nxt, codeaddr(c));
        c->bptr->u.IfBlock.nxt = 0;
        ParseRValue(c);
        FRequire(c, T_THEN);
        putcbyte(c, OP_BRF);
        c->bptr->u.IfBlock.nxt = putcword(c, 0);
        FRequire(c, T_EOL);
        break;
    default:
        ParseError(c, "ELSE IF without a matching IF");
        break;
    }
}

/* ParseElse - parse the 'ELSE' statement */
static void ParseElse(ParseContext *c)
{
    int end;
    switch (CurrentBlockType(c)) {
    case BLOCK_IF:
        putcbyte(c, OP_BR);
        end = putcword(c, c->bptr->u.IfBlock.end);
        fixupbranch(c, c->bptr->u.IfBlock.nxt, codeaddr(c));
        c->bptr->type = BLOCK_ELSE;
        c->bptr->u.ElseBlock.end = end;
        break;
    default:
        ParseError(c, "ELSE without a matching IF");
        break;
    }
    FRequire(c, T_EOL);
}

/* ParseEndIf - parse the 'END IF' statement */
static void ParseEndIf(ParseContext *c)
{
    switch (CurrentBlockType(c)) {
    case BLOCK_IF:
        fixupbranch(c, c->bptr->u.IfBlock.nxt, codeaddr(c));
        fixupbranch(c, c->bptr->u.IfBlock.end, codeaddr(c));
        PopBlock(c);
        break;
    case BLOCK_ELSE:
        fixupbranch(c, c->bptr->u.ElseBlock.end, codeaddr(c));
        PopBlock(c);
        break;
    default:
        ParseError(c, "END IF without a matching IF/ELSE IF/ELSE");
        break;
    }
    FRequire(c, T_EOL);
}


/* ParseEnd - parse the 'END' statement */
static void ParseEnd(ParseContext *c)
{
    putcbyte(c, OP_HALT);
    FRequire(c, T_EOL);
}

/* ParseFor - parse the 'FOR' statement */
static void ParseFor(ParseContext *c)
{
    ParseTreeNode *var, *step;
    int test, body, inst;
    int tkn;
    PVAL pv;

    PushBlock(c);
    c->bptr->type = BLOCK_FOR;

    /* get the control variable */
    FRequire(c, T_IDENTIFIER);
    var = GetSymbolRef(c, c->token);
    code_lvalue(c, var, &pv);
    FRequire(c, '=');

    /* parse the starting value expression */
    ParseRValue(c);

    /* parse the TO expression and generate the loop termination test */
    test = codeaddr(c);
    (*pv.fcn)(c, PV_STORE, &pv);
    (*pv.fcn)(c, PV_LOAD, &pv);
    FRequire(c, T_TO);
    ParseRValue(c);
    putcbyte(c, OP_LE);
    putcbyte(c, OP_BRT);
    body = putcword(c, 0);

    /* branch to the end if the termination test fails */
    putcbyte(c, OP_BR);
    c->bptr->u.ForBlock.end = putcword(c, 0);

    /* update the for variable after an iteration of the loop */
    c->bptr->u.ForBlock.nxt = codeaddr(c);
    (*pv.fcn)(c, PV_LOAD, &pv);

    /* get the STEP expression */
    if ((tkn = GetToken(c)) == T_STEP) {
        step = ParseExpr(c);
        code_rvalue(c, step);
        tkn = GetToken(c);
    }

    /* no step so default to one */
    else {
        putcbyte(c, OP_LIT);
        putcword(c, 1);
    }

    /* generate the increment code */
    putcbyte(c, OP_ADD);
    inst = putcbyte(c, OP_BR);
    putcword(c, test - inst - 1 - sizeof(VMUVALUE));

    /* branch to the loop body */
    fixupbranch(c, body, codeaddr(c));
    Require(c, tkn, T_EOL);
}

/* ParseNext - parse the 'NEXT' statement */
static void ParseNext(ParseContext *c)
{
    int inst;
    switch (CurrentBlockType(c)) {
    case BLOCK_FOR:
        FRequire(c, T_IDENTIFIER);
        GetSymbolRef(c, c->token);
        /* BUG: check to make sure it matches the symbol used in the FOR */
        inst = putcbyte(c, OP_BR);
        putcword(c, c->bptr->u.ForBlock.nxt - inst - 1 - sizeof(VMUVALUE));
        fixupbranch(c, c->bptr->u.ForBlock.end, codeaddr(c));
        PopBlock(c);
        break;
    default:
        ParseError(c, "NEXT without a matching FOR");
        break;
    }
    FRequire(c, T_EOL);
}

/* ParseDo - parse the 'DO' statement */
static void ParseDo(ParseContext *c)
{
    PushBlock(c);
    c->bptr->type = BLOCK_DO;
    c->bptr->u.DoBlock.nxt = codeaddr(c);
    c->bptr->u.DoBlock.end = 0;
    FRequire(c, T_EOL);
}

/* ParseDoWhile - parse the 'DO WHILE' statement */
static void ParseDoWhile(ParseContext *c)
{
    PushBlock(c);
    c->bptr->type = BLOCK_DO;
    c->bptr->u.DoBlock.nxt = codeaddr(c);
    ParseRValue(c);
    putcbyte(c, OP_BRF);
    c->bptr->u.DoBlock.end = putcword(c, 0);
    FRequire(c, T_EOL);
}

/* ParseDoUntil - parse the 'DO UNTIL' statement */
static void ParseDoUntil(ParseContext *c)
{
    PushBlock(c);
    c->bptr->type = BLOCK_DO;
    c->bptr->u.DoBlock.nxt = codeaddr(c);
    ParseRValue(c);
    putcbyte(c, OP_BRT);
    c->bptr->u.DoBlock.end = putcword(c, 0);
    FRequire(c, T_EOL);
}

/* ParseLoop - parse the 'LOOP' statement */
static void ParseLoop(ParseContext *c)
{
    int inst;
    switch (CurrentBlockType(c)) {
    case BLOCK_DO:
        inst = putcbyte(c, OP_BR);
        putcword(c, c->bptr->u.DoBlock.nxt - inst - 1 - sizeof(VMUVALUE));
        fixupbranch(c, c->bptr->u.DoBlock.end, codeaddr(c));
        PopBlock(c);
        break;
    default:
        ParseError(c, "LOOP without a matching DO");
        break;
    }
    FRequire(c, T_EOL);
}

/* ParseLoopWhile - parse the 'LOOP WHILE' statement */
static void ParseLoopWhile(ParseContext *c)
{
    int inst;
    switch (CurrentBlockType(c)) {
    case BLOCK_DO:
        ParseRValue(c);
        inst = putcbyte(c, OP_BRT);
        putcword(c, c->bptr->u.DoBlock.nxt - inst - 1 - sizeof(VMUVALUE));
        fixupbranch(c, c->bptr->u.DoBlock.end, codeaddr(c));
        PopBlock(c);
        break;
    default:
        ParseError(c, "LOOP without a matching DO");
        break;
    }
    FRequire(c, T_EOL);
}

/* ParseLoopUntil - parse the 'LOOP UNTIL' statement */
static void ParseLoopUntil(ParseContext *c)
{
    int inst;
    switch (CurrentBlockType(c)) {
    case BLOCK_DO:
        ParseRValue(c);
        inst = putcbyte(c, OP_BRF);
        putcword(c, c->bptr->u.DoBlock.nxt - inst - 1 - sizeof(VMUVALUE));
        fixupbranch(c, c->bptr->u.DoBlock.end, codeaddr(c));
        PopBlock(c);
        break;
    default:
        ParseError(c, "LOOP without a matching DO");
        break;
    }
    FRequire(c, T_EOL);
}

/* ParseStop - parse the 'STOP' statement */
static void ParseStop(ParseContext *c)
{
    putcbyte(c, OP_HALT);
    FRequire(c, T_EOL);
}

/* ParseGoto - parse the 'GOTO' statement */
static void ParseGoto(ParseContext *c)
{
    FRequire(c, T_IDENTIFIER);
    putcbyte(c, OP_BR);
    putcword(c, ReferenceLabel(c, c->token, codeaddr(c)));
    FRequire(c, T_EOL);
}

/* ParseReturn - parse the 'RETURN' statement */
static void ParseReturn(ParseContext *c)
{
    int tkn;
    if ((tkn = GetToken(c)) == T_EOL) {
        putcbyte(c, OP_LIT);
        putcword(c, 0);
    }
    else {
        SaveToken(c, tkn);
        ParseRValue(c);
        FRequire(c, T_EOL);
    }
    putcbyte(c, OP_RETURN);
}

/* ParsePrint - handle the 'PRINT' statement */
static void ParsePrint(ParseContext *c)
{
    int needNewline = VMTRUE;
    ParseTreeNode *expr;
    int tkn;

    while ((tkn = GetToken(c)) != T_EOL) {
        switch (tkn) {
        case ',':
            needNewline = VMFALSE;
            CallHandler(c, TRAP_PrintTab, NULL);
            break;
        case ';':
            needNewline = VMFALSE;
            break;
        default:
            needNewline = VMTRUE;
            SaveToken(c, tkn);
            expr = ParseExpr(c);
            switch (expr->nodeType) {
            case NodeTypeStringLit:
                CallHandler(c, TRAP_PrintStr, expr);
                break;
            default:
                CallHandler(c, TRAP_PrintInt, expr);
                break;
            }
            break;
        }
    }

    if (needNewline)
        CallHandler(c, TRAP_PrintNL, NULL);
    else
        CallHandler(c, TRAP_PrintFlush, NULL);
}

#ifdef USE_ASM

#include "db_vmdebug.h"

static void Assemble(ParseContext *c, char *name);
static VMVALUE ParseIntegerConstant(ParseContext *c);

/* ParseAsm - parse the 'ASM ... END ASM' statement */
static void ParseAsm(ParseContext *c)
{
    int tkn;
    
    /* check for the end of the 'ASM' statement */
    FRequire(c, T_EOL);
    
    /* parse each assembly instruction */
    for (;;) {
    
        /* get the next line */
        if (!GetLine(c))
            ParseError(c, "unexpected end of file in ASM statement");
        
        /* check for the end of the assembly instructions */
        if ((tkn = GetToken(c)) == T_END_ASM)
            break;
            
        /* check for an empty statement */
        else if (tkn == T_EOL)
            continue;
            
        /* check for an opcode name */
        else if (tkn != T_IDENTIFIER)
            ParseError(c, "expected an assembly instruction opcode");
            
        /* assemble a single instruction */
        Assemble(c, c->token);
    }
    
    /* check for the end of the 'END ASM' statement */
    FRequire(c, T_EOL);
}

/* Assemble - assemble a single line */
static void Assemble(ParseContext *c, char *name)
{
    FLASH_SPACE OTDEF *def;
    
    /* lookup the opcode */
    for (def = OpcodeTable; def->name != NULL; ++def)
        if (strcasecmp(name, def->name) == 0) {
            putcbyte(c, def->code);
            switch (def->fmt) {
            case FMT_NONE:
                break;
            case FMT_BYTE:
            case FMT_SBYTE:
                putcbyte(c, ParseIntegerConstant(c));
                break;
            case FMT_WORD:
                putcword(c, ParseIntegerConstant(c));
                break;
            default:
                ParseError(c, "instruction not currently supported");
                break;
            }
            FRequire(c, T_EOL);
            return;
        }
        
    ParseError(c, "undefined opcode");
}

/* ParseIntegerConstant - parse an integer constant expression */
static VMVALUE ParseIntegerConstant(ParseContext *c)
{
    ParseTreeNode *expr;
    expr = ParseExpr(c);
    if (!IsIntegerLit(expr))
        ParseError(c, "expecting an integer constant expression");
    return expr->u.integerLit.value;
}

#endif

/* CallHandler - compile a call to a runtime print function */
static void CallHandler(ParseContext *c, int trap, ParseTreeNode *expr)
{
    /* compile the argument */
    if (expr)
        code_rvalue(c, expr);
    
    /* compile the function symbol reference */
    putcbyte(c, OP_TRAP);
    putcbyte(c, trap);
}

/* DefineLabel - define a local label */
static void DefineLabel(ParseContext *c, char *name, int offset)
{
    Label *label;

    /* check to see if the label is already in the table */
    for (label = c->labels; label != NULL; label = label->next)
        if (strcasecmp(name, label->name) == 0) {
            if (!label->fixups)
                ParseError(c, "duplicate label: %s", name);
            else {
                fixupbranch(c, label->fixups, offset);
                label->offset = offset;
                label->fixups = 0;
            }
            return;
        }

    /* allocate the label structure */
    label = (Label *)LocalAlloc(c, sizeof(Label) + strlen(name));
    memset(label, 0, sizeof(Label));
    strcpy(label->name, name);
    label->offset = offset;
    label->next = c->labels;
    c->labels = label;
}

/* ReferenceLabel - add a reference to a local label */
static int ReferenceLabel(ParseContext *c, char *name, int offset)
{
    Label *label;

    /* check to see if the label is already in the table */
    for (label = c->labels; label != NULL; label = label->next)
        if (strcasecmp(name, label->name) == 0) {
            int link;
            if (!(link = label->fixups))
                return label->offset - offset - sizeof(VMUVALUE);
            else {
                label->fixups = offset;
                return link;
            }
        }

    /* allocate the label structure */
    label = (Label *)LocalAlloc(c, sizeof(Label) + strlen(name));
    memset(label, 0, sizeof(Label));
    strcpy(label->name, name);
    label->fixups = offset;
    label->next = c->labels;
    c->labels = label;

    /* return zero to terminate the fixup list */
    return 0;
}

/* CheckLabels - check for undefined labels */
void CheckLabels(ParseContext *c)
{
    Label *label, *next;
    for (label = c->labels; label != NULL; label = next) {
        next = label->next;
        if (label->fixups)
            Fatal(c, "undefined label: %s", label->name);
    }
    c->labels = NULL;
}

/* CurrentBlockType - make sure there is a block on the stack */
BlockType  CurrentBlockType(ParseContext *c)
{
    return c->bptr < c->blockBuf ? BLOCK_NONE : c->bptr->type;
}

/* PushBlock - push a block on the block stack */
static void PushBlock(ParseContext *c)
{
    if (++c->bptr >= c->btop)
        Fatal(c, "statements too deeply nested");
}

/* PopBlock - pop a block off the block stack */
static void PopBlock(ParseContext *c)
{
    --c->bptr;
}
