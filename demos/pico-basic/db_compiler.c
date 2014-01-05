/* db_compiler.c - a simple basic compiler
 *
 * Copyright (c) 2009 by David Michael Betz.  All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <setjmp.h>
#include <string.h>
#include <ctype.h>
#include "db_compiler.h"
#include "db_vmdebug.h"
#include "db_vm.h"
        
/* Compile - compile a program */
VMHANDLE Compile(System *sys, ObjHeap *heap, int oneStatement)
{
    int prompt = VMFALSE;
    ParseContext *c;

    /* allocate and initialize the parse context */
    if (!(c = (ParseContext *)AllocateFreeSpace(sys, sizeof(ParseContext))))
        return NULL;
    memset(c, 0, sizeof(ParseContext));
    c->sys = sys;
    c->heap = heap;
    
    /* setup the heap before/after compact functions */
    heap->beforeCompact = heap->afterCompact = NULL;
    
    /* setup an error target */
    if (setjmp(c->sys->errorTarget) != 0)
        return NULL;

    /* use the rest of the free space for the compiler heap */
    c->nextLocal = sys->freeNext;
    c->heapSize = sys->freeTop - sys->freeNext;

    /* initialize block nesting table */
    c->btop = (Block *)((char *)c->blockBuf + sizeof(c->blockBuf));
    c->bptr = c->blockBuf - 1;

    /* initialize the code staging buffer */
    c->ctop = c->codeBuf + sizeof(c->codeBuf);
    c->cptr = c->codeBuf;

    /* initialize the scanner */
    c->savedToken = 0;
    c->labels = NULL;

    /* start in the main code */
    c->codeType = CODE_TYPE_MAIN;

    /* initialize scanner */
    c->inComment = VMFALSE;
    
    /* get the next line */
    do {
        Token tkn;
        
        /* get the next line */
        if (!oneStatement || prompt) {
            if (prompt)
                VM_printf("  > ");
            if (!GetLine(c->sys))
                break;
        }
            
        /* prompt on continuation lines */
        prompt = (oneStatement ? VMTRUE : VMFALSE);
            
        /* parse the statement */
        if ((tkn = GetToken(c)) != T_EOL)
            ParseStatement(c, tkn);
            
    } while (!oneStatement || c->codeType != CODE_TYPE_MAIN || c->bptr >= c->blockBuf);
        
    /* end the main code with a halt */
    putcbyte(c, OP_HALT);
    
    /* write the main code */
    InitSymbolTable(&c->arguments);
    StartCode(c, "main", CODE_TYPE_MAIN, NULL);
    StoreCode(c);

    /* return the main code object */
    return c->code;
}

/* StartCode - start a function under construction */
void StartCode(ParseContext *c, char *name, CodeType type, VMHANDLE returnType)
{
    /* all functions must precede or follow the main code */
    if (type != CODE_TYPE_MAIN && c->cptr > c->codeBuf)
        ParseError(c, "functions must precede or follow the main code", NULL);

    /* don't allow nested functions */
    if (type != CODE_TYPE_MAIN && c->codeType != CODE_TYPE_MAIN)
        ParseError(c, "nested functions are not supported", NULL);

    /* initialize the code object under construction */
    c->codeName = name;
    c->argumentCount = c->handleArgumentCount = 0;
    InitSymbolTable(&c->locals);
    c->code = NewCode(c->heap, 0);
    c->localOffset = -F_SIZE - 1;
    c->handleLocalOffset = HF_SIZE + 1;
    c->codeType = type;
    c->returnType = returnType;
    
    /* write the code prolog */
    if (type != CODE_TYPE_MAIN) {
        putcbyte(c, OP_RESERVE);
        putcbyte(c, 0);
        putcbyte(c, 0);
    }
}

/* StoreCode - store the function or method under construction */
void StoreCode(ParseContext *c)
{
    int codeSize;

    /* check for unterminated blocks */
    switch (CurrentBlockType(c)) {
    case BLOCK_IF:
    case BLOCK_ELSE:
        ParseError(c, "expecting END IF", NULL);
    case BLOCK_FOR:
        ParseError(c, "expecting NEXT", NULL);
    case BLOCK_DO:
        ParseError(c, "expecting LOOP", NULL);
    case BLOCK_NONE:
        break;
    }

    /* fixup the RESERVE instruction at the start of the code */
    if (c->codeType != CODE_TYPE_MAIN) {
        c->codeBuf[1] = (-F_SIZE - 1) - c->localOffset;
        c->codeBuf[2] = c->handleLocalOffset - 1;
        if (c->returnFixups == codeaddr(c) - sizeof(VMVALUE)) {
            c->returnFixups = rd_cword(c, c->returnFixups);
            c->cptr -= sizeof(VMVALUE) + 1;
        }
        fixupbranch(c, c->returnFixups, codeaddr(c));
        if (c->codeType == CODE_TYPE_FUNCTION)
            putcbyte(c, IsHandleType(c->returnType) ? OP_RETURNH : OP_RETURN);
        else
            putcbyte(c, OP_RETURNV);
        putcbyte(c, c->argumentCount);
        putcbyte(c, c->handleArgumentCount);
    }

    /* make sure all referenced labels were defined */
    CheckLabels(c);

    /* determine the code size */
    codeSize = (int)(c->cptr - c->codeBuf);

#if 0
    VM_printf("%s:\n", c->codeName);
    DecodeFunction(0, c->codeBuf, codeSize);
    DumpLocalVariables(c);
    VM_putchar('\n');
#endif

    /* store the vector object */
    StoreByteVectorData(c->heap, c->code, c->codeBuf, codeSize);

    /* empty the local heap */
    c->nextLocal = c->sys->freeNext;
    InitSymbolTable(&c->arguments);
    InitSymbolTable(&c->locals);
    c->labels = NULL;

    /* reset to compile the next code */
    c->codeType = CODE_TYPE_MAIN;
    c->cptr = c->codeBuf;
}

/* DumpLocalVariables - dump a local symbol table */
void DumpLocalVariables(ParseContext *c)
{
    DumpLocals(&c->arguments, "arguments");
    DumpLocals(&c->locals, "locals");
}

/* LocalAlloc - allocate memory from the local heap */
void *LocalAlloc(ParseContext *c, size_t size)
{
    System *sys = c->sys;
    void *p;
    size = (size + ALIGN_MASK) & ~ALIGN_MASK;
    if (c->nextLocal + size > sys->freeTop)
        Abort(c->sys, "insufficient memory");
    p = c->nextLocal;
    c->nextLocal += size;
    return p;
}
