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

#ifdef WIN32
#define strcasecmp  _stricmp
#endif

/* program limits */
#define MAXLINE         128
#define MAXTOKEN        32
#if 0
#define MAXCODE         32768
#else
#define MAXCODE         1024
#endif

/* forward type declarations */
typedef struct SymbolTable SymbolTable;
typedef struct Symbol Symbol;
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

typedef struct String String;
struct String {
    String *next;
    int placed;
    int16_t object;
    int fixups;
    uint8_t value[1];
};

typedef struct Label Label;
struct Label {
    Label *next;
    int offset;
    int fixups;
    char name[1];
};

/* storage class ids */
typedef enum {
    SC_CONSTANT,
    SC_VARIABLE
} StorageClass;

/* symbol table */
struct SymbolTable {
    Symbol *head;
    Symbol **pTail;
    int count;
};

/* symbol structure */
struct Symbol {
    Symbol *next;
    StorageClass storageClass;
    int16_t value;
    int16_t initialValue;
    char name[1];
};

/* code types */
typedef enum {
    CODE_TYPE_MAIN,
    CODE_TYPE_FUNCTION
} CodeType;

/* parse context */
typedef struct {
    jmp_buf errorTarget;            /* error target */
    uint8_t *freeSpace;             /* base of free space */
    uint8_t *freeNext;              /* next free space available */
    uint8_t *freeTop;               /* top of free space */
    uint8_t *nextGlobal;            /* next global heap space location */
    uint8_t *nextLocal;             /* next local heap space location */
    size_t heapSize;                /* size of heap space in bytes */
    size_t maxHeapUsed;             /* maximum amount of heap space allocated so far */
    int (*getLine)(void *cookie, char *buf, int len, int16_t *pLineNumber);
                                    /* scan - function to get a line of input */
    void *getLineCookie;            /* scan - cookie for the getLine function */
    char lineBuf[MAXLINE];          /* scan - current input line */
    char *linePtr;                  /* scan - pointer to the current character */
    int lineNumber;                 /* scan - current line number */
    Token savedToken;               /* scan - lookahead token */
    int tokenOffset;                /* scan - offset to the start of the current token */
    char token[MAXTOKEN];           /* scan - current token string */
    int16_t value;                  /* scan - current token integer value */
    int inComment;                  /* scan - inside of a slash/star comment */
    SymbolTable globals;            /* parse - global variables and constants */
    String *strings;                /* parse - string constants */
    Label *labels;                  /* parse - local labels */
    CodeType codeType;              /* parse - type of code under construction */
    char *codeName;                 /* parse - name of code under construction */
    SymbolTable arguments;          /* parse - arguments of current function definition */
    SymbolTable locals;             /* parse - local variables of current function definition */
    int localOffset;                /* parse - offset to next available local variable */
    int16_t code;                   /* parse - code object under construction */
    Block blockBuf[10];             /* parse - stack of nested blocks */
    Block *bptr;                    /* parse - current block */
    Block *btop;                    /* parse - top of block stack */
    uint8_t *cptr;                  /* generate - next available code staging buffer position */
    uint8_t *ctop;                  /* generate - top of code staging buffer */
    uint8_t codeBuf[MAXCODE];       /* generate - code staging buffer */
    ImageHdr *image;                /* bytecode - header of image being constructed */
    int16_t maxObjects;             /* bytecode - maximum number of objects */
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
        String *str;
        int val;
    } u;
};

/* parse tree node types */
enum {
    NodeTypeSymbolRef,
    NodeTypeStringLit,
    NodeTypeIntegerLit,
    NodeTypeFunctionLit,
    NodeTypeUnaryOp,
    NodeTypeBinaryOp,
    NodeTypeArrayRef,
    NodeTypeFunctionCall,
    NodeTypeDisjunction,
    NodeTypeConjunction
};

/* parse tree node structure */
struct ParseTreeNode {
    int nodeType;
    union {
        struct {
            Symbol *symbol;
            void (*fcn)(ParseContext *c, PValOp op, PVAL *pv);
            int offset;
        } symbolRef;
        struct {
            String *string;
        } stringLit;
        struct {
            int16_t value;
        } integerLit;
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
            ExprListEntry *args;
            int argc;
        } functionCall;
        struct {
            ExprListEntry *exprs;
        } exprList;
    } u;
};

/* expression list entry structure */
struct ExprListEntry {
    ParseTreeNode *expr;
    ExprListEntry *next;
};

/* db_compiler.c */
void InitCompiler(ParseContext *c, uint8_t *freeSpace, size_t freeSize);
int Compile(ParseContext *c, int maxObjects);
void StartCode(ParseContext *c, char *name, CodeType type);
void StoreCode(ParseContext *c);
void AddIntrinsic(ParseContext *c, char *name, int index);
String *AddString(ParseContext *c, char *value);
int16_t AddStringRef(String *str, int offset);
void *GlobalAlloc(ParseContext *c, size_t size);
void *LocalAlloc(ParseContext *c, size_t size);
void Fatal(ParseContext *c, char *fmt, ...);

/* db_statement.c */
void ParseStatement(ParseContext *c, Token tkn);
BlockType CurrentBlockType(ParseContext *c);
void CheckLabels(ParseContext *c);

/* db_expr.c */
void ParseRValue(ParseContext *c);
ParseTreeNode *ParseExpr(ParseContext *c);
ParseTreeNode *ParsePrimary(ParseContext *c);
ParseTreeNode *GetSymbolRef(ParseContext *c, char *name);
int IsIntegerLit(ParseTreeNode *node);

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
void ParseError(ParseContext *c, char *fmt, ...);

/* db_symbols.c */
void InitSymbolTable(SymbolTable *table);
Symbol *AddGlobal(ParseContext *c, const char *name, StorageClass storageClass, int value, int16_t initialValue);
Symbol *AddArgument(ParseContext *c, const char *name, StorageClass storageClass, int value);
Symbol *AddLocal(ParseContext *c, const char *name, StorageClass storageClass, int value);
Symbol *FindSymbol(SymbolTable *table, const char *name);
int IsConstant(Symbol *symbol);
void DumpSymbols(SymbolTable *table, char *tag);

/* db_generate.c */
void code_lvalue(ParseContext *c, ParseTreeNode *expr, PVAL *pv);
void code_rvalue(ParseContext *c, ParseTreeNode *expr);
void rvalue(ParseContext *c, PVAL *pv);
void chklvalue(ParseContext *c, PVAL *pv);
void code_global(ParseContext *c, PValOp fcn, PVAL *pv);
void code_local(ParseContext *c, PValOp fcn, PVAL *pv);
int codeaddr(ParseContext *c);
int putcbyte(ParseContext *c, int b);
int putcword(ParseContext *c, int w);
int merge(ParseContext *c, int chn, int chn2);
void fixup(ParseContext *c, int chn, int val);
void fixupbranch(ParseContext *c, int chn, int val);

/* db_image.c */
int16_t StoreBVector(ParseContext *c, const uint8_t *buf, int size);
void StoreBVectorData(ParseContext *c, int16_t object, int16_t proto, const uint8_t *buf, int size);
int16_t StoreVector(ParseContext *c, const int16_t *buf, int size);
int16_t NewObject(ParseContext *c);

/* scratch buffer interface */
int BufWriteWords(int offset, const int16_t *buf, int size);
int BufReadWords(int offset, int16_t *buf, int size);

#endif

