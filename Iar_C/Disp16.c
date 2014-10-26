//----------------------------------------------------------------------------

//LCD 1601 support module

//----------------------------------------------------------------------------

#include "Main.h"
#include "Disp.h"
#include "Usart.h"

//----------------------------- Constants: -----------------------------------

//#define LCD1601            //LCD 1601 addressing mode: 00..07, 40..47

#define LCD_SIZE 16        //size of LCD
#define DIGITS   10        //number of BCD digits 

//------------------------------ Variables: ----------------------------------

static char Lcd[LCD_SIZE]; //display copy
static char Bcd[DIGITS];   //hex to bcd buffer
static char Pos;           //current position

//------------------------- Function prototypes: -----------------------------

void LCD_Wr4(char d);      //write nibble to LCD
void LCD_WrCmd(char d);    //write command to LCD
void LCD_WrData(char d);   //write data to LCD
void Delay_ms(int d);      //ms range delay
void LCD_Clear(void);      //LCD clear
void LCD_Pos(char pos);    //set LCD position
void Long2BCD(unsigned long x, char *buff); //convert long to BCD

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

//---------------------- Write data to LCD (RS = 1): -------------------------

void LCD_WrData(char d)
{
  LCD_Wr4(__swap_nibbles(d) | 0x10);
  Delay_us(10);
  LCD_Wr4(d | 0x10);
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

//----------------------------------------------------------------------------
//--------------------------- Exported functions: ----------------------------
//----------------------------------------------------------------------------

//----------------------------- Display init: --------------------------------

void Disp_Init(void)
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
  Disp_Clear();        //clear display
}

//---------------------------- Display update: -------------------------------

static __flash char Str_U[4][4] =
{
  {"kHz"}, //frequency units
  {"mS "}, //period units
  {"rpm"}, //RPM units
  {"   "}  //no units
};

void Disp_Update(void)
{
  char pos = Pos;
  //add units:
  Disp_SetPos(14);
  char c = Lcd[0];
  if(Lcd[1] == 'F') c = 'F';
  if(Lcd[1] == 'r') c = ' ';
  switch(c)
  {
  case 'F':
    Disp_PutString(Str_U[0]);
    break;
  case 'P':
  case 'H':
  case 'L':
  case 'G':
    Disp_PutString(Str_U[1]);
    break;
  case 'R':
    Disp_PutString(Str_U[2]);
    break;
  default:  
    Disp_PutString(Str_U[3]);
  }
  LCD_Pos(1);
  for(char i = 0; i < LCD_SIZE; i++)
  {
#ifdef LCD1601  
    if(i == 8) LCD_Pos(9);
#endif  
    LCD_WrData(Lcd[i]);
  }
  Pos = pos;
  USART_Update(Lcd, LCD_SIZE); //TxPtr = 0; //request to TX 
}

//---------------------------- Clear display: --------------------------------

void Disp_Clear(void)
{
  for(char i = 0; i < LCD_SIZE; i++)
    Lcd[i] = ' ';
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
    Lcd[Pos++] = c;
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
  char n = s + 1; //skip one position
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
      Lcd[n++] = '-';
      if(i == p) Lcd[n++] = '.';
  }
  //if not overflow, display value:
  else
  {
    char ch = ' ';
    for(i = s; i < DIGITS; i++)
    {
      char d = Bcd[i];
      if(minus && (ch == ' ') && (d || (i == p)))
        Lcd[n - 1] = '-';
      if((ch != ' ') || d || (i == p)) ch = d + 0x30;
      Lcd[n++] = ch;
      if(i == p) Lcd[n++] = '.';
    }
  }
}

//----------------------------------------------------------------------------
