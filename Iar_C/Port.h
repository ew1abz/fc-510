//----------------------------------------------------------------------------

//Serial port support module: header file

//----------------------------------------------------------------------------

#ifndef PortH
#define PortH

//------------------------- Function prototypes: -----------------------------

void Port_Init(void);       //port init
void Port_Exe(bool t);      //TX display copy via UART
void Port_StartTX(void);    //TX request

//----------------------------------------------------------------------------

#endif
