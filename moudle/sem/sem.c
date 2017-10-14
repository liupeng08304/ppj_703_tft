#include "sem.h"


#define MAX_SEM 20


ENUM_SEM EnumSem[MAX_SEM]={INIT_UI,NO_SEM};
uint8_t InSem=1;
uint8_t OutSem=0;
/******************************************************************************
*******************************
Function: // bool IsSemEmpty(void)
Description: //
Input: // none
//
Output: // none
Return: // ???????
Others: //
*******************************************************************************
**************************************/
void SemEmpty(void)
{
OutSem=0;
InSem=0;
}


//bool HaveSem=true;
/******************************************************************************
*******************************
Function: // void SendSem(ENUM_SEM Sem)
Description: // ?????
Input: // none
//
Output: // none
Return: // none
Others: //
*******************************************************************************
**************************************/
void SendSem(ENUM_SEM Sem)
{
// if(MAX_SEM> InSem)
// {
EnumSem[InSem++] =Sem;
InSem%=MAX_SEM;
//HaveSem=true;
//}
}
/******************************************************************************
*******************************
Function: // ENUM_SEM GetSem(void)
Description: // ?????
Input: // none
//
Output: // none
Return: // none
Others: //
*******************************************************************************
**************************************/
ENUM_SEM GetSem(void)
{
ENUM_SEM RetSem;
if(OutSem!=InSem)
{
RetSem = EnumSem[OutSem++];
OutSem%=MAX_SEM;
return RetSem;
}
return NO_SEM;
}
/******************************************************************************
*******************************
Function: // bool IsSemEmpty(void)
Description: // ????????
Input: // none
//
Output: // none
Return: // ???????
Others: //
*******************************************************************************
**************************************/
bool IsSemEmpty(void)
{
if(OutSem!=InSem)
{
return false;
}
return true;
}



