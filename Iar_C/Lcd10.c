//----------------------------------------------------------------------------

//LCD MT10-T7 support module

//----------------------------------------------------------------------------

#include "Main.h"
#include "Lcd.h"

//----------------------------- Constants: -----------------------------------

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

//------------------------- Function prototypes: -----------------------------

void LCD_Write(char d);       //write data to LCD
void LCD_WriteNibble(char d); //write nibble to LCD
void LCD_WriteAddres(char d); //write address to LCD
char Encode(char s);          //encode symbol to 7-segment code

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
  case 'F':
  case 'f': d = 0x0F; break; //character "F"
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
  case 'u': d = 0x18; break; //character "t"
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

//-------------------------------- LCD init: ---------------------------------

void LCD_Init(void)
{
  LCD_WriteAddres(0x0F);     //write BLK register address
  LCD_WriteNibble(0x0F);     //0x0F - enable bus
}

//--------------------------- Set LCD position: ------------------------------

//pos = 1..10

void LCD_Pos(char pos)
{
  pos = pos - 1;
  LCD_WriteAddres(pos);      //write SG1 address
}

//---------------------- Write data to LCD (RS = 1): -------------------------

void LCD_WrData(char d)
{
  char c = Encode(d);
  LCD_WriteNibble(c);        //write nibble from d to LCD
  c = __swap_nibbles(c);
  LCD_WriteNibble(c);        //write nibble from d to LCD
}

//----------------------------------------------------------------------------
