/*
# #########################################################
# This file demonstrates starting and running a PASM TV
# driver from C.
#   
# Copyright (c) 2011 Steve Denson
# MIT Licensed
# #########################################################
*/

#include "stdio.h"
#include "cog.h"
#include "propeller.h"
#include "TvText.h"

void main (int argc, char* argv[])
{
    printf("hello, world!");
    tvText_start(12);
    tvText_str("Hello TV!");
    printf("\r\ngoodbyte, world!\r\n");
    while(1);
}
