/*
propeller-elf-gcc   -mlmm -o log2.elf   log2.c 
*/

#include <stdio.h>

#ifdef __PROPELLER__
#include <propeller.h>
#endif

//#define DEBUG_LOG2

int time_clocks()
{
#if defined(__propeller__)

    return (CNT);

#else
    return 0;
#endif
}

unsigned short findLeadingBitPosition(unsigned aNumber)
{
    short pos;

    if(aNumber == 0)
        return 0;

    for(pos = 31; pos >= 0; pos--)
    {
        if( aNumber & (1 << pos))
        {
            return (unsigned short)pos;             
        }
    }
}

int errUnsignedToBase2Exponent( unsigned num , unsigned * exponent)
{
    unsigned short bitpos,tmp2,index,exp=0;
    unsigned tmp,remainder;
    int shift;
    if(num <= 0 )
    { 
        // underflow
        *exponent = ~2;
        return -2;
    }
    if(num >= 0x80000000)
    {
        // overflow
        *exponent = ~1;
        return -1;
    }
    bitpos = findLeadingBitPosition(num);
#ifdef DEBUG_LOG2
    printf("bitpos:     0x%X  %d\n",bitpos,bitpos);
#endif
    tmp2 = bitpos;
    tmp = 1 << bitpos;
#ifdef DEBUG_LOG2
    printf("tmp: 0x%X  %d\n",tmp,tmp);
#endif
    remainder = num - tmp;
#ifdef DEBUG_LOG2
    printf("remainder:  0x%X  %d\n",remainder,remainder);
#endif
    if(remainder <= 0x0F)
    {
        index = remainder << 8;
    }
    else if(remainder <= 0xFF)
    {
        index = remainder << 4;
    }
    else if(remainder <= 0xFFF)
    {
        index = remainder;
    }
    else
    {
        for(shift = 11; shift >= 0; shift--)
        {
            bitpos --;
            if(remainder & (1<<bitpos))
            {
                index |= (1 << shift);
            }
        }
    }
    index &= 0xFFF;
    index += 0xC000;
#ifdef DEBUG_LOG2
    printf("index:  0x%X  %d\n",index,index);
#endif
#ifdef __PROPELLER__
    exp = *( unsigned short *)(index);
#endif
#ifdef DEBUG_LOG2
    printf("exp: 0x%X\n",exp);
#endif
    *exponent = (tmp2<<16) | exp;
    return 0;
}
int main(int argc, char *argv[])
{
    unsigned num[] = {0x60000000,2048,2047,2049,0x1FFF,0x40,2,4,8,17,0,0x80000000};
    unsigned n,n1,n2,exp,exp1,exp2;
    int err,i;
    int start, end;
    for(i=0; i < sizeof(num)/sizeof(unsigned); i++)
    {
        start= time_clocks();        
        err = errUnsignedToBase2Exponent( num[i] , &exp);
        end = time_clocks(); 
        printf("Error=%02d\tnumber=0x%08X\tBase2Exponent=0x%08X\t\tclocks:%d\n",err,num[i],exp,end-start);
    }

    printf("\n\n------------------------- cube root demo ---------------------\n");
    n1 = 2048;
    err = errUnsignedToBase2Exponent( n1 , &exp1);
    exp2 = exp1/3;
    printf("Error=%02d\tnumber=0x%08X\tBase2Exponent=0x%08X\t0x%08X\n",err,n1,exp1,exp2);
    printf("exact: 2048 ^ (1/3) = 12.699208\n");
    printf("result: %X integer part = %X  fractional part = %X\n",exp2,(exp2&0xFF0000)>>16,exp2&0xFFFF);
    printf("means: 2 ^ ( 3 + 1/2 + 1/8 + 1/32 + 1/128 .... + 1/32768) = 12.699118\n");

    return 0;
}
