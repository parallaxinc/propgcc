/* db_generate.c - code generation functions
 *
 * Copyright (c) 2009 by David Michael Betz.  All rights reserved.
 *
 */

#include "db_compiler.h"

/* local function prototypes */
static void code_expr(ParseContext *c, ParseTreeNode *expr, PVAL *pv);
static void code_shortcircuit(ParseContext *c, int op, ParseTreeNode *expr, PVAL *pv);
static void code_arrayref(ParseContext *c, ParseTreeNode *expr, PVAL *pv);
static void code_call(ParseContext *c, ParseTreeNode *expr, PVAL *pv);
static void code_index(ParseContext *c, PValOp fcn, PVAL *pv);
static VMVALUE rd_cword(ParseContext *c, VMUVALUE off);
static void wr_cword(ParseContext *c, VMUVALUE off, VMVALUE w);
static void wr_cfloat(ParseContext *c, VMUVALUE off, VMFLOAT f);

/* code_lvalue - generate code for an l-value expression */
void code_lvalue(ParseContext *c, ParseTreeNode *expr, PVAL *pv)
{
    code_expr(c, expr, pv);
    chklvalue(c, pv);
}

/* code_rvalue - generate code for an r-value expression */
void code_rvalue(ParseContext *c, ParseTreeNode *expr)
{
    PVAL pv;
    code_expr(c, expr, &pv);
    rvalue(c, &pv);
}

/* code_expr - generate code for an expression parse tree */
static void code_expr(ParseContext *c, ParseTreeNode *expr, PVAL *pv)
{
    switch (expr->nodeType) {
    case NodeTypeSymbolRef:
        pv->u.hValue = expr->u.symbolRef.symbol;
        pv->fcn = expr->u.symbolRef.fcn;
        break;
    case NodeTypeStringLit:
        putcbyte(c, OP_LITH);
        putcword(c, (VMUVALUE)expr->u.stringLit.string);
        pv->fcn = NULL;
        break;
    case NodeTypeIntegerLit:
        putcbyte(c, OP_LIT);
        putcword(c, expr->u.integerLit.value);
        pv->fcn = NULL;
        break;
    case NodeTypeFloatLit:
        break;
    case NodeTypeFunctionLit:
        putcbyte(c, OP_LIT);
        putcword(c, expr->u.functionLit.offset);
        pv->fcn = NULL;
        break;
    case NodeTypeUnaryOp:
        code_rvalue(c, expr->u.unaryOp.expr);
        putcbyte(c, expr->u.unaryOp.op);
        pv->fcn = NULL;
        break;
    case NodeTypeBinaryOp:
        code_rvalue(c, expr->u.binaryOp.left);
        code_rvalue(c, expr->u.binaryOp.right);
        putcbyte(c, expr->u.binaryOp.op);
        pv->fcn = NULL;
        break;
    case NodeTypeArrayRef:
        code_arrayref(c, expr, pv);
        break;
    case NodeTypeFunctionCall:
        code_call(c, expr, pv);
        break;
    case NodeTypeDisjunction:
        code_shortcircuit(c, OP_BRTSC, expr, pv);
        break;
    case NodeTypeConjunction:
        code_shortcircuit(c, OP_BRFSC, expr, pv);
        break;
    }
}

/* code_shortcircuit - generate code for a conjunction or disjunction of boolean expressions */
static void code_shortcircuit(ParseContext *c, int op, ParseTreeNode *expr, PVAL *pv)
{
    ExprListEntry *entry = expr->u.exprList.exprs.head;
    int end = 0;

    code_rvalue(c, entry->expr);
    entry = entry->next;

    do {
        putcbyte(c, op);
        end = putcword(c, end);
        code_rvalue(c, entry->expr);
    } while ((entry = entry->next) != NULL);

    fixupbranch(c, end, codeaddr(c));

    pv->fcn = NULL;
}

/* code_arrayref - code an array reference */
static void code_arrayref(ParseContext *c, ParseTreeNode *expr, PVAL *pv)
{
    /* code the array */
    code_rvalue(c, expr->u.arrayRef.array);

    /* code the first index */
    code_rvalue(c, expr->u.arrayRef.index);

    /* setup the element type */
    pv->fcn = code_index;
}

/* code_call - code a function call */
static void code_call(ParseContext *c, ParseTreeNode *expr, PVAL *pv)
{
    ExprListEntry *arg;

    /* code each argument expression */
    for (arg = expr->u.functionCall.args.tail; arg != NULL; arg = arg->prev)
        code_rvalue(c, arg->expr);

    /* get the value of the function */
    code_rvalue(c, expr->u.functionCall.fcn);

    /* call the function */
    putcbyte(c, OP_CALL);
    putcbyte(c, expr->u.functionCall.argc);

    /* we've got an rvalue now */
    pv->fcn = NULL;
}

/* rvalue - get the rvalue of a partial expression */
void rvalue(ParseContext *c, PVAL *pv)
{
    if (pv->fcn) {
        (*pv->fcn)(c, PV_LOAD, pv);
        pv->fcn = NULL;
    }
}

/* chklvalue - make sure we've got an lvalue */
void chklvalue(ParseContext *c, PVAL *pv)
{
    if (pv->fcn == NULL)
        ParseError(c,"Expecting an lvalue", NULL);
}

/* code_global - compile a global variable reference */
void code_global(ParseContext *c, PValOp fcn, PVAL *pv)
{
    switch (fcn) {
    case PV_LOAD:
        putcbyte(c, OP_GREF);
        putcword(c, (VMUVALUE)pv->u.hValue);
        break;
    case PV_STORE:
        putcbyte(c, OP_GSET);
        putcword(c, (VMUVALUE)pv->u.hValue);
        break;
    }
}

/* code_local - compile an local reference */
void code_local(ParseContext *c, PValOp fcn, PVAL *pv)
{
    Local *sym = (Local *)GetLocalPtr(pv->u.hValue);
    switch (fcn) {
    case PV_LOAD:
        putcbyte(c, OP_LREF);
        putcbyte(c, sym->offset);
        break;
    case PV_STORE:
        putcbyte(c, OP_LSET);
        putcbyte(c, sym->offset);
        break;
    }
}

/* code_index - compile a vector reference */
static void code_index(ParseContext *c, PValOp fcn, PVAL *pv)
{
    switch (fcn) {
    case PV_LOAD:
        putcbyte(c, OP_VREF);
        break;
    case PV_STORE:
        putcbyte(c, OP_VSET);
        break;
    }
}

/* codeaddr - get the current code address (actually, offset) */
int codeaddr(ParseContext *c)
{
    return (int)(c->cptr - c->codeBuf);
}

/* putcbyte - put a code byte into the code buffer */
int putcbyte(ParseContext *c, int b)
{
    int addr = codeaddr(c);
    if (c->cptr >= c->ctop)
        Fatal(c, "Bytecode buffer overflow");
    *c->cptr++ = b;
    return addr;
}

/* putcword - put a code word into the code buffer */
int putcword(ParseContext *c, VMVALUE w)
{
    int addr = codeaddr(c);
    if (c->cptr + sizeof(VMVALUE) > c->ctop)
        Fatal(c, "Bytecode buffer overflow");
    wr_cword(c, (VMUVALUE)(c->cptr - c->codeBuf), w);
    c->cptr += sizeof(VMVALUE);
    return addr;
}

int putcfloat(ParseContext *c, VMFLOAT f)
{
    int addr = codeaddr(c);
    if (c->cptr + sizeof(VMFLOAT) > c->ctop)
        Fatal(c, "Bytecode buffer overflow");
    wr_cfloat(c, (VMUVALUE)(c->cptr - c->codeBuf), f);
    c->cptr += sizeof(VMFLOAT);
    return addr;
}

/* rd_cword - get a code word from the code buffer */
static VMVALUE rd_cword(ParseContext *c, VMUVALUE off)
{
    uint8_t *p = &c->codeBuf[off] + sizeof(VMVALUE);
    int cnt = sizeof(VMVALUE);
    VMVALUE w = 0;
    while (--cnt >= 0) {
        w <<= 8;
        w |= *--p;
    }
    return w;
}

/* wr_cword - put a word into the code buffer */
static void wr_cword(ParseContext *c, VMUVALUE off, VMVALUE v)
{
    int i;
    union {
        VMVALUE value;
        uint8_t bytes[sizeof(VMVALUE)];
    } u;
    u.value = v;
    for (i = 0; i < sizeof(u.bytes); ++i)
       c->codeBuf[off++] = u.bytes[i];
}

/* wr_cfloat - put a float into the code buffer */
static void wr_cfloat(ParseContext *c, VMUVALUE off, VMFLOAT f)
{
    int i;
    union {
        VMFLOAT value;
        uint8_t bytes[sizeof(VMFLOAT)];
    } u;
    u.value = f;
    for (i = 0; i < sizeof(u.bytes); ++i)
       c->codeBuf[off++] = u.bytes[i];
}

/* merge - merge two reference chains */
int merge(ParseContext *c, VMUVALUE chn, VMUVALUE chn2)
{
    int last, nxt;

    /* if the chain we're adding is empty, just return the original chain */
    if (!chn2)
        return chn;

    /* find the last entry in the new chain */
    last = chn2;
    while (last != 0) {
        if (!(nxt = rd_cword(c, last)))
            break;
        last = nxt;
    }

    /* link the last entry in the new chain to the first entry in the original chain */
    wr_cword(c, last, chn);

    /* return the new chain now linked to the original chain */
    return chn2;
}

/* fixup - fixup a reference chain */
void fixup(ParseContext *c, VMUVALUE chn, VMUVALUE val)
{
    while (chn != 0) {
        int nxt = rd_cword(c, chn);
        wr_cword(c, chn, val);
        chn = nxt;
    }
}

/* fixupbranch - fixup a reference chain */
void fixupbranch(ParseContext *c, VMUVALUE chn, VMUVALUE val)
{
    while (chn != 0) {
        int nxt = rd_cword(c, chn);
        int off = val - (chn + sizeof(VMUVALUE)); /* this assumes all 1+sizeof(VMUVALUE) byte branch instructions */
        wr_cword(c, chn, off);
        chn = nxt;
    }
}
