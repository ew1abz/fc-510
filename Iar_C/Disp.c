//----------------------------------------------------------------------------

//Display support module

//----------------------------------------------------------------------------

#include "Main.h"
#include "Disp.h"
#include "Lcd.h"
#include "Port.h"

//----------------------------- Constants: -----------------------------------

#ifdef LCD16XX
  #define LCD_SIZE 16      //size of LCD16XX
#else
  #define LCD_SIZE 10      //size of LCD10
#endif
#define MSG_SIZE 16        //size of message string
#define DIGITS   10        //number of BCD digits 

//------------------------------ Variables: ----------------------------------

static char Msg[MSG_SIZE]; //message string
static char Bcd[DIGITS];   //hex to bcd buffer
static char Pos;           //current position
#ifdef LCD10
  static char SkipPos;     //skipped position
#endif

//------------------------- Function prototypes: -----------------------------

void Long2BCD(unsigned long x, char *buff); //convert long to BCD

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
  LCD_Init();
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
  char c = Msg[0];
  if(Msg[1] == 'F') c = 'F';
  if(Msg[1] == 'r') c = ' ';
  switch(c)
  {
  case 'f':
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
  //load display:
  LCD_Pos(1);
  char ptr = 0;
  for(char i = 0; i < LCD_SIZE; i++)
  {
#ifdef LCD1601  
    if(i == 8) LCD_Pos(9);
#endif
    char s = Msg[ptr++];
#ifdef LCD10
    if(s == 'f') s = 'F' + POINT;
    if((ptr < MSG_SIZE) && (Msg[ptr] == '.'))
    {
      s = s + POINT;
      ptr++;
    }
    if(ptr == SkipPos) s = Msg[ptr++];
#endif
    LCD_WrData(s);    
  }
  Pos = pos;
  Port_StartTX(); //request to TX 
}

//---------------------------- Clear display: --------------------------------

void Disp_Clear(void)
{
  for(char i = 0; i < MSG_SIZE; i++)
    Msg[i] = ' ';
  Pos = 0;
#ifdef LCD10
  SkipPos = MSG_SIZE;
#endif    
  Disp_Update();
}

//----------------------------- Set position: --------------------------------

void Disp_SetPos(char p)
{
  if(p < 1) p = 1;
  if(p > MSG_SIZE) p = MSG_SIZE;
  Pos = p - 1;
}

// ---------------------------- Display char: --------------------------------

void Disp_PutChar(char c)
{
  if(Pos < MSG_SIZE)
  {
    Msg[Pos++] = c;
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
#ifdef LCD10
  SkipPos = s; //save skipped position
#endif  
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
      Msg[n++] = '-';
      if(i == p) Msg[n++] = '.'; //insert point
  }
  //if not overflow, display value:
  else
  {
    char ch = ' ';
    for(i = s; i < DIGITS; i++)
    {
      char d = Bcd[i];
      if(minus && (ch == ' ') && (d || (i == p)))
        Msg[n - 1] = '-';
      if((ch != ' ') || d || (i == p))
        ch = d + 0x30;
      Msg[n++] = ch;
      if(i == p) Msg[n++] = '.'; //insert point
    }
  }
}

//--------------------- Get char from message buffer: ------------------------

char Disp_GetChar(char n)
{
  return(Msg[n]);
}

//----------------------------------------------------------------------------
