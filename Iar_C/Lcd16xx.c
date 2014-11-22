//----------------------------------------------------------------------------

//LCD 1601/1602 support module

//----------------------------------------------------------------------------

#include "Main.h"
#include "Lcd.h"

//------------------------- Function prototypes: -----------------------------

void LCD_Wr4(char d);      //write nibble to LCD
void LCD_WrCmd(char d);    //write command to LCD
void Delay_ms(int d);      //ms range delay
void LCD_Clear(void);      //LCD clear

//------------------------ Write nibble to LCD: ------------------------------

void LCD_Wr4(char d)
{
  for(char i = 0; i < 5; i++)
  {
    Port_SCLK_0;
    if(d & 0x10) Port_DATA_1;
      else Port_DATA_0;
    d = d << 1;
    Port_SCLK_1;
  }
  Port_LOAD_1;        //E <- 1
  Port_DATA_1;
  Delay_us(2);        //delay 2 uS
  Port_LOAD_0;        //E <- 0
}

//--------------------- Write command to LCD (RS = 0): -----------------------

void LCD_WrCmd(char d)
{
  LCD_Wr4(__swap_nibbles(d) & 0x0F);
  Delay_us(10);
  LCD_Wr4(d & 0x0F);
  Delay_us(50);
}

//-------------------------- ms range delay: ---------------------------------

void Delay_ms(int d)
{
  while(d)
  {
    Delay_us(1000);
    __watchdog_reset();
    d--;
  }
}

//------------------------------ LCD clear: ----------------------------------

void LCD_Clear(void)
{
  LCD_WrCmd(0x01);    //DISPLAY CLEAR
  Delay_ms(5);        //delay >1.64mS
}

//----------------------------------------------------------------------------
//--------------------------- Exported functions: ----------------------------
//----------------------------------------------------------------------------

//-------------------------------- LCD init: ---------------------------------

void LCD_Init(void)
{
  Port_LOAD_0;         //E <- 0
  Delay_ms(15);
  LCD_WrCmd(0x30);
  Delay_ms(5);         //delay >4.1 mS
  LCD_WrCmd(0x30);
  Delay_us(100);       //delay >100 uS
  LCD_WrCmd(0x30);
  Delay_ms(5);         //delay >4.1 mS
  LCD_WrCmd(0x20);     //FUNCTION SET (8 bit)
  Delay_ms(15);
  LCD_WrCmd(0x28);     //FUNCTION SET (4 bit)
  Delay_ms(15);
  LCD_WrCmd(0x0C);     //DISPLAY ON
  Delay_ms(15);
  LCD_WrCmd(0x06);     //ENTRY MODE SET
  Delay_ms(15);
  LCD_Clear();         //CLEAR
  Delay_ms(15);
}

//--------------------------- Set LCD position: ------------------------------

//pos = 1..16

void LCD_Pos(char pos)
{
  pos = pos - 1;
#ifdef LCD1601  
  if(pos > 7)
    pos = (pos & 0x07) | 0x40;
#endif  
  pos = pos | 0x80;
  LCD_WrCmd(pos);
}

//---------------------- Write data to LCD (RS = 1): -------------------------

void LCD_WrData(char d)
{
  LCD_Wr4(__swap_nibbles(d) | 0x10);
  Delay_us(10);
  LCD_Wr4(d | 0x10);
  Delay_us(50);
}

//----------------------------------------------------------------------------
