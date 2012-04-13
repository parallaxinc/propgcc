/* db_symbols.c - symbol table routines
 *
 * Copyright (c) 2009 by David Michael Betz.  All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "db_compiler.h"

/* InitSymbolTable - initialize a symbol table */
void InitSymbolTable(SymbolTable *table)
{
    table->head = NULL;
    table->tail = NULL;
    table->count = 0;
}

/* AddGlobal - add a global symbol to the symbol table */
VMHANDLE AddGlobal(ParseContext *c, const char *name, StorageClass storageClass, VMHANDLE type)
{
    VMHANDLE symbol;
    
    /* allocate the symbol object */
    if (!(symbol = NewSymbol(c->heap, name, storageClass, type)))
        return NULL;
        
    /* add it to the symbol table */
    if (c->globals.tail == NULL)
        c->globals.head = c->globals.tail = symbol;
    else {
        Symbol *last = GetSymbolPtr(c->globals.tail);
        c->globals.tail = symbol;
        last->next = symbol;
    }
    ++c->globals.count;
    
    /* return the symbol */
    return symbol;
}

/* FindGlobal - find a symbol in the global symbol table */
VMHANDLE FindGlobal(ParseContext *c, const char *name)
{
    VMHANDLE symbol = c->globals.head;
    while (symbol) {
        Symbol *sym = GetSymbolPtr(symbol);
        if (strcasecmp(name, sym->name) == 0)
            return symbol;
        symbol = sym->next;
    }
    return NULL;
}

/* AddLocal - add a symbol to a local symbol table */
VMHANDLE AddLocal(ParseContext *c, const char *name, VMHANDLE type, VMVALUE offset)
{
    VMHANDLE local;
    
    /* allocate the local symbol object */
    if (!(local = NewLocal(c->heap, name, type, offset)))
        return NULL;
        
    /* add it to the symbol table */
    if (c->locals.tail == NULL)
        c->locals.head = c->locals.tail = local;
    else {
        Local *last = GetLocalPtr(c->locals.tail);
        c->locals.tail = local;
        last->next = local;
    }
    ++c->locals.count;
    
    /* return the symbol */
    return local;
}

/* FindLocal - find a local symbol in a symbol table */
VMHANDLE FindLocal(ParseContext *c, const char *name)
{
    VMHANDLE local = c->locals.head;
    while (local) {
        Local *sym = GetLocalPtr(local);
        if (strcasecmp(name, sym->name) == 0)
            return local;
        local = sym->next;
    }
    return NULL;
}

/* IsConstant - check to see if the value of a symbol is a constant */
int IsConstant(Symbol *symbol)
{
    return symbol->storageClass == SC_CONSTANT;
}

/* DumpGlobals - dump the global symbol table */
void DumpGlobals(ParseContext *c)
{
    VMHANDLE symbol = c->globals.head;
    if (symbol) {
        VM_printf("Globals:\n");
        while (symbol) {
            Symbol *sym = GetSymbolPtr(symbol);
            VM_printf("  %s %04x\n", sym->name, sym->v.iValue);
            symbol = sym->next;
        }
    }
}

/* DumpLocals - dump a local symbol table */
void DumpLocals(ParseContext *c)
{
    VMHANDLE local = c->locals.head;
    if (local) {
        VM_printf("Locals:\n");
        while (local) {
            Local *sym = GetLocalPtr(local);
            VM_printf("  %s %d\n", sym->name, sym->offset);
            local = sym->next;
        }
    }
}

