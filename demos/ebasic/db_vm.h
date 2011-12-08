/* db_vm.h - definitions for a simple virtual machine
 *
 * Copyright (c) 2009 by David Michael Betz.  All rights reserved.
 *
 */

#ifndef __DB_VM_H__
#define __DB_VM_H__

#include <stdio.h>
#include <stdarg.h>
#include <setjmp.h>
#include "db_image.h"

/* forward type declarations */
typedef struct Interpreter Interpreter;

/* intrinsic function handler type */
typedef void IntrinsicFcn(Interpreter *i);

/* interpreter state structure */
struct Interpreter {
    ImageHdr *image;
    jmp_buf errorTarget;
    int16_t *stack;
    int16_t *stackTop;
    int16_t *fp;
    int16_t *sp;
    uint8_t *pc;
    int argc;
};

/* stack manipulation macros */
#define Reserve(i, n)   do {                                    \
                            if ((i)->sp + (n) > (i)->stackTop)  \
                                StackOverflow(i);               \
                            else  {                             \
                                int cnt = (n);                  \
                                while (--cnt >= 0)              \
                                    Push(i, 0);                 \
                            }                                   \
                        } while (0)
#define CPush(i, v)     do {                                    \
                            if ((i)->sp + 1 >= (i)->stackTop)   \
                                StackOverflow(i);               \
                            else                                \
                                Push(i, v);                     \
                        } while (0)
#define Push(i, v)      (*++(i)->sp = (v))
#define Pop(i)          (*(i)->sp--)
#define Drop(i, n)      ((i)->sp -= (n))

/* prototypes */
void StackOverflow(Interpreter *i);

extern FLASH_SPACE char str_null[];
extern FLASH_SPACE char str_open_failed_err[];
extern FLASH_SPACE char str_run_failed_err[];
extern FLASH_SPACE char str_load_failed_err[];
extern FLASH_SPACE char str_string_index_range_err[];
extern FLASH_SPACE char str_no_heap_err[];
extern FLASH_SPACE char str_no_heap_handles_err[];
extern FLASH_SPACE char str_no_heap_space_err[];
extern FLASH_SPACE char str_heap_debug_output[];
extern FLASH_SPACE char str_heap_debug_output_tail[];
extern FLASH_SPACE char str_array_subscript_err[];
extern FLASH_SPACE char str_array_subscript1_err[];
extern FLASH_SPACE char str_array_subscript2_err[];
extern FLASH_SPACE char str_image_header_err[];
extern FLASH_SPACE char str_image_target_err[];
extern FLASH_SPACE char str_data_space_err[];
extern FLASH_SPACE char str_opcode_err[];
extern FLASH_SPACE char str_stack_overflow_err[];
extern FLASH_SPACE char str_object_number_err[];
extern FLASH_SPACE char str_tag_not_found_err[];
extern FLASH_SPACE char str_not_code_object_err[];
extern FLASH_SPACE char str_argument_count_err[];
extern FLASH_SPACE char str_wrong_type_err[];
extern FLASH_SPACE char str_warn_prefix[];
extern FLASH_SPACE char str_abort_prefix[];
extern FLASH_SPACE char str_program_id[];
extern FLASH_SPACE char str_ready_for_download[];

/* prototypes and variables from db_vmfcn.c */
extern IntrinsicFcn * FLASH_SPACE Intrinsics[];
extern int IntrinsicCount;

/* prototypes from db_vmint.c */
uint8_t *InitInterpreter(Interpreter *i, size_t stackSize);
int Execute(Interpreter *i, ImageHdr *image);
void Warn(const char *fmt, ...);                    /* fmt in FLASH_SPACE */
void Abort(Interpreter *i, const char *fmt, ...);   /* fmt in FLASH_SPACE */

/* prototypes from db_vmdebug.c */
void DecodeFunction(uint16_t base, const uint8_t *code, int len);
int DecodeInstruction(uint16_t base, const uint8_t *code, const uint8_t *lc);
void ShowStack(Interpreter *i);

void VM_sysinit(int argc, char *argv[]);
void VM_printf(const char *fmt, ...);           /* fmt in FLASH_SPACE */
void VM_vprintf(const char *fmt, va_list ap);   /* fmt in FLASH_SPACE */
int VM_getchar(void);
void VM_putchar(int ch);
void VM_getline(char *buf, int size);

void LOG_printf(const char *fmt, ...);

#endif
