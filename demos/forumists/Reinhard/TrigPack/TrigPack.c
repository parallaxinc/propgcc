#include "TrigPack.h"
/*
{{
+------------------------------------------------------------------------+
¦    SPIN_TrigPack.spin v2.0    ¦ Author: I.Kövesdi ¦  Rel.: 17.10.2011  ¦
+------------------------------------------------------------------------¦
¦                    Copyright (c) 2011 CompElit Inc.                    ¦               
¦                   See end of file for terms of use.                    ¦               
+------------------------------------------------------------------------¦
¦  This small Qs15-16 Fixed-point trig package is written entirely in    ¦
¦ SPIN and provides you complete floating point math and the basic       ¦
¦ trigonometric functions for robot and navigation projects. You can     ¦
¦ do ATAN2 without enlisting extra COGs to run a full Floating-point     ¦
¦ library. This object has StringToNumber, NumberToString conversion     ¦
¦ utilities to make Fixed-point math easy for your applications.         ¦
¦  This object contains the first True Random Number Generator (TRNG)    ¦
¦ with the Propeller microcontroller using only SPIN code. This TRNG will¦
¦ repeat its sequence only after the End of Times.                       ¦
¦                                                                        ¦ 
+------------------------------------------------------------------------¦
¦ Background and Detail:                                                 ¦
¦  32-bit Fixed-point arithmetic with SPIN is done in Qs15_16 format. The¦
¦ Qvalue numbers have a sign bit, 15 bits for the integer part and 16    ¦
¦ bits for the fraction.                                                 ¦
¦                                                                        ¦ 
+------------------------------------------------------------------------¦
¦ Note:                                                                  ¦
¦  The author thanks Timmoore and Chuck Taylor for bug reports and good  ¦
¦ suggestions.                                                           ¦
¦                                                                        ¦
+------------------------------------------------------------------------+

}}


CON

'General constants
_64K             = 65_536
_32K             = _64K / 2
_MAXABS          = _32K * _64K

_Q1              = _64K

_SEED            = 2011 * _Q1

_MAXANGLE        = 94_388_224  'Max angle [deg] in Qvalue before conv.

_SQRTWO          = 92_682      'sqr(2) in Qvalue format

'ROM address constants----------------------------------------------------
_BASE_SINTABLE   = $E000
_PIHALF          = $0800

'Procedure IDs
_STR2QVAL        = 1
_QVAL2STR        = 2
_QVAL            = 3
_INTFRAC2QVAL    = 4
_IANG2QVAL       = 5
_QVAL2IANG       = 6 
_DEG2RAD         = 7 
_RAD2DEG         = 8
_MUL             = 9 
_DIV             = 10
_MULDIV          = 11
_SQR             = 12
_SIND            = 13
_COSD            = 14
_TAND            = 15
_DASIN           = 16
_DACOS           = 17
_DATAN           = 18
_DATAN2          = 19
_QRADIUS         = 20
_SINR            = 21
_COSR            = 22
_TRNG            = 23
_PRNG            = 24

'Error IDs
_OVERFLOW        = 1
_DIVZERO         = 2
_INVALID_ARG     = 3
_STRFORM_ERR     = 4        

'Error response
_CONTINUE        = 0
_NOTIFY          = 1
_ABORT           = 2

'String parameters
_MAX_STR_LEN     = 12

'Trig
_270D            = 270 << 16
_180D            = 180 << 16
_90D             = 90 << 16 

 
VAR

'Global error variables
LONG             e_action       'Action on error  
LONG             e_orig         'ID of procedure where error occured
LONG             e_kind         'Type of error
LONG             e_arg1         'Critical procedure argument 1
LONG             e_arg2         'Critical procedure argument 2

'64-bit results
LONG             dQval[2]       '64-bit result

'RND Seed
LONG             qSeedTRNG      'Seed for Real Random Generator
LONG             qSeedPRNG      'Seed for Pseudo random Generator     

'Strings
BYTE             strB[_MAX_STR_LEN]    'String Buffer
*/

//'General constants
#define _64K  65536
#define _32K  (_64K / 2)
#define _MAXABS (_32K * _64K)

#define _Q1 _64K

#define _SEED (2011 * _Q1)

#define _MAXANGLE 94388224  //'Max angle [deg] in Qvalue before conv.

#define _SQRTWO  92682      //'sqr(2) in Qvalue format

//'ROM address constants----------------------------------------------------
#define _BASE_SINTABLE 0xE000
#define _PIHALF 0x0800

//'String parameters
#define _MAX_STR_LEN  12

//'Trig
#define _270D (270 << 16)
#define _180D (180 << 16)
#define _90D   (90 << 16)

char strB[_MAX_STR_LEN];

//'Conversions between Strings and Qvalues------------------------------ 


unsigned StrToQval(char * strP) 
{
unsigned qV , sg, ip, d, fp, r ;
int i;
/*'-------------------------------------------------------------------------
'-------------------------------+-----------+-----------------------------
'-------------------------------¦ StrToQval ¦-----------------------------
'-------------------------------+-----------+-----------------------------
'-------------------------------------------------------------------------
''     Action: Converts a String to Qs15_16 (Qvalue) format
'' Parameters: Pointer to zero terminated ASCII string                               
''     Result: Number in Qs15_16 Fixed-point Qvalue format              
''+Reads/Uses: - _MAX_STR_LENGTH                           (CON/LONG)
''             - _STR2QVAL, _STRFORM_ERR, _OVERFLOW        (CON/LONG) 
''             - _32K                                      (CON/LONG) 
''    +Writes: e_orig, e_kind                              (VAR/LONG)      
''      Calls: SPIN_TrigPack_Error
'-------------------------------------------------------------------------*/
sg=0;
ip=0; 
d=0;
fp=0;
qV =0;
while (*strP)
{
  if      (*strP == '-')
    sg = 1;
  else if (*strP == '+')
    continue;
  else if (*strP == '.' || *strP == ',')     
     d = 1;
  else if (*strP >= '0' && *strP <= '9') 
  {
      if (d == 0)                          //'Collect integer part
        ip = ip * 10 + *strP - '0';
      else 
      {                                    //'Collect decimal part
        fp = fp * 10 + *strP - '0';
        d++;
      }
  }
  else if(*strP == 0)
      break;                                 //'End of string
  else
      return 0; 
//    OTHER:
//      e_orig := _STR2QVAL
//      e_kind := _STRFORM_ERR
//      SPIN_TrigPack_Error   'This will decide what to do:CONT/NOTIFY/ABORT
//      RETURN 0
  ++strP;
}

  
//'Process Integer part
if (ip > (_32K))
{
//  e_orig := _STR2QVAL
//  e_kind := _OVERFLOW
//  SPIN_TrigPack_Error       'This will decide what to do:CONT/NOTIFY/ABORT
  return 0;
}
//'Integer part ready  
ip = ip << 16;

//'Process Fractional part
r=0;  
if (d > 1)
{
  fp = fp << (17 - d);
  for (i=1; i<=(d-1); i++)
  {
    r = fp % 5;
    fp= fp / 5;
    if (r >= 2)
      ++fp;
   }
}
//'Get Qvalue      
qV = ip + fp;

//'Set sign
 if (sg)
  qV = -qV;

return qV  ;
} 
//'-------------------------------End StrToQval----------------------------- 

void QvalToStr(unsigned qV)
{
//char * strP;
unsigned i,ip, fp, d, nz, cp, c;

/*'-------------------------------------------------------------------------
'-------------------------------+-----------+-----------------------------
'-------------------------------¦ QvalToStr ¦-----------------------------
'-------------------------------+-----------+-----------------------------
'-------------------------------------------------------------------------
''     Action: Converts a Qs15_16 (Qvalue) number into ASCII string
'' Parameters: Number in Qs15_16 format                              
''     Result: Pointer to zero terminated ASCII string             
''+Reads/Uses: None                   
''    +Writes: None                                    
''      Calls: None
'-------------------------------------------------------------------------*/
//'Set pointer to string buffer
//strP = strB;
cp=0;

//'Check sign of Qvalue
if (qV & 0x8000000)
{
  qV = qV & 0x7FFFFFF;
  strB[cp++] = '-'; 
}
//'Round up
qV = qV + 1  ;

//'Separate Integer and Fractional parts
ip = qV >> 16;
fp = (qV << 16)   ;
fp >>= 16;
d = 100000 ;                  //'2^16 approx. 64K, 5 decimal
                             // 'digit range
nz=0;
for(i=1; i<=6;i++)
{
  if (ip >= d)
  {
    c = (ip / d) + '0';
    strB[cp++] = c;               
    ip %= d;
    nz=1;                                
  }
  else if (nz || (d == 1))
  {
    c = (ip / d) + '0';                 
    strB[cp++] = c ;
  }     
  d /= 10;
}

if (fp > 0)
{
  strB[cp++] = '.';            //'Add decimal point
  fp = (fp * 3125) >> 11;      //'Normalize fractional part

  d = 10000;                   //'1/2^16 approx. 2E-5, 4 decimal
                               //'digit range
 
  for(i=1;i<=4;i++)
  {
    if (fp >= d)
    {
      c = (fp / d) + '0';
      strB[cp++] = c;               
      fp %= d;                                
    }
    else
    {
      c = (fp / d) + '0';                 
      strB[cp++] = c; 
    }     
    d /= 10;
  }
  //'Remove trailing zeroes of decimal fraction
  for(;;)
  {
    c = strB[--cp];
    if (c != '0')
      break;
  }
       
  strB[++cp] = 0;

  if (strB[cp-1] == '.')
    strB[cp-1] = 0 ;
}
else
  strB[cp] = 0;   

return;
}
//'------------------------------End of QvalToStr---------------------------


//'Conversions between number formats-----------------------------------


unsigned Qval(int intP) 
{
unsigned s, qv ;
/*'-------------------------------------------------------------------------
'----------------------------------+------+-------------------------------
'----------------------------------¦ QVal ¦-------------------------------
'----------------------------------+------+-------------------------------
'-------------------------------------------------------------------------
''     Action: Converts a LONG Integer into  Qs15_16 Fixed-point format
'' Parameters: LONG number               
''     Result: Number in Qs15_16 Fixed-point format                   
''+Reads/Uses: - _32K
''             - _QVAL, _OVERFLOW                   
''    +Writes: e_orig, e_kind
''      Calls: SPIN_TrigPack_Error                                    
'-------------------------------------------------------------------------*/
qv=0;
s=0;
//'Check sign
if (intP & 0x80000000)
{
  intP = -intP;
  s = 1;
}

//'Check input and signal error
if (intP > _32K)
{
//  e_orig := _QVAL
//  e_kind := _OVERFLOW
//  SPIN_TrigPack_Error
  return 0;
}

//'Shift integer part
qv = (intP << 16);

//'Restore negative sign
if (s)
  qv = -qv;

return qv;
}
//'-------------------------------End of Qval-------------------------------


unsigned IangleToQval(int iA)
{
unsigned  s, ip, fp, qv ;
/*'-------------------------------------------------------------------------
'-----------------------------+--------------+----------------------------
'-----------------------------¦ IangleToQval ¦----------------------------
'-----------------------------+--------------+----------------------------
'-------------------------------------------------------------------------
''     Action: Converts iAngle (4K=2Pi) into Qs15_16 Fixed-point Qvalue
''             in degrees    
'' Parameters: Iangle as LONG integer (4096=2Pi)               
''     Result: Angle in degrees in Qs15_16 Fixed-point Qvalue format                   
''+Reads/Uses: None                   
''    +Writes: None
''      Calls: None                                 
'-------------------------------------------------------------------------*/ 
s=0;

if (iA < 0)
{
  iA = -iA ;
  s = 1;
}

//'Multiply back and shift + roundup
qv = ((iA * 45) << 6) + 1440;

if (s == 1)
  qv = -qv;

return qv;
}
//'---------------------------End of IangleToQval---------------------------

unsigned QvalToIangle(unsigned qV)
{
unsigned s, ip, fp;
unsigned ia;
/*'-------------------------------------------------------------------------
'-----------------------------+--------------+----------------------------
'-----------------------------¦ QvalToIangle ¦----------------------------
'-----------------------------+--------------+----------------------------
'-------------------------------------------------------------------------
''     Action: Converts Qs15_16 Qvalue Angle [deg] to iAngle format Angle
'' Parameters: Angle [deg] in Qs15_16 Fixed-point format                               
''     Result: Angle in iAngle (Index of Angle) format (4K=2Pi)                  
''+Reads/Uses: - _C_QVD2IA error constant                  (CON/LONG)
''             - _OVERFLOW error constant                  (CON/LONG)
''             - _94388224 overflow limit                  (CON/LONG)
''    +Writes: - e_orig    global error variable           (VAR/LONG)
''             - e_kind    global error variable           (VAR/LONG)                     
''      Calls: SPIN_TrigPack_Error
''       Note: - iAngle format is the index of the angle format for the 
''               ROM table reading procedures
''             - This procedure takes care of roundings
'-------------------------------------------------------------------------*/
s=0;
if (qV & 0x80000000)
{
  qV &= 0x7FFFFFFF;
  s = 1;
}

//'Check magnitude of input  to be < 1440.25 degrees
if (qV >= _MAXANGLE)
{
//  e_orig := _IANG2QVAL
//  e_kind := _OVERFLOW
//  e_arg1 := qV
//  SPIN_TrigPack_Error    'This will decide what to do: CONT/NOTIFY/ABORT
  qV %= 23592960 ;         //'Try to respond, when error handler doesn't care
                           //'Take Mod 360 deg of large argument 
}
    
//'Scale up integer part of Qvalue.
ip = qV >> 7;   //'This includes now a hefty part of the orig. fraction

//'Multiply up this scaled-up integer part
ia = ip * 2912;

//'Get fraction of the product
fp = ia & 0x0000FFFF;

//'Calculate integer part of the product, this is unrounded iAngle
ia = ia >> 16;

//'Round iAngle
if (fp >= 0x8000)
  ia++;

//'Set sign
if (s == 1)
  ia = -ia;

return ia;
}
//'-------------------------------------------------------------------------





unsigned Qmuldiv(unsigned arg1,unsigned  arg2,unsigned arg3)
{
unsigned  s ;
unsigned long long qV,h;
/*'-------------------------------------------------------------------------
'--------------------------------+---------+------------------------------
'--------------------------------¦ Qmuldiv ¦------------------------------
'--------------------------------+---------+------------------------------
'-------------------------------------------------------------------------
''     Action: Multiplies 2 Qvalues and divides the result with a Qvalue
''
''                                        Arg1 * Arg2
''                              Result = -------------
''                                            Arg3     
''
'' Parameters: Multiplier, multiplicand and divisor in Qs15_16 Fixed-point
''             format    
''     Result: Result in Qs15_16 Fixed-point format                   
''+Reads/Uses: _MULDIV, _DIVZERO                   
''    +Writes: e_orig, e_kind                                    
''      Calls: SPIN_TrigPack_Error
''       Note: The reason for this procedure is that the product and so
''             the divident is represented with 64-bit Fixed-point
''             dQvalue (double Qvalue) and the result of the division
''             is more precise than the result of the
'' 
''                              x   := Qmul(arg1, arg2)
''                              res := Qdiv(x, arg3)
''
''             sequence of procedures, where x is shrinked into 32-bit,
''             before the division
'-------------------------------------------------------------------------*/
if (arg3 == 0)
{            //'Check divison by zero
//  e_orig := _MULDIV
//  e_kind := _DIVZERO
//  SPIN_TrigPack_Error    'This will decide what to do: CONT/NOTIFY/ABORT
  return 0;
}

//'Check signs
s=0;
if (arg1 & 0x80000000)
{
  s = 1;
  arg1 &= 0x7FFFFFFF;
}
if (arg2 & 0x80000000)
{
  s = s ^ 1;
  arg2 &= 0x7FFFFFFF;
}
if (arg3 & 0x80000000)
{
  s = s ^ 1;
  arg3 &= 0x7FFFFFFF;  
}

//'Multiply  
h = (unsigned long long)( (unsigned long long)arg1 * (unsigned long long)arg2); // !!!! 64 bit

//'Perform division of 64-bit dQvalue [[h][l]] with 32-bit Qvalue [arg3]
//qV = Div64(h, l, arg3);
qV = (unsigned)(h / arg3);

//'Set sign
if (s)
  qV = -qV;

return qV;
}
//'-------------------------------End of Qmuldiv----------------------------

// 'Conversions between angle formats------------------------------------


unsigned DegToRad(unsigned dg)
{
unsigned qV ;
/*'-------------------------------------------------------------------------
'--------------------------------+----------+-----------------------------
'--------------------------------¦ DegToRad ¦-----------------------------
'--------------------------------+----------+-----------------------------
'-------------------------------------------------------------------------
''     Action: Converts an angle in degrees into radians   
'' Parameters: Angle as Qs15_16 Fixed-point format Qvalue              
''     Result: Angle in radians in Qs15_16 Fixed-point format                   
''+Reads/Uses: None                   
''    +Writes: None
''      Calls: Qmuldiv, Qval
''       Note: Pi as 355/113, so Pi/180=71/4068                                 
'-------------------------------------------------------------------------*/ 
return ( Qmuldiv(dg, Qval(71), Qval(4068)) );
}
//'-------------------------------------------------------------------------

unsigned RadToDeg(unsigned dg)
{
unsigned qV ;
/*'-------------------------------------------------------------------------
'--------------------------------+----------+-----------------------------
'--------------------------------¦ RadToDeg ¦-----------------------------
'--------------------------------+----------+-----------------------------
'-------------------------------------------------------------------------
''     Action: Converts an angle in radians into degrees   
'' Parameters: Angle in radians as Qs15_16 Fixed-point format Qvalue              
''     Result: Angle in degrees in Qs15_16 Fixed-point format                   
''+Reads/Uses: None                   
''    +Writes: None
''      Calls: Qmuldiv, Qval
''       Note: Pi as 355/113, so 180/Pi=4068/71                                 
'------------------------------------------------------------------------- */
return ( Qmuldiv(dg, Qval(4068), Qval(71)) );
}
//'-------------------------------------------------------------------------

/*
unsigned Qsqr(unsigned arg)
{
unsigned qV , ls, fs, o, iv ;
'-------------------------------------------------------------------------
'----------------------------------+------+-------------------------------
'----------------------------------¦ Qsqr ¦-------------------------------
'----------------------------------+------+-------------------------------
'-------------------------------------------------------------------------
''     Action: Calculates the square root of a Qs15_16 Fixed-point number
'' Parameters: Argument in Qs15_16 Fixed-point format                                               
''     Result: Square-root of argument in Qs15_16 Fixed-point format    
''+Reads/Uses: - _SQR, _INVALID_ARG                         (CON/LONG)
''             - _SQRTWO (1.4142 in Qvalue format)          (CON/LONG)
''    +Writes: e_orig, e_kind                               (VAR/LONG)     
''      Calls: - SPIN_TrigPack_Error
''             - Qdiv, StrToQval
'-------------------------------------------------------------------------
if (arg & 0x80000000)             //'Check for negative argument
{
//  e_orig := _SQR
//  e_kind := _INVALID_ARG
//  SPIN_TrigPack_Error    //'This will decide what to do: CONT/NOTIFY/ABORT
  return 0; 
}

ls := 32 - (>| arg)
arg := arg << ls
iv := ^^arg
o := ls // 2
ls := ls / 2
IF ( ls =< 8)
  qV := iv << (8 - ls)
ELSE
  qV := iv >> (ls - 8)
  
IF (o == 1)
  qV := Qdiv(qV, _SQRTWO) 
}
'------------------------------- End of Qsqr------------------------------
*/


unsigned Qint(unsigned qV)
{
//'-------------------------------------------------------------------------
//'---------------------------------+------+--------------------------------
//'---------------------------------¦ Qint ¦--------------------------------
//'---------------------------------+------+--------------------------------
//'-------------------------------------------------------------------------
//''     Action: Returns the integer part of a Qvalue
//'' Parameters: Qvalue                                              
//''     Result: Integer part of a Qvalue    
//''+Reads/Uses: None
//''    +Writes: None                                 
//''      Calls: None
//''       Note: Result is in Qvalue format
//'-------------------------------------------------------------------------
return ( qV & 0xFFFF0000 );
}
//'--------------------------------End of Qint------------------------------

unsigned Qfrac(unsigned qV)
{
//'-------------------------------------------------------------------------
//'---------------------------------+-------+-------------------------------
//'---------------------------------¦ Qfrac ¦-------------------------------
//'---------------------------------+-------+-------------------------------
//'-------------------------------------------------------------------------
//''     Action: Returns the fractional part of a Qvalue
//'' Parameters: Qvalue                                              
//''     Result: Fractional part of a Qvalue    
//''+Reads/Uses: None
//''    +Writes: None                                 
//''      Calls: None
//'-------------------------------------------------------------------------
//IF (qV => 0)
//  RESULT := qV & $0000_FFFF 
//ELSE
//  RESULT := -(||qV & $0000_FFFF) 
return ( qV & 0x0000FFFF );
}
//'-------------------------------End of Qfrac------------------------------


// 'Functions to access ROM Tables---------------------------------------


unsigned SIN_ROM(unsigned iA)
{
//'-------------------------------------------------------------------------
//'--------------------------------+---------+------------------------------
//'--------------------------------¦ SIN_ROM ¦------------------------------
//'--------------------------------+---------+------------------------------
//'-------------------------------------------------------------------------
//'     Action: - Reads value from SIN Table according to iAngle address
//' Parameters: Angle in iAngle (Index of Angle) units                                 
//'     Result: Sine value for Angle in Qs15_16 Qvalue format                    
//'+Reads/Uses: - _BASE_SINTABLE   (= $E000)                   (CON/LONG)
//'             - qValue from ROM SIN Table                    (ROM/WORD)
//'    +Writes: None                                    
//'      Calls: None
//'       Note: - SIN table contains 2K 16-bit word data for the 1st
//'               quadrant in [$E000-$F000] 4KB locations
//'             - Word index goes up and down and up and down in quadrants
//'                  [0, 90]      [90, 180]      [180, 270]     [270, 380]
//'                    up            down            up            down
//'
//'   quadrant:        1              2              3              4
//'   angle:     $0000...$07FF  $0800...$0FFF  $1000...$17FF  $1800...$1FFF
//'   w.index:   $0000...$07FF  $0800...$0001  $0000...$07FF  $0800...$0001
//'       (The above 3 lines were taken after the Propeller Manual v1.1)
//'
//'             - The returned value from the table is a Qs15_16 fraction
//'-------------------------------------------------------------------------
//CASE (iA & %1_1000_0000_0000)
//  %0_0000_0000_0000:                             '1st quadrant
//    RETURN WORD[_BASE_SINTABLE][iA & $7FF]
//  %0_1000_0000_0000:                             '2nd quadrant according
//    IF (iA & $7FF)                               'to Chuck Taylor's
//      RETURN WORD[_BASE_SINTABLE][(-iA & $7FF)]  'modification
//    ELSE
//      RETURN _Q1                                       
//  %1_0000_0000_0000:                             '3rd quadrant
//    RETURN -WORD[_BASE_SINTABLE][iA & $7FF]
//  %1_1000_0000_0000:                             '4th quadrant according
//    IF (iA & $7FF)                               'to Chuck Taylor's
//      RETURN -WORD[_BASE_SINTABLE][(-iA & $7FF)] 'modification
//    ELSE
//      RETURN -_Q1  
unsigned short * ptr;
unsigned short temp = iA & 0x1800;
if (temp == 0)
{

    ptr = (  (unsigned short*)(_BASE_SINTABLE + (2*iA )) );
    return (*ptr );
}
// TODO ------------------------------------------------------------------------
else if (temp == 0x800)
{
    if( iA & 0x7FF)
    {
        return ( *(unsigned short*)(_BASE_SINTABLE + ((-2*iA) & 0x7FF)) );
    }
    else
    {
        return _Q1;
    }
}

else if (temp == 0x1000)
{
    return (- *(unsigned short*)(_BASE_SINTABLE + ((2*iA) & 0x7FF)) );
}

else if ( temp == 0x1800)
{
    if( iA & 0x7FF)
    {
        return (- *(unsigned short*)(_BASE_SINTABLE + ((-2*iA) & 0x7FF)) );
    }
    else
    {
        return -_Q1;
    }
}
// ----------------------------------------------------------------- TODO

}   
//'------------------------------End of SIN_ROM-----------------------------

unsigned SIN_Deg(unsigned qVd)
{
//'-------------------------------------------------------------------------
//'--------------------------------+---------+------------------------------
//'--------------------------------¦ SIN_Deg ¦------------------------------
//'--------------------------------+---------+------------------------------
//'-------------------------------------------------------------------------
//''     Action: Calculates sine of Angle 
//'' Parameters: Angle [deg] in Qs15_16 Fixed-point format (Qvalue)                              
//''     Result: Sine of Angle in Qs15_16 Fixed-point format (Qvalue)                  
//''+Reads/Uses: None                   
//''    +Writes: None                                    
//''      Calls: QvalToIangle, SIN_ROM 
//'-------------------------------------------------------------------------
return ( SIN_ROM(QvalToIangle(qVd)) );
}
//'------------------------------End of SIN_Deg-----------------------------



