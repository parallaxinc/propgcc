/* db_vm.h - definitions for a simple virtual machine
 *
 * Copyright (c) 2014 by David Michael Betz.  All rights reserved.
 *
 */

#ifndef __DB_VM_H__
#define __DB_VM_H__

#include "db_system.h"
#include "db_image.h"

/* prototypes from db_vmint.c */
int Execute(System *sys, ImageHdr *image, VMVALUE main);

#endif
