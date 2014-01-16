/* db_compiler.h - definitions for a simple basic compiler
 *
 * Copyright (c) 2009 by David Michael Betz.  All rights reserved.
 *
 */

#ifndef __DB_COMPILER_H__
#define __DB_COMPILER_H__

#include <stdio.h>
#include "db_types.h"
#include "db_image.h"
#include "db_system.h"
#include "db_vm.h"

#ifdef WIN32
#define strcasecmp  _stricmp
#endif

/* program limits */
#define MAXTOKEN        32

/* forward type declarations */
typedef struct ParseTreeNode ParseTreeNode;
typedef struct ExprListEntry ExprListEntry;

/* lexical tokens */
typedef enum {
    T_NONE,
    T_REM = 0x100,  /* keywords start here */
    T_DEF,
    T_FUNCTION,
    T_SUB,
    T_DIM,
    T_AS,
    T_LET,
    T_IF,
    T_THEN,
    T_ELSE,
    T_END,
    T_FOR,
    T_TO,
    T_STEP,
    T_NEXT,
    T_DO,
    T_WHILE,
    T_UNTIL,
    T_LOOP,
    T_GOTO,
    T_MOD,
    T_AND,
    T_OR,
    T_NOT,
    T_STOP,
    T_RETURN,
    T_PRINT,
    T_ELSE_IF,  /* compound keywords */
    T_END_FUNCTION,
    T_END_SUB,
    T_END_IF,
    T_DO_WHILE,
    T_DO_UNTIL,
    T_LOOP_WHILE,
    T_LOOP_UNTIL,
    T_LE,       /* non-keyword tokens */
    T_NE,
    T_GE,
    T_SHL,
    T_SHR,
    T_IDENTIFIER,
    T_NUMBER,
    T_STRING,
    T_EOL,
    T_EOF
} Token;

typedef enum {
    BLOCK_NONE,
    BLOCK_IF,
    BLOCK_ELSE,
    BLOCK_FOR,
    BLOCK_DO
} BlockType;

typedef struct Block Block;
struct Block {
    BlockType type;
    union {
        struct {
            int nxt;
            int end;
        } IfBlock;
        struct {
            int end;
        } ElseBlock;
        struct {
            int nxt;
            int end;
        } ForBlock;
        struct {
            int nxt;
            int end;
        } DoBlock;
    } u;
};

typedef struct Label Label;
struct Label {
    Label *next;
    int offset;
    int fixups;
    char name[1];
};

/* code types */
typedef enum {
    CODE_TYPE_MAIN,
    CODE_TYPE_FUNCTION,
    CODE_TYPE_SUB
} CodeType;

/* parse context */
typedef struct {
    System *sys;                    /* system context */
    ObjHeap *heap;                  /* heap */
    uint8_t *nextLocal;             /* next local heap space location */
    size_t heapSize;                /* size of heap space in bytes */
    Token savedToken;               /* scan - lookahead token */
    int tokenOffset;                /* scan - offset to the start of the current token */
    char token[MAXTOKEN];           /* scan - current token string */
    VMVALUE value;                  /* scan - current token integer value */
    int inComment;                  /* scan - inside of a slash/star comment */
    Label *labels;                  /* parse - local labels */
    CodeType codeType;              /* parse - type of code under construction */
    char *codeName;                 /* parse - name of code under construction */
    VMHANDLE returnType;            /* parse - return type of code under construction */
    int argumentCount;              /* parse - number of non-handle arguments */
    int handleArgumentCount;        /* parse - number of handle arguments */
    SymbolTable arguments;          /* parse - arguments of current function definition */
    SymbolTable locals;             /* parse - local variables of current function definition */
    int localOffset;                /* parse - offset to next available local variable */
    int handleLocalOffset;          /* parse - offset to next available local handle variable */
    int returnFixups;               /* parse - branches to the function return */
    VMHANDLE code;                  /* parse - code object under construction */
    Block blockBuf[10];             /* parse - stack of nested blocks */
    Block *bptr;                    /* parse - current block */
    Block *btop;                    /* parse - top of block stack */
    uint8_t *cptr;                  /* generate - next available code staging buffer position */
    uint8_t *ctop;                  /* generate - top of code staging buffer */
    uint8_t codeBuf[MAXCODE];       /* generate - code staging buffer */
} ParseContext;

/* partial value function codes */
typedef enum {
    PV_LOAD,
    PV_STORE
} PValOp;

/* partial value structure */
typedef struct PVAL PVAL;
struct PVAL {
    void (*fcn)(ParseContext *c, PValOp op, PVAL *pv);
    union {
        VMVALUE iValue;
        VMHANDLE hValue;
    } u;
};

/* parse tree node types */
typedef enum {
    NodeTypeSymbolRef,
    NodeTypeStringLit,
    NodeTypeIntegerLit,
    NodeTypeHandleLit,
    NodeTypeUnaryOp,
    NodeTypeBinaryOp,
    NodeTypeArrayRef,
    NodeTypeFunctionCall,
    NodeTypeDisjunction,
    NodeTypeConjunction
} NodeType;

/* expression list entry structure */
struct ExprListEntry {
    ParseTreeNode *expr;
    ExprListEntry *next;
    ExprListEntry *prev;
};

/* expression list */
typedef struct {
    ExprListEntry *head;
    ExprListEntry *tail;
} ExprList;

/* parse tree node structure */
struct ParseTreeNode {
    VMHANDLE type;
    NodeType nodeType;
    union {
        struct {
            VMHANDLE symbol;
            void (*fcn)(ParseContext *c, PValOp op, PVAL *pv);
        } symbolRef;
        struct {
            VMHANDLE string;
        } stringLit;
        struct {
            VMVALUE value;
        } integerLit;
        struct {
            VMHANDLE handle;
        } handleLit;
        struct {
            int op;
            ParseTreeNode *expr;
        } unaryOp;
        struct {
            int op;
            ParseTreeNode *left;
            ParseTreeNode *right;
        } binaryOp;
        struct {
            ParseTreeNode *array;
            ParseTreeNode *index;
        } arrayRef;
        struct {
            ParseTreeNode *fcn;
            ExprList args;
        } functionCall;
        struct {
            ExprList exprs;
        } exprList;
    } u;
};

/* db_compiler.c */
VMHANDLE Compile(System *sys, ObjHeap *heap, int oneStatement);
void StartCode(ParseContext *c, char *name, CodeType type, VMHANDLE returnType);
void StoreCode(ParseContext *c);
void DumpLocalVariables(ParseContext *c);
void *LocalAlloc(ParseContext *c, size_t size);

/* db_statement.c */
void ParseStatement(ParseContext *c, Token tkn);
BlockType CurrentBlockType(ParseContext *c);
void CheckLabels(ParseContext *c);

/* db_expr.c */
void ParseRValue(ParseContext *c);
ParseTreeNode *ParseExpr(ParseContext *c);
ParseTreeNode *ParsePrimary(ParseContext *c);
ParseTreeNode *GetSymbolRef(ParseContext *c, char *name);
VMHANDLE DefaultType(ParseContext *c, const char *name);
int IsConstant(Symbol *symbol);
int IsIntegerLit(ParseTreeNode *node);

/* db_scan.c */
void FRequire(ParseContext *c, Token requiredToken);
void Require(ParseContext *c, Token token, Token requiredToken);
int GetToken(ParseContext *c);
void SaveToken(ParseContext *c, Token token);
char *TokenName(Token token);
int SkipSpaces(ParseContext *c);
int GetChar(ParseContext *c);
void UngetC(ParseContext *c);
void ParseError(ParseContext *c, const char *err, ...);

/* db_generate.c */
void code_lvalue(ParseContext *c, ParseTreeNode *expr, PVAL *pv);
void code_rvalue(ParseContext *c, ParseTreeNode *expr);
void rvalue(ParseContext *c, PVAL *pv);
void chklvalue(ParseContext *c, PVAL *pv);
void code_global(ParseContext *c, PValOp fcn, PVAL *pv);
void code_local(ParseContext *c, PValOp fcn, PVAL *pv);
int codeaddr(ParseContext *c);
int putcbyte(ParseContext *c, int b);
int putcword(ParseContext *c, VMVALUE w);
VMVALUE rd_cword(ParseContext *c, VMUVALUE off);
void wr_cword(ParseContext *c, VMUVALUE off, VMVALUE v);
int merge(ParseContext *c, VMUVALUE chn, VMUVALUE chn2);
void fixup(ParseContext *c, VMUVALUE chn, VMUVALUE val);
void fixupbranch(ParseContext *c, VMUVALUE chn, VMUVALUE val);

#endif

