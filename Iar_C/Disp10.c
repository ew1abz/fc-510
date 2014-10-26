//----------------------------------------------------------------------------

//10-digit LCD support module

//----------------------------------------------------------------------------

#include "Main.h"
#include "Disp.h"
#include "Usart.h"

//----------------------------- Constants: -----------------------------------

#define LCD_SIZE 10        //size of LCD
#define DIGITS   10        //number of BCD digits 

//Segments layout:
//
//    -- A --
//   |       |
//   F       B
//   |       |
//    -- G --
//   |       |
//   E       C
//   |       |
//    -- D --  (H)

#define _A_ 0x08
#define _B_ 0x20
#define _C_ 0x40
#define _D_ 0x04
#define _E_ 0x02
#define _F_ 0x80
#define _G_ 0x01
#define _H_ 0x10

//------------------------------ Variables: ----------------------------------

static char Buf[LCD_SIZE+2]; //display copy for send to USART + 2 points
static char Dig[LCD_SIZE]; //display copy
static char Bcd[DIGITS];   //hex to bcd buffer
static char Pos;           //current position (left to right, 0..9)

//------------------------- Function prototypes: -----------------------------

void LCD_Write(char d);       //write data to LCD
void LCD_WriteNibble(char d); //write nibble to LCD
void LCD_WriteAddres(char d); //write address to LCD
void Long2BCD(unsigned long x, char *buff); //convert long to BCD
char Encode(char s);          //encode symbol to 7-segment code

//------------------------- Write nibble to LCD: -----------------------------

void LCD_WriteNibble(char d)
{
  LCD_Write((d & 0x0F) | 0x10); //address = 1
}

//------------------------- Write address to LCD: ----------------------------

void LCD_WriteAddres(char d)
{
  LCD_Write(d & 0x0F);
}

//-------------------------- Write data to LCD: ------------------------------

void LCD_Write(char d)
{
  for(char i = 0; i < 5; i++)
  {
    Port_SCLK_0;
    if(d & 0x10) Port_DATA_1;
      else Port_DATA_0;
    d = d << 1;
    Port_SCLK_1;
  }
  Port_LOAD_0;
  Port_DATA_1;
  Port_LOAD_1;
}

//------------------------- Long2BCD conversion: -----------------------------

//Convert binary digit to BCD:
//x - input binary digit (32 bits, unsigned)
//buff - output array (10 digits)

void Long2BCD(unsigned long x, char *buff)
{
  for(char i = 0; i < 10; i++)
    buff[i] = 0;                      //output buffer clear
  for(char i = 0; i < 32; i++)        //cycle for input bits count
  {
    char c = (x >> 31) & 1;           //save carry
    x = x << 1;                       //input value shift
    for(signed char p = 9; p >= 0; p--) //cycle for digits number
    {
      char s = buff[p];               //read digit
      s = (s << 1) | c; c = 0;        //shift with carry
      if(s > 9) { s += 0x06; c = 1; } //digit correction
      s &= 0x0F;                      //select nibble
      buff[p] = s;                    //save digit
    }
  }
}

//--------------------------- Load copy to LCD: ------------------------------

char Encode(char s)
{
  static char __flash Font[]= //7-segment code look-up table
  {
    _A_ | _B_ | _C_ | _D_ | _E_ | _F_,       //code 00H, character "0"
    _B_ | _C_,                               //code 01H, character "1"
    _A_ | _B_ | _G_ | _E_ | _D_,             //code 02H, character "2"
    _A_ | _B_ | _C_ | _D_ | _G_,             //code 03H, character "3"
    _F_ | _G_ | _B_ | _C_ ,                  //code 04H, character "4"
    _A_ | _F_ | _G_ | _C_ | _D_,             //code 05H, character "5"
    _A_ | _F_ | _G_ | _C_ | _D_ | _E_,       //code 06H, character "6"
    _A_ | _B_ | _C_,                         //code 07H, character "7"
    _A_ | _B_ | _C_ | _D_ | _E_ | _F_ | _G_, //code 08H, character "8"
    _A_ | _B_ | _C_ | _D_ | _F_ | _G_,       //code 09H, character "9"
    _A_ | _B_ | _C_ | _E_ | _F_ | _G_,       //code 0AH, character "A"
    _C_ | _D_ | _E_ | _F_ | _G_,             //code 0BH, character "b"
    _A_ | _D_ | _E_ | _F_,                   //code 0CH, character "C"
    _B_ | _C_ | _D_ | _E_ | _G_,             //code 0DH, character "d"
    _A_ | _D_ | _E_ | _F_ | _G_,             //code 0EH, character "E"
    _A_ | _E_ | _F_ | _G_,                   //code 0FH, character "F"
    _A_ | _C_ | _D_ | _E_ | _F_,             //code 10H, character "G"
    _B_ | _C_ | _E_ | _F_ | _G_,             //code 11H, character "H"
    _D_ | _E_ | _F_,                         //code 12H, character "L"
    _C_ | _E_ |  _G_,                        //code 13H, character "n"
    _C_ | _D_ | _E_ |  _G_,                  //code 14H, character "o"
    _A_ | _B_ | _E_ | _F_ | _G_,             //code 15H, character "P"
    _E_ |  _G_,                              //code 16H, character "r"
    _D_ | _E_ | _F_ | _G_,                   //code 17H, character "t"
    _C_ | _D_ | _E_,                         //code 18H, character "u"
    _G_,                                     //code 19H, character -
    0                                        //code 1AH, character blank  
  };
  
  char d = s & 0x7F;         //d <- digit
  bool p = s & 0x80;         //p <- d.7 (point)
  if(d >= '0' && d <= '9') d = d - '0';
  
  switch(d)
  {
  case 'A': d = 0x0A; break; //character "A"
  case 'b': d = 0x0B; break; //character "b"
  case 'C': d = 0x0C; break; //character "C"
  case 'D':
  case 'd': d = 0x0D; break; //character "d"
  case 'e': d = 0x0E; break; //character "E"
  case 'f':
  case 'F': d = 0x0F; break; //character "F"
  case 'G': d = 0x10; break; //character "G"
  case 'H': d = 0x11; break; //character "H"
  case 'I': d = 0x01; break; //character "I"
  case 'L': d = 0x12; break; //character "L"
  case 'n': d = 0x13; break; //character "n"
  case 'O': d = 0x00; break; //character "O"
  case 'o': d = 0x14; break; //character "o"
  case 'P': d = 0x15; break; //character "P"
  case 'R':
  case 'r': d = 0x16; break; //character "r"
  case 't': d = 0x17; break; //character "t"
  case 'u': d = 0x18; break; //character "u"
  case '-': d = 0x19; break; //character "-"
  case ' ': d = 0x1A; break; //character "blank"
  }
  if(d > 0x1A) d = 0x19;     //character "-" for unknown code
  d = Font[d];               //read byte from table
  if(p) d |= _H_;            //add segment H
  return(d);  
}

//----------------------------------------------------------------------------
//--------------------------- Exported functions: ----------------------------
//----------------------------------------------------------------------------

//----------------------------- Display init: --------------------------------

void Disp_Init(void)
{
  LCD_WriteAddres(0x0F);     //write BLK register address
  LCD_WriteNibble(0x0F);     //0x0F - enable bus
  Disp_Clear();              //clear display
}

//---------------------------- Display update: -------------------------------

void Disp_Update(void)
{
  LCD_WriteAddres(0);        //write SG1 address
  for(char i = 0; i < LCD_SIZE; i++)
  {
    char d = Encode(Dig[i]);
    LCD_WriteNibble(d);      //write nibble from d to LCD
    d = __swap_nibbles(d);
    LCD_WriteNibble(d);      //write nibble from d to LCD
  }
  // output to USART
  char i, j;
  for(i = 0, j = 0; i < LCD_SIZE; i++)
  {
    char d = (Dig[i] & ~POINT);
    if(d < 10) Buf[j++] = d + 0x30; else Buf[j++] = d;
    if(Dig[i] & POINT) Buf[j++] = '.';
  }
  USART_Update(Buf, j); 
}

//---------------------------- Clear display: --------------------------------

void Disp_Clear(void)
{
  for(char i = 0; i < 10; i++)
    Dig[i] = ' ';
  Pos = 0;
  Disp_Update();
}

//----------------------------- Set position: --------------------------------

void Disp_SetPos(char p)
{
  if(p < 1) p = 1;
  if(p > LCD_SIZE) p = LCD_SIZE;
  Pos = p - 1;
}

// ---------------------------- Display char: --------------------------------

void Disp_PutChar(char c)
{
  if(Pos < LCD_SIZE)
  {
    Dig[Pos++] = c;
  }
}

// ---------------------- Display string from flash: -------------------------

void Disp_PutString(char __flash *s)
{
  while(*s)
  {
    Disp_PutChar(*s);
    s++;
  }
}

//----------------------------- Display value: -------------------------------

//s - start position 1..10
//p - point position 1..10, 0 - no point
//v - value ±1999999999

void Disp_Val(char s, char p, long v)
{
  char i; bool minus = 0;
  s--; p--; //align to 0..9 range
 if(v < 0) { v = -v; minus = 1; }
  Long2BCD(v, Bcd);
  //check for overflow:
  for(i = 0; i < s; i++)
  {
    if(Bcd[i]) break;
  }
  //if overflow, display dashes:
  if(i != s)
  {
    for(i = s; i < DIGITS; i++)
      Dig[i] = '-' | ((i == p)? POINT : 0);
  }
  //if not overflow, display value:
  else
  {
    char ch = ' ';
    for(i = s; i < DIGITS; i++)
    {
      char d = Bcd[i];
      if(minus && (ch == ' ') && (d || (i == p)))
        Dig[i - 1] = '-';
      if((ch != ' ') || d || (i == p)) ch = d;
      Dig[i] = ch | ((i == p)? POINT : 0);
    }
  }
}

//----------------------------------------------------------------------------
