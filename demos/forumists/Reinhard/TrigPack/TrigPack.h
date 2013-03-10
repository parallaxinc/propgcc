#include <stdio.h>
#include <stdlib.h>

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


unsigned StrToQval(char * strP); 
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


void QvalToStr(unsigned qV);

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



//'Conversions between number formats-----------------------------------


unsigned Qval(int intP); 

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



unsigned IangleToQval(int iA);

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

unsigned QvalToIangle(unsigned qV);

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






unsigned Qmuldiv(unsigned arg1,unsigned  arg2,unsigned arg3);

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


// 'Conversions between angle formats------------------------------------


unsigned DegToRad(unsigned dg);

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


unsigned RadToDeg(unsigned dg);

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




unsigned Qint(unsigned qV);

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


unsigned Qfrac(unsigned qV);

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



// 'Functions to access ROM Tables---------------------------------------


unsigned SIN_ROM(unsigned iA);

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


unsigned SIN_Deg(unsigned qVd);

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




