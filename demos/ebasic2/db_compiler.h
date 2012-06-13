/* db_compiler.h - definitions for a simple basic compiler
 *
 * Copyright (c) 2009 by David Michael Betz.  All rights reserved.
 *
 */

#ifndef __DB_COMPILER_H__
#define __DB_COMPILER_H__

#include <stdio.h>
#include <setjmp.h>
#include "db_types.h"
#include "db_image.h"
#include "db_system.h"
#include "db_vmheap.h"

#ifdef WIN32
#define strcasecmp  _stricmp
#endif

/* program limits */
#define MAXTOKEN        32
#if 0
#define MAXCODE         32768
#else
#define MAXCODE         1024
#endif

/* forward type declarations */
typedef struct ParseTreeNode ParseTreeNode;
typedef struct ExprListEntry ExprListEntry;

/* lexical tokens */
typedef enum {
    T_NONE,
    T_REM = 0x100,  /* keywords start here */
    T_DEF,
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
    T_END_DEF,
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
    CODE_TYPE_FUNCTION
} CodeType;

/* initialize a common type field */
#define InitCommonType(c, field, typeid)                \
            (c)->field.type.id = typeid;                \
            (c)->field.data = (void *)&(c)->field.type; \
            (c)->field.hdr.handle = NULL;               \
            (c)->field.hdr.refCnt = 0;                  \
            (c)->field.hdr.type = ObjTypeType;          \
            (c)->field.hdr.size = sizeof(Type);

/* get a handle to one of the common types */
#define CommonType(c, field)        (&(c)->field.data)

/* parse context */
typedef struct {
    System *sys;                    /* system context */
    uint8_t *freeMark;              /* saved position for reclaiming compiler memory */
    jmp_buf errorTarget;            /* error target */
    uint8_t *nextLocal;             /* next local heap space location */
    size_t heapSize;                /* size of heap space in bytes */
    int (*getLine)(void *cookie, char *buf, int len, VMVALUE *pLineNumber);
                                    /* scan - function to get a line of input */
    void *getLineCookie;            /* scan - cookie for the getLine function */
    int lineNumber;                 /* scan - current line number */
    Token savedToken;               /* scan - lookahead token */
    int tokenOffset;                /* scan - offset to the start of the current token */
    char token[MAXTOKEN];           /* scan - current token string */
    VMVALUE value;                  /* scan - current token integer value */
    int inComment;                  /* scan - inside of a slash/star comment */
    SymbolTable globals;            /* parse - global variables and constants */
    Label *labels;                  /* parse - local labels */
    CodeType codeType;              /* parse - type of code under construction */
    char *codeName;                 /* parse - name of code under construction */
    SymbolTable arguments;          /* parse - arguments of current function definition */
    SymbolTable locals;             /* parse - local variables of current function definition */
    int localOffset;                /* parse - offset to next available local variable */
    VMHANDLE code;                  /* parse - code object under construction */
    Block blockBuf[10];             /* parse - stack of nested blocks */
    Block *bptr;                    /* parse - current block */
    Block *btop;                    /* parse - top of block stack */
    uint8_t *cptr;                  /* generate - next available code staging buffer position */
    uint8_t *ctop;                  /* generate - top of code staging buffer */
    uint8_t codeBuf[MAXCODE];       /* generate - code staging buffer */
    ConstantType integerType;       /* parse - integer type */
    ConstantType integerArrayType;  /* parse - integer array type */
    ConstantType byteType;          /* parse - byte type */
    ConstantType byteArrayType;     /* parse - byte array type */
    ConstantType floatType;         /* parse - float type */
    ConstantType floatArrayType;    /* parse - float array type */
    ConstantType stringType;        /* parse - string type */
    ConstantType stringArrayType;   /* parse - string array type */
    ObjHeap *heap;                  /* heap */
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
        VMFLOAT fValue;
        VMHANDLE hValue;
    } u;
};

/* parse tree node types */
typedef enum {
    NodeTypeSymbolRef,
    NodeTypeStringLit,
    NodeTypeIntegerLit,
    NodeTypeFloatLit,
    NodeTypeFunctionLit,
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
            VMFLOAT value;
        } floatLit;
        struct {
            int offset;
        } functionLit;
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
            int argc;
        } functionCall;
        struct {
            ExprList exprs;
        } exprList;
    } u;
};

/* db_compiler.c */
ParseContext *InitCompiler(System *sys, int maxObjects);
VMHANDLE Compile(ParseContext *c, int oneStatement);
void StartCode(ParseContext *c, char *name, CodeType type);
void StoreCode(ParseContext *c);
void AddIntrinsic(ParseContext *c, char *name, char *types, int index);
void *LocalAlloc(ParseContext *c, size_t size);
void Fatal(ParseContext *c, const char *fmt, ...);

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
int IsIntegerLit(ParseTreeNode *node);
int IsFloatLit(ParseTreeNode *node);

/* db_scan.c */
int GetLine(ParseContext *c);
void FRequire(ParseContext *c, Token requiredToken);
void Require(ParseContext *c, Token token, Token requiredToken);
int GetToken(ParseContext *c);
void SaveToken(ParseContext *c, Token token);
char *TokenName(Token token);
int SkipSpaces(ParseContext *c);
int GetChar(ParseContext *c);
void UngetC(ParseContext *c);
void ParseError(ParseContext *c, const char *err, ...);

/* db_symbols.c */
void InitSymbolTable(SymbolTable *table);
VMHANDLE AddGlobal(ParseContext *c, const char *name, StorageClass storageClass, VMHANDLE type);
VMHANDLE FindGlobal(ParseContext *c, const char *name);
VMHANDLE AddLocal(ParseContext *c, const char *name, VMHANDLE type, VMVALUE offset);
VMHANDLE FindLocal(ParseContext *c, const char *name);
int IsConstant(Symbol *symbol);
void DumpGlobals(ParseContext *c);
void DumpLocals(ParseContext *c);

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
int putcfloat(ParseContext *c, VMFLOAT w);
int merge(ParseContext *c, VMUVALUE chn, VMUVALUE chn2);
void fixup(ParseContext *c, VMUVALUE chn, VMUVALUE val);
void fixupbranch(ParseContext *c, VMUVALUE chn, VMUVALUE val);

/* scratch buffer interface */
void BufRewind(void);
int BufWriteWords(const VMVALUE *buf, int size);
int BufReadWords(VMVALUE *buf, int size);

#endif

