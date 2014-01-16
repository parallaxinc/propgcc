/* db_vm.h - definitions for a simple virtual machine
 *
 * Copyright (c) 2009 by David Michael Betz.  All rights reserved.
 *
 */

#ifndef __DB_VM_H__
#define __DB_VM_H__

#include <stdio.h>
#include <stdarg.h>
#include "db_types.h"
#include "db_image.h"
#include "db_system.h"

/* forward type declarations */
typedef struct Interpreter Interpreter;

/* must be after the typedef for Interpreter */
#include "db_vmheap.h"

/* interpreter state structure */
struct Interpreter {
    System *sys;
    ObjHeap *heap;                  /* heap */
    VMVALUE *stack;
    VMVALUE *stackTop;
    VMVALUE *fp;
    VMVALUE *sp;
    VMHANDLE *hfp;
    VMHANDLE *hsp;
    VMHANDLE code;
    uint8_t *cbase;
    uint8_t *pc;
};

/* stack manipulation macros */
#define Reserve(i, n)   do {                                            \
                            if ((i)->sp - (n) <= (VMVALUE *)(i)->hsp)   \
                                StackOverflow(i);                       \
                            else  {                                     \
                                int cnt = (n);                          \
                                while (--cnt >= 0)                      \
                                    Push(i, 0);                         \
                            }                                           \
                        } while (0)
#define CPush(i, v)     do {                                            \
                            if (--(i)->sp <= (VMVALUE *)(i)->hsp) {     \
                                ++(i)->sp;                              \
                                StackOverflow(i);                       \
                            }                                           \
                            else                                        \
                                *(i)->sp = (v);                         \
                        } while (0)
#define Push(i, v)      (*--(i)->sp = (v))
#define Pop(i)          (*(i)->sp++)
#define Drop(i, n)      ((i)->sp += (n))

#define ReserveH(i, n)  do {                                            \
                            if ((i)->hsp + (n) >= (VMHANDLE *)(i)->sp)  \
                                StackOverflow(i);                       \
                            else  {                                     \
                                int cnt = (n);                          \
                                while (--cnt >= 0)                      \
                                    PushH(i, NULL);                     \
                            }                                           \
                        } while (0)
#define CPushH(i, v)    do {                                            \
                            if (++(i)->hsp >= (VMHANDLE *)(i)->sp) {    \
                                --(i)->hsp;                             \
                                StackOverflow(i);                       \
                            }                                           \
                            else                                        \
                                *(i)->hsp = (v);                        \
                        } while (0)
#define PushH(i, v)     (*++(i)->hsp = (v))
#define PopH(i)         (*(i)->hsp--)
#define DropH(i, n)     ((i)->hsp -= (n))

/* prototypes */
void StackOverflow(Interpreter *i);

extern FLASH_SPACE char str_subscript_err[];
extern FLASH_SPACE char str_stack_overflow_err[];
extern FLASH_SPACE char str_not_code_object_err[];
extern FLASH_SPACE char str_opcode_err[];
extern FLASH_SPACE char str_value_fmt[];
extern FLASH_SPACE char str_hfp_tag[];
extern FLASH_SPACE char str_hstack_entry_fmt[];
extern FLASH_SPACE char str_stack_separator[];
extern FLASH_SPACE char str_fp_tag[];
extern FLASH_SPACE char str_stack_entry[];

/* prototypes from db_vmint.c */
int Execute(System *sys, ObjHeap *heap, VMHANDLE main);

/* prototypes from db_vmdebug.c */
void DecodeFunction(VMUVALUE base, const uint8_t *code, int len);
int DecodeInstruction(VMUVALUE base, const uint8_t *code, const uint8_t *lc);
void ShowStack(Interpreter *i);

/* directory entry structure (platform specific) */
typedef struct VMDIRENT VMDIRENT;

/* open directory structure (platform specific) */
typedef struct VMDIR VMDIR;

void VM_sysinit(int argc, char *argv[]);
void VM_flush(void);
int VM_opendir(const char *path, VMDIR *dir);
int VM_readdir(VMDIR *dir, VMDIRENT *entry);
void VM_closedir(VMDIR *dir);

VMFILE *VM_fopen(const char *name, const char *mode);
int VM_fclose(VMFILE *fp);
char *VM_fgets(char *buf, int size, VMFILE *fp);
int VM_fputs(const char *buf, VMFILE *fp);

#endif
