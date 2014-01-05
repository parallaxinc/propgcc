/* db_expr.c - expression parser
 *
 * Copyright (c) 2009-2012 by David Michael Betz.  All rights reserved.
 *
 */

#include <stdlib.h>
#include "db_compiler.h"

/* local function prototypes */
static ParseTreeNode *ParseExpr2(ParseContext *c);
static ParseTreeNode *ParseExpr3(ParseContext *c);
static ParseTreeNode *ParseExpr4(ParseContext *c);
static ParseTreeNode *ParseExpr5(ParseContext *c);
static ParseTreeNode *ParseExpr6(ParseContext *c);
static ParseTreeNode *ParseExpr7(ParseContext *c);
static ParseTreeNode *ParseExpr8(ParseContext *c);
static ParseTreeNode *ParseExpr9(ParseContext *c);
static ParseTreeNode *ParseExpr10(ParseContext *c);
static ParseTreeNode *ParseExpr11(ParseContext *c);
static ParseTreeNode *ParseSimplePrimary(ParseContext *c);
static ParseTreeNode *ParseArrayReference(ParseContext *c, ParseTreeNode *arrayNode);
static ParseTreeNode *ParseCall(ParseContext *c, ParseTreeNode *functionNode);
static ParseTreeNode *MakeUnaryOpNode(ParseContext *c, int op, ParseTreeNode *expr);
static ParseTreeNode *MakeBinaryOpNode(ParseContext *c, int op, ParseTreeNode *left, ParseTreeNode *right);
static ParseTreeNode *NewParseTreeNode(ParseContext *c, VMHANDLE type, int nodeType);
static void AddExprToList(ParseContext *c, ExprList *list, ParseTreeNode *expr);

/* ParseRValue - parse and generate code for an r-value */
void ParseRValue(ParseContext *c)
{
    ParseTreeNode *expr;
    expr = ParseExpr(c);
    code_rvalue(c, expr);
}

/* ParseExpr - handle the OR operator */
ParseTreeNode *ParseExpr(ParseContext *c)
{
    ParseTreeNode *node;
    Token tkn;
    node = ParseExpr2(c);
    if ((tkn = GetToken(c)) == T_OR) {
        ParseTreeNode *node2 = NewParseTreeNode(c, NULL, NodeTypeDisjunction);
        ExprList *list = &node2->u.exprList.exprs;
        list->head = list->tail = NULL;
        AddExprToList(c, list, node);
        do {
            AddExprToList(c, list, ParseExpr2(c));
        } while ((tkn = GetToken(c)) == T_OR);
        node = node2;
    }
    SaveToken(c, tkn);
    return node;
}

/* ParseExpr2 - handle the AND operator */
static ParseTreeNode *ParseExpr2(ParseContext *c)
{
    ParseTreeNode *node;
    Token tkn;
    node = ParseExpr3(c);
    if ((tkn = GetToken(c)) == T_AND) {
        ParseTreeNode *node2 = NewParseTreeNode(c, NULL, NodeTypeConjunction);
        ExprList *list = &node2->u.exprList.exprs;
        list->head = list->tail = NULL;
        AddExprToList(c, list, node);
        do {
            AddExprToList(c, list, ParseExpr3(c));
        } while ((tkn = GetToken(c)) == T_AND);
        node = node2;
    }
    SaveToken(c, tkn);
    return node;
}

/* ParseExpr3 - handle the BXOR operator */
static ParseTreeNode *ParseExpr3(ParseContext *c)
{
    ParseTreeNode *expr, *expr2;
    Token tkn;
    expr = ParseExpr4(c);
    while ((tkn = GetToken(c)) == '^') {
        expr2 = ParseExpr4(c);
        if (IsIntegerLit(expr) && IsIntegerLit(expr2))
            expr->u.integerLit.value = expr->u.integerLit.value ^ expr2->u.integerLit.value;
        else
            expr = MakeBinaryOpNode(c, OP_BXOR, expr, expr2);
    }
    SaveToken(c,tkn);
    return expr;
}

/* ParseExpr4 - handle the BOR operator */
static ParseTreeNode *ParseExpr4(ParseContext *c)
{
    ParseTreeNode *expr, *expr2;
    Token tkn;
    expr = ParseExpr5(c);
    while ((tkn = GetToken(c)) == '|') {
        expr2 = ParseExpr5(c);
        if (IsIntegerLit(expr) && IsIntegerLit(expr2))
            expr->u.integerLit.value = expr->u.integerLit.value | expr2->u.integerLit.value;
        else
            expr = MakeBinaryOpNode(c, OP_BOR, expr, expr2);
    }
    SaveToken(c,tkn);
    return expr;
}

/* ParseExpr5 - handle the BAND operator */
static ParseTreeNode *ParseExpr5(ParseContext *c)
{
    ParseTreeNode *expr, *expr2;
    Token tkn;
    expr = ParseExpr6(c);
    while ((tkn = GetToken(c)) == '&') {
        expr2 = ParseExpr6(c);
        if (IsIntegerLit(expr) && IsIntegerLit(expr2))
            expr->u.integerLit.value = expr->u.integerLit.value & expr2->u.integerLit.value;
        else
            expr = MakeBinaryOpNode(c, OP_BAND, expr, expr2);
    }
    SaveToken(c,tkn);
    return expr;
}

/* ParseExpr6 - handle the '=' and '<>' operators */
static ParseTreeNode *ParseExpr6(ParseContext *c)
{
    ParseTreeNode *expr, *expr2;
    Token tkn;
    expr = ParseExpr7(c);
    while ((tkn = GetToken(c)) == '=' || tkn == T_NE) {
        int op;
        expr2 = ParseExpr7(c);
        switch ((int)tkn) {
        case '=':
            op = OP_EQ;
            break;
        case T_NE:
            op = OP_NE;
            break;
        default:
            /* not reached */
            op = 0;
            break;
        }
        expr = MakeBinaryOpNode(c, op, expr, expr2);
    }
    SaveToken(c,tkn);
    return expr;
}

/* ParseExpr7 - handle the '<', '<=', '>=' and '>' operators */
static ParseTreeNode *ParseExpr7(ParseContext *c)
{
    ParseTreeNode *expr, *expr2;
    Token tkn;
    expr = ParseExpr8(c);
    while ((tkn = GetToken(c)) == '<' || tkn == T_LE || tkn == T_GE || tkn == '>') {
        int op;
        expr2 = ParseExpr8(c);
        switch ((int)tkn) {
        case '<':
            op = OP_LT;
            break;
        case T_LE:
            op = OP_LE;
            break;
        case T_GE:
            op = OP_GE;
            break;
        case '>':
            op = OP_GT;
            break;
        default:
            /* not reached */
            op = 0;
            break;
        }
        expr = MakeBinaryOpNode(c, op, expr, expr2);
    }
    SaveToken(c,tkn);
    return expr;
}

/* ParseExpr8 - handle the '<<' and '>>' operators */
static ParseTreeNode *ParseExpr8(ParseContext *c)
{
    ParseTreeNode *expr, *expr2;
    Token tkn;
    expr = ParseExpr9(c);
    while ((tkn = GetToken(c)) == T_SHL || tkn == T_SHR) {
        expr2 = ParseExpr9(c);
        if (IsIntegerLit(expr) && IsIntegerLit(expr2)) {
            switch (tkn) {
            case T_SHL:
                expr->u.integerLit.value = expr->u.integerLit.value << expr2->u.integerLit.value;
                break;
            case T_SHR:
                expr->u.integerLit.value = expr->u.integerLit.value >> expr2->u.integerLit.value;
                break;
            default:
                /* not reached */
                break;
            }
        }
        else {
            int op;
            switch (tkn) {
            case T_SHL:
                op = OP_SHL;
                break;
            case T_SHR:
                op = OP_SHR;
                break;
            default:
                /* not reached */
                op = 0;
                break;
            }
            expr = MakeBinaryOpNode(c, op, expr, expr2);
        }
    }
    SaveToken(c,tkn);
    return expr;
}

/* ParseExpr9 - handle the '+' and '-' operators */
static ParseTreeNode *ParseExpr9(ParseContext *c)
{
    ParseTreeNode *expr, *expr2;
    Token tkn;
    expr = ParseExpr10(c);
    while ((tkn = GetToken(c)) == '+' || tkn == '-') {
        expr2 = ParseExpr10(c);
        if (IsIntegerLit(expr) && IsIntegerLit(expr2)) {
            switch ((int)tkn) {
            case '+':
                expr->u.integerLit.value = expr->u.integerLit.value + expr2->u.integerLit.value;
                break;
            case '-':
                expr->u.integerLit.value = expr->u.integerLit.value - expr2->u.integerLit.value;
                break;
            default:
                /* not reached */
                break;
            }
        }
        else {
            int op;
            switch ((int)tkn) {
            case '+':
                op = expr->type == CommonType(c->heap, stringType) ? OP_CAT : OP_ADD;
                break;
            case '-':
                op = OP_SUB;
                break;
            default:
                /* not reached */
                op = 0;
                break;
            }
            expr = MakeBinaryOpNode(c, op, expr, expr2);
        }
    }
    SaveToken(c, tkn);
    return expr;
}

/* ParseExpr10 - handle the '*', '/' and MOD operators */
static ParseTreeNode *ParseExpr10(ParseContext *c)
{
    ParseTreeNode *node, *node2;
    Token tkn;
    node = ParseExpr11(c);
    while ((tkn = GetToken(c)) == '*' || tkn == '/' || tkn == T_MOD) {
        node2 = ParseExpr11(c);
        if (IsIntegerLit(node) && IsIntegerLit(node2)) {
            switch ((int)tkn) {
            case '*':
                node->u.integerLit.value = node->u.integerLit.value * node2->u.integerLit.value;
                break;
            case '/':
                if (node2->u.integerLit.value == 0)
                    ParseError(c, "division by zero in constant expression", NULL);
                node->u.integerLit.value = node->u.integerLit.value / node2->u.integerLit.value;
                break;
            case T_MOD:
                if (node2->u.integerLit.value == 0)
                    ParseError(c, "division by zero in constant expression", NULL);
                node->u.integerLit.value = node->u.integerLit.value % node2->u.integerLit.value;
                break;
            default:
                /* not reached */
                break;
            }
        }
        else {
            int op;
            switch ((int)tkn) {
            case '*':
                op = OP_MUL;
                break;
            case '/':
                op = OP_DIV;
                break;
            case T_MOD:
                op = OP_REM;
                break;
            default:
                /* not reached */
                op = 0;
                break;
            }
            node = MakeBinaryOpNode(c, op, node, node2);
        }
    }
    SaveToken(c, tkn);
    return node;
}

/* ParseExpr11 - handle unary operators */
static ParseTreeNode *ParseExpr11(ParseContext *c)
{
    ParseTreeNode *node;
    Token tkn;
    switch ((int)(tkn = GetToken(c))) {
    case '+':
        node = ParsePrimary(c);
        break;
    case '-':
        node = ParsePrimary(c);
        if (IsIntegerLit(node))
            node->u.integerLit.value = -node->u.integerLit.value;
        else
            node = MakeUnaryOpNode(c, OP_NEG, node);
        break;
    case T_NOT:
        node = ParsePrimary(c);
        if (IsIntegerLit(node))
            node->u.integerLit.value = !node->u.integerLit.value;
        else
            node = MakeUnaryOpNode(c, OP_NOT, node);
        break;
    case '~':
        node = ParsePrimary(c);
        if (IsIntegerLit(node))
            node->u.integerLit.value = ~node->u.integerLit.value;
        else
            node = MakeUnaryOpNode(c, OP_BNOT, node);
        break;
    default:
        SaveToken(c,tkn);
        node = ParsePrimary(c);
        break;
    }
    return node;
}

/* ParsePrimary - parse function calls and array references */
ParseTreeNode *ParsePrimary(ParseContext *c)
{
    ParseTreeNode *node;
    int tkn;
    node = ParseSimplePrimary(c);
    while ((tkn = GetToken(c)) == '(' || tkn == '[') {
        switch ((int)tkn) {
        case '[':
            node = ParseArrayReference(c, node);
            break;
        case '(':
            node = ParseCall(c, node);
            break;
        default:
            /* not reached */
            break;
        }
    }
    SaveToken(c, tkn);
    return node;
}

/* ParseArrayReference - parse an array reference */
static ParseTreeNode *ParseArrayReference(ParseContext *c, ParseTreeNode *arrayNode)
{
    ParseTreeNode *node = NewParseTreeNode(c, NULL, NodeTypeArrayRef);

    /* setup the array reference */
    node->u.arrayRef.array = arrayNode;

    /* get the index expression */
    node->u.arrayRef.index = ParseExpr(c);

    /* check for the close bracket */
    FRequire(c, ']');
    return node;
}

/* ParseCall - parse a function or subroutine call */
static ParseTreeNode *ParseCall(ParseContext *c, ParseTreeNode *functionNode)
{
    Type *type = GetTypePtr(functionNode->type);
    ParseTreeNode *node = NewParseTreeNode(c, type->u.functionInfo.returnType, NodeTypeFunctionCall);
    VMHANDLE arg = type->u.functionInfo.arguments.head;
    ExprList *list = &node->u.functionCall.args;
    Token tkn;

    /* intialize the function call node */
    node->u.functionCall.fcn = functionNode;
    list->head = list->tail = NULL;

    /* parse the argument list */
    if ((tkn = GetToken(c)) != ')') {
        SaveToken(c, tkn);
        do {
            ParseTreeNode *actual;
        
            /* get the actual argument */
            actual = ParseExpr(c);
        
            /* check the argument count and type */
            if (arg) {
                Local *sym = GetLocalPtr(arg);
                if (actual->type != sym->type)
                    ParseError(c, "wrong argument type");
                arg = sym->next;
            }
            else
                ParseError(c, "too many arguments");

            AddExprToList(c, list, actual);
        } while ((tkn = GetToken(c)) == ',');
        Require(c, tkn, ')');
    }

    /* make sure there werent' too many arguments specified */
    if (arg)
        ParseError(c, "too few arguments");

    /* return the function call node */
    return node;
}

/* ParseSimplePrimary - parse a primary expression */
static ParseTreeNode *ParseSimplePrimary(ParseContext *c)
{
    ParseTreeNode *node;
    switch (GetToken(c)) {
    case '(':
        node = ParseExpr(c);
        FRequire(c,')');
        break;
    case T_NUMBER:
        node = NewParseTreeNode(c, CommonType(c->heap, integerType), NodeTypeIntegerLit);
        node->u.integerLit.value = c->value;
        break;
    case T_STRING:
        node = NewParseTreeNode(c, CommonType(c->heap, stringType), NodeTypeStringLit);
        node->u.stringLit.string = StoreByteVector(c->heap, ObjTypeString, (uint8_t *)c->token, strlen(c->token));
        break;
    case T_IDENTIFIER:
        node = GetSymbolRef(c, c->token);
        break;
    default:
        ParseError(c, "Expecting a primary expression", NULL);
        node = NULL; /* not reached */
        break;
    }
    return node;
}

/* GetSymbolRef - setup a symbol reference */
ParseTreeNode *GetSymbolRef(ParseContext *c, char *name)
{
    ParseTreeNode *node = NewParseTreeNode(c, NULL, NodeTypeSymbolRef);
    VMHANDLE symbol;

    /* handle local variables within a function */
    if (c->codeType != CODE_TYPE_MAIN && (symbol = FindLocal(&c->locals, name)) != NULL) {
        node->u.symbolRef.symbol = symbol;
        node->u.symbolRef.fcn = code_local;
    }

    /* handle function arguments */
    else if (c->codeType != CODE_TYPE_MAIN && (symbol = FindLocal(&c->arguments, name)) != NULL) {
        node->u.symbolRef.symbol = symbol;
        node->u.symbolRef.fcn = code_local;
    }

    /* handle global symbols */
    else if ((symbol = FindGlobal(c->heap, c->token)) != NULL) {
        Symbol *sym = GetSymbolPtr(symbol);
        if (IsConstant(sym)) {
            if (IsHandleType(sym->type)) {
                node->nodeType = NodeTypeHandleLit;
                node->u.handleLit.handle = sym->v.hValue;
            }
            else {
                node->nodeType = NodeTypeIntegerLit;
                node->u.integerLit.value = sym->v.iValue;
            }
        }
        else {
            node->u.symbolRef.symbol = symbol;
            node->u.symbolRef.fcn = code_global;
        }
    }

    /* handle undefined symbols */
    else {
        symbol = AddGlobal(c->heap, name, SC_VARIABLE, DefaultType(c, name));
        node->u.symbolRef.symbol = symbol;
        node->u.symbolRef.fcn = code_global;
    }
    
    /* store the type */
    node->type = GetSymbolPtr(symbol)->type;

    /* return the symbol reference node */
    return node;
}

/* MakeUnaryOpNode - allocate a unary operation parse tree node */
static ParseTreeNode *MakeUnaryOpNode(ParseContext *c, int op, ParseTreeNode *expr)
{
    ParseTreeNode *node = NewParseTreeNode(c, expr->type, NodeTypeUnaryOp);
    node->u.unaryOp.op = op;
    node->u.unaryOp.expr = expr;
    return node;
}

/* MakeBinaryOpNode - allocate a binary operation parse tree node */
static ParseTreeNode *MakeBinaryOpNode(ParseContext *c, int op, ParseTreeNode *left, ParseTreeNode *right)
{
    ParseTreeNode *node = NewParseTreeNode(c, left->type, NodeTypeBinaryOp);
    node->u.binaryOp.op = op;
    node->u.binaryOp.left = left;
    node->u.binaryOp.right = right;
    return node;
}

/* NewParseTreeNode - allocate a new parse tree node */
static ParseTreeNode *NewParseTreeNode(ParseContext *c, VMHANDLE type, int nodeType)
{
    ParseTreeNode *node = (ParseTreeNode *)LocalAlloc(c, sizeof(ParseTreeNode));
    memset(node, 0, sizeof(ParseTreeNode));
    node->type = type;
    node->nodeType = nodeType;
    return node;
}

/* AddExprToList - add an expression to an expression list */
static void AddExprToList(ParseContext *c, ExprList *list, ParseTreeNode *expr)
{
    ExprListEntry *entry = (ExprListEntry *)LocalAlloc(c, sizeof(ExprListEntry));
    entry->expr = expr;
    if (!(entry->prev = list->tail))
        list->head = entry;
    else
        list->tail->next = entry;
    list->tail = entry;
    entry->next = NULL;
}

/* DefaultType - get the default type based on the variable name */
VMHANDLE DefaultType(ParseContext *c, const char *name)
{
    VMHANDLE type;
    switch (name[strlen(name) - 1]) {
    case '$':
        type = CommonType(c->heap, stringType);
        break;
    default:
        type = CommonType(c->heap, integerType);
        break;
    }
    return type;
}

/* IsConstant - check to see if the value of a symbol is a constant */
int IsConstant(Symbol *symbol)
{
    return symbol->storageClass == SC_CONSTANT;
}

/* IsIntegerLit - check to see if a node is an integer literal */
int IsIntegerLit(ParseTreeNode *node)
{
    return node->nodeType == NodeTypeIntegerLit;
}
