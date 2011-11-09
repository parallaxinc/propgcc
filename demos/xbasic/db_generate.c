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
        pv->fcn = expr->u.symbolRef.fcn;
        pv->u.val = expr->u.symbolRef.offset;
        break;
    case NodeTypeStringLit:
        putcbyte(c, OP_LIT);
        putcword(c, AddStringRef(expr->u.stringLit.string, codeaddr(c)));
        pv->fcn = NULL;
        break;
    case NodeTypeIntegerLit:
        putcbyte(c, OP_LIT);
        putcword(c, expr->u.integerLit.value);
        pv->fcn = NULL;
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
    ExprListEntry *entry = expr->u.exprList.exprs;
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

    /* get the value of the function */
    code_rvalue(c, expr->u.functionCall.fcn);

    /* code each argument expression */
    for (arg = expr->u.functionCall.args; arg != NULL; arg = arg->next)
        code_rvalue(c, arg->expr);

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
        ParseError(c,"Expecting an lvalue");
}

/* code_global - compile a global variable reference */
void code_global(ParseContext *c, PValOp fcn, PVAL *pv)
{
    switch (fcn) {
    case PV_LOAD:
        putcbyte(c, OP_GREF);
        putcword(c, pv->u.val);
        break;
    case PV_STORE:
        putcbyte(c, OP_GSET);
        putcword(c, pv->u.val);
        break;
    }
}

/* code_local - compile an local reference */
void code_local(ParseContext *c, PValOp fcn, PVAL *pv)
{
    switch (fcn) {
    case PV_LOAD:
        putcbyte(c, OP_LREF);
        putcbyte(c, pv->u.val);
        break;
    case PV_STORE:
        putcbyte(c, OP_LSET);
        putcbyte(c, pv->u.val);
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
int putcword(ParseContext *c, int w)
{
    int addr = codeaddr(c);
    if (c->cptr + 2 > c->ctop)
        Fatal(c, "Bytecode buffer overflow");
    *c->cptr++ = w;
    *c->cptr++ = w >> 8;
    return addr;
}

/* merge - merge two reference chains */
int merge(ParseContext *c, int chn, int chn2)
{
    int last, nxt;

    /* if the chain we're adding is empty, just return the original chain */
    if (!chn2)
        return chn;

    /* find the last entry in the new chain */
    last = chn2;
    while (last != 0) {
        if (!(nxt = (int16_t)(c->codeBuf[last] | (c->codeBuf[last + 1] << 8))))
            break;
        last = nxt;
    }

    /* link the last entry in the new chain to the first entry in the original chain */
    c->codeBuf[last] = chn;
    c->codeBuf[last + 1] = chn >> 8;

    /* return the new chain now linked to the original chain */
    return chn2;
}

/* fixup - fixup a reference chain */
void fixup(ParseContext *c, int chn, int val)
{
    while (chn != 0) {
        int nxt = (int16_t)(c->codeBuf[chn] | (c->codeBuf[chn + 1] << 8));
        c->codeBuf[chn] = val;
        c->codeBuf[chn + 1] = val >> 8;
        chn = nxt;
    }
}

/* fixupbranch - fixup a reference chain */
void fixupbranch(ParseContext *c, int chn, int val)
{
    while (chn != 0) {
        int nxt = (int16_t)(c->codeBuf[chn] | (c->codeBuf[chn + 1] << 8));
        int off = val - (chn + 2); /* this assumes all three byte branch instructions */
        c->codeBuf[chn] = off;
        c->codeBuf[chn + 1] = off >> 8;
        chn = nxt;
    }
}
