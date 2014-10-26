//----------------------------------------------------------------------------

//LCD support module: header file

//----------------------------------------------------------------------------

#ifndef DispH
#define DispH

//----------------------------- Constants: -----------------------------------

#define POINT 0x80 //decimal point

//------------------------- Function prototypes: -----------------------------

void Disp_Init(void);       //display init
void Disp_Update(void);     //copy display memory to LCD
void Disp_Clear(void);      //display clear and set first position
void Disp_SetPos(char p);   //set display position
void Disp_PutChar(char ch); //display char
void Disp_PutString(char __flash *s);  //display string
void Disp_Val(char s, char p, long t); //display value

//----------------------------------------------------------------------------

#endif
