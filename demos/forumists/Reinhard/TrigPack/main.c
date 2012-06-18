#include "TrigPack.h"

int main(int argc, char *argv[])
{
    char * degrees[] = {"15.0","30.0","45.0","60.0","75.0","90.0",NULL}; // TODO 2,3,4 Quadrant
    char * degree;
    int i=0;
    unsigned angle,sin,arg1,arg2,arg3,res;

    for(;;)
    {
        degree = degrees[i];
        if(degree == NULL)
            break;

        angle = StrToQval(degree);
        sin = SIN_Deg(angle);
        QvalToStr(sin); 
        printf("Sin ( %s [degree] ) = %s \n",degree,strB); 
        i++;     
    }
    
    arg1 = StrToQval(degrees[0]);
    arg2 = StrToQval(degrees[1]);
    arg3 = StrToQval(degrees[3]);
    res = Qmuldiv(arg1,arg2,arg3);
    QvalToStr(res); 
    printf("%s * %s / %s = %s\n",degrees[0],degrees[1],degrees[3],strB);


    return 0;
}
