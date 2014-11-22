//----------------------------------------------------------------------------

//Frequency Counter FC-510
//Menu implementation module

//----------------------------------------------------------------------------

#include "Main.h"
#include "Menu.h"
#include "Keyboard.h"
#include "Disp.h"
#include "Port.h"
#include "Sound.h"
#include "Count.h"

//----------------------------- Constants: -----------------------------------

#define SIGNATURE 0xBEDA //EEPROM signature
#define T_SPLASH    2500 //splash screen indication delay, ms
#define T_CALIB      300 //calibration update period, ms
#define T_AUTO      2000 //auto scale indication time, ms
#define T_BLINK      250 //display blink time, ms
#define AUTO_SCALE  0x80 //auto scale flag

enum
{
  MNU_NO,     //no menu code
  MNU_SPLASH, //splash screen menu code
  MNU_MAIN,   //main menu code
  MNU_AUTO,   //auto scale indication menu
  MNU_SETUP   //setup menu code
};

enum
{
  PAR_MODE, //mode parameter index
  PAR_GATE, //gate parameter index
  PAR_AVG,  //average parameter index
  PAR_IF,   //IF parameter index
  PAR_PRE,  //prescaler parameter index
  PAR_INT,  //interpolator parameter index
  PARAMS    //params count
};

#define PAR_GTE_MIN 10     //min gate time, ms
#define PAR_GTE_NOM 1000   //nom gate time, ms
#define PAR_GTE_MAX 10000  //max gate time, ms
#define PAR_AVG_MIN 1      //min averages
#define PAR_AVG_NOM 1      //nom averages
#define PAR_AVG_MAX 100    //max averages
#define PAR_IF_MIN -999999 //min IF frequency, x0.1 kHz
#define PAR_IF_NOM  0      //nom IF frequency, x0.1 kHz
#define PAR_IF_MAX  999999 //max IF frequency, x0.1 kHz
#define PAR_PRE_MIN 1      //min precsaler ratio
#define PAR_PRE_NOM 1      //nom precsaler ratio
#define PAR_PRE_MAX 1000   //max precsaler ratio
#define SCALE_NOM   5      //nom scale

//----------------------------- Variables: -----------------------------------

static char Menu;      //active menu index
static char DispMenu;  //displayed menu
static int  MenuTimer; //menu delay timer
static char Param;     //current editing param index
static char KeyCode;   //last pressed key code
static bool ReplayS;   //slow replay flag
static bool ReplayF;   //fast replay flag
static bool Hold;      //display hold flag
static bool Hide;      //display hide flag
static int  Calib;     //interpolator calibration value
static char Scale;     //current value scale

static char Par_Mode;  //Mode parameter
static int  Par_Gate;  //Gate parameter
static char Par_Avg;   //Average parameter
static long Par_IF;    //IF parameter
static int  Par_Pre;   //Precscaler parameter
static bool Par_Int;   //Interpolator parameter

__no_init __eeprom int  ESignature; //EEPROM signature
__no_init __eeprom char EPar_Mode;  //Mode in EEPROM
__no_init __eeprom int  EPar_Gate;  //Gate in EEPROM
__no_init __eeprom char EPar_Avg;   //Average in EEPROM
__no_init __eeprom long EPar_IF;    //IF in EEPROM
__no_init __eeprom int  EPar_Pre;   //Precscalerin EEPROM
__no_init __eeprom bool EPar_Int;   //Interpolator in EEPROM
__no_init __eeprom char EScale[MODES]; //Scales in EEPROM

//------------------------- Function prototypes: -----------------------------

void Mnu_Splash(bool ini);    //splash screen menu
void Mnu_Main(bool ini);      //main menu
void Mnu_Auto(bool ini);      //auto scale indication menu
void Mnu_Setup(bool ini);     //setup menu

void ParToEEPROM(void);       //save params to the EEPROM
void Show_Main(char n);       //show main menu
void Show_Setup(char m);      //show setup menu
bool MoveDP(char key);        //move DP
bool Param_Up(char m);        //step editing param up
bool Param_Dn(char m);        //step editing param down
bool Param_Reset(char m);     //reset param to nominal value

//------------------------------ Menu init: ----------------------------------

void Menu_Init(void)
{
  Disp_Init();                //display init
  Port_Init();                //port init
  Keyboard_Init();            //keyboard init

  if(ESignature != SIGNATURE) //check EEPROM signature,
  {
    Par_Mode = MODE_F;        //if error, init params
    Par_Gate = PAR_GTE_NOM;
    Par_Avg  = PAR_AVG_NOM;
    Par_IF   = PAR_IF_NOM;
    Par_Pre  = PAR_PRE_NOM;
    Par_Int  = 1;
    ParToEEPROM();            //EEPROM init
    for(char i = 0; i < MODES; i++)
    EScale[i] = SCALE_NOM;    //scales init
    Scale = SCALE_NOM;
  }
  else
  {
    Par_Mode = EPar_Mode;     //read params from EEPROM
    Par_Gate = EPar_Gate;
    Par_Avg  = EPar_Avg;
    Par_IF   = EPar_IF;
    Par_Pre  = EPar_Pre;
    Par_Int  = EPar_Int;
    Scale = EScale[Par_Mode];
  }
  Count_SetMode(Par_Mode);    //set counter mode
  Count_SetGate(Par_Gate);    //set gate time
  Count_SetAvg(Par_Avg);      //set number of averages
  Count_SetIF(Par_IF);        //set IF
  Count_SetPre(Par_Pre);      //set prescaler ratio
  Count_SetInt(Par_Int);      //interpolator enable/disable

  Count_SetScale(Scale);      //set output data scale factor
  Count_Start();              //start counter

  DispMenu = MNU_NO;          //no menu
  Hold = 0;                   //no hold mode
  Hide = 0;                   //display not hided
  Menu = MNU_SPLASH;          //request splash screen menu
}

//----------------------------- Menu execute: --------------------------------

void Menu_Exe(bool t)
{
  Port_Exe(t);                    //TX display copy via UART
  Keyboard_Exe(t);                //scan keyboard
  if(t)
  {
    if(MenuTimer) MenuTimer--;    //menu timer processing
  }
  KeyCode = Keyboard_GetCode();   //read key code
  ReplayS = KeyCode & REP_S;      //set/clear slow replay flag
  ReplayF = KeyCode & REP_F;      //set/clear fast replay flag
  KeyCode &= ~(REP_S | REP_F);    //clear replay flags in key code
  if(KeyCode && !ReplayS && !ReplayF)
    Sound_Beep();                 //beep if not replay

  bool MnuIni = Menu != DispMenu; //need menu draw flag
  switch(Menu)                    //execute menus
  {
  case MNU_SPLASH:  Mnu_Splash(MnuIni); break; //splash screen menu
  case MNU_MAIN:    Mnu_Main(MnuIni);   break; //main menu
  case MNU_AUTO:    Mnu_Auto(MnuIni);   break; //auto scale menu
  case MNU_SETUP:   Mnu_Setup(MnuIni);  break; //setup menu
  }

  if(!ReplayS && !ReplayF && (KeyCode != KEY_NO)) //if key not processed
    Sound_Bell();                    //error bell
}

//------------------------- Splash screen menu: ------------------------------

void Mnu_Splash(bool ini)
{
  static char __flash Mnu_Str[] = "  FC-510  ";
  //draw menu:
  if(ini)                        //if redraw needed
  {
    Disp_PutString(Mnu_Str);     //show menu text
    Disp_Update();               //display update
    Sound_Beep();                //beep
    MenuTimer = ms2sys(T_SPLASH); //load menu timer
    DispMenu = Menu;             //menu displayed
  }
  //menu timer check:
  if(!MenuTimer)                 //timer overflow,
  {
    Sound_Beep();                //beep
    Menu = MNU_MAIN;             //go to main menu
  }
  //check any key press:
  if(KeyCode != KEY_NO)          //any key
  {
    MenuTimer = 0;               //timer clear
    Sound_Beep();                //beep
    KeyCode = KEY_NO;            //key code processed
    Menu = MNU_MAIN;             //go to main menu
  }
}

//----------------------------- Main menu: -----------------------------------

void Mnu_Main(bool ini)
{
  //draw menu:
  if(ini)                        //if redraw needed
  {
    Show_Main(Par_Mode);         //draw main menu
    MenuTimer = ms2sys(T_BLINK); //load blink timer
    DispMenu = Menu;             //menu displayed
  }
  //blink display in hold mode:
  if(Hold && !MenuTimer)
  {
    Hide = !Hide;
    DispMenu = MNU_NO;           //redraw menu
  }
  //display measured value:
  if(Count_Ready())              //check counter ready
  {
    DispMenu = MNU_NO;           //redraw menu
  }
  //MENU key:
  if(KeyCode == KEY_MN)
  {
    if(Hold)
    {
      Hold = 0;                  //off hold mode
      Hide = 0;                  //show display
      Count_Start();             //start counter
      DispMenu = MNU_NO;         //redraw menu
    }
    else
    {
      Param = PAR_MODE;          //reset param index
      Menu = MNU_SETUP;          //go to setup menu
    }
    KeyCode = KEY_NO;            //key code processed
  }
  //OK key:
  if(KeyCode == KEY_OK)
  {
    Hold = !Hold;                //on/off hold mode
    Hide = Hold;                 //hide display value if hold
    if(Hold)
    {
      Count_Stop();              //stop counter if hold
    }
    else
    {
      Count_ClearStat();         //clear statistics
      Count_Start();             //start counter
    }
    DispMenu = MNU_NO;           //redraw menu
    KeyCode = KEY_NO;            //key code processed
  }
  //DOWN key:
  if(KeyCode != KEY_NO)
  {
    if(MoveDP(KeyCode))          //shift DP
    {
      Count_SetScale(Scale);     //set new scale
      EScale[Par_Mode] = Scale;  //save scale to EEPROM
      DispMenu = MNU_NO;         //redraw menu
      if(KeyCode == KEY_UD)      //UP + DOWN pressed,
        Menu = MNU_AUTO;         //go to auto scale menu
      KeyCode = KEY_NO;          //key code processed
    }
  }
}

//-------------------------------- Auto: -------------------------------------

void Mnu_Auto(bool ini)
{
  static char __flash Str_Auto1[] = " Auto On  ";
  static char __flash Str_Auto0[] = " Auto Off ";
  //draw menu:
  if(ini)                        //if redraw needed
  {
    Disp_Clear();                //clear display
    if(Scale & AUTO_SCALE)       //show menu text
      Disp_PutString(Str_Auto1);
        else Disp_PutString(Str_Auto0);
    Disp_Update();               //display update
    MenuTimer = ms2sys(T_AUTO);  //load menu timer
    DispMenu = Menu;             //menu displayed
  }
  //block keys:
  KeyCode = KEY_NO;              //key code processed
  //check timer:
  if(!MenuTimer)                 //timer overflow,
  {
    Sound_Beep();                //beep
    Menu = MNU_MAIN;             //go to main menu
  }
}

//----------------------------- Setup menu: ----------------------------------

void Mnu_Setup(bool ini)
{
  //draw menu:
  if(ini)                        //if redraw needed
  {
    Count_Stop();                //stop counter
    Show_Setup(Param);           //draw setup menu
    if(Param == PAR_INT)         //if interpolator par,
    {
      Count_StartCalib();        //start interpolator calibration
      MenuTimer = ms2sys(T_CALIB); //reload timer
    }
    DispMenu = Menu;             //menu displayed
  }
  //interpolator calibration timer check:
  if(Param == PAR_INT && !MenuTimer)
  {
    Calib = Count_GetCalib();    //read calibration value
    DispMenu = MNU_NO;           //redraw menu
  }
  //MENU key:
  if(KeyCode == KEY_MN)
  {
    if(++Param == PARAMS)        //new param
    {
      ParToEEPROM();             //save parameters to EEPROM
      Count_Start();             //start counter
      Menu = MNU_MAIN;           //go to main menu
    }
    DispMenu = MNU_NO;           //redraw menu request
    KeyCode = KEY_NO;            //key code processed
  }
  //DOWN key:
  if(KeyCode == KEY_DN)
  {
    if(Param_Dn(Param))          //step param down
    {
      DispMenu = MNU_NO;         //redraw menu request
      KeyCode = KEY_NO;          //key code processed
    }
  }
  //UP key:
  if(KeyCode == KEY_UP)
  {
    if(Param_Up(Param))          //step param up
    {
      DispMenu = MNU_NO;         //redraw menu request
      KeyCode = KEY_NO;          //key code processed
    }
  }
  //UP + DOWN key:
  if(KeyCode == KEY_UD)
  {
    if(Param_Reset(Param))       //reset param value
    {
      DispMenu = MNU_NO;         //redraw menu request
      KeyCode = KEY_NO;          //key code processed
    }
  }
  //OK key:
  if(KeyCode == KEY_OK)
  {
    ParToEEPROM();               //save parameters to EEPROM
    Count_Start();               //start counter
    Menu = MNU_MAIN;             //go to main menu
    KeyCode = KEY_NO;            //key code processed
  }
}

//----------------------------------------------------------------------------
//------------------------ Additional functions: -----------------------------
//----------------------------------------------------------------------------

//---------------------- Save params to the EEPROM: --------------------------

void ParToEEPROM(void)
{
  if(EPar_Mode != Par_Mode)   EPar_Mode  = Par_Mode;
  if(EPar_Gate != Par_Gate)   EPar_Gate  = Par_Gate;
  if(EPar_Avg  != Par_Avg)    EPar_Avg   = Par_Avg;
  if(EPar_IF != Par_IF)       EPar_IF    = Par_IF;
  if(EPar_Pre != Par_Pre)     EPar_Pre   = Par_Pre;
  if(EPar_Int != Par_Int)     EPar_Int   = Par_Int;
  if(ESignature != SIGNATURE) ESignature = SIGNATURE;
}

//--------------------------- Show main menu: --------------------------------

static __flash char Str_V[MODES][4] =
{
  "F  ", //frequency
  "FIF", //frequency ± IF
  "P  ", //period
  "HI ", //high level duration
  "LO ", //low lewel duration
  "D  ", //duty cycle
  "Rot", //RPM
  "FH ", //maximum frequency
  "FL ", //minimum frequency
  "dF "  //variation of frequency
};

void Show_Main(char n)
{
  long min, max; char s;
  Disp_Clear();                       //clear display
  if(n == MODE_FIF)
    Disp_PutChar('f');                //display "f"
      else Disp_PutString(Str_V[n]);  //show value name
  long v = Count_GetValue();          //read counter
  if(Count_Ready()) Count_Start();    //start counter

  char p = (Scale & ~AUTO_SCALE) + 2; //DP position
  switch(Par_Mode)
  {
  case MODE_F:
  case MODE_FIF:
  case MODE_P:
  case MODE_D:
    s = 3;
    min = 10000000L;
    max = 99999999L;
    break;
  case MODE_R:
    s = 5;
    min = 100000L;
    max = 999999L;
    break;
  default:
    s = 4;                            //first position
    min = 1000000L;                   //min value for auto scale
    max = 9999999L;                   //max value for auto scale
  }

  if(!Hide) Disp_Val(s, p, v);        //show value
  Disp_Update();                      //update display

  if(Scale & AUTO_SCALE)              //if auto scale
  {
    if(v >= 0)
    {
      if(v < min) MoveDP(KEY_DN);     //move DP left
      if(v > max) MoveDP(KEY_UP);     //move DP right
    }
    else
    {
      if(v > -min) MoveDP(KEY_DN);    //move DP left
      if(v < -max) MoveDP(KEY_UP);    //move DP right
    }
    Scale |= AUTO_SCALE;              //restore auto scale flag
    Count_SetScale(Scale);            //update scale
  }
}

//-------------------------- Show setup menu: --------------------------------

static __flash char Str_P[PARAMS][4] =
{
  "Ind", //indication mode
  "G  ", //gate
  "A  ", //average
  "IF ", //IF frequency
  "Pre", //prescaler ratio
  "Int"  //interpolator on/off
};

static __flash char Str_On[4] = "On ";
static __flash char Str_Off[4] = "Off";

void Show_Setup(char m)
{
  Disp_Clear();                       //clear display
  Disp_PutString(Str_P[m]);           //show param name

  switch(m)
  {
  case PAR_MODE:
    Disp_SetPos(5);                   //set display position
    Disp_PutString(Str_V[Par_Mode]);  //show value name
    break;
  case PAR_GATE:
    Disp_Val(5, 0, Par_Gate);         //show Gate value
    break;
  case PAR_AVG:
    Disp_Val(5, 0, Par_Avg);          //show Average value
    break;
  case PAR_IF:
    Disp_Val(5, 9, Par_IF);           //show IF value
    break;
  case PAR_PRE:
    Disp_Val(5, 0, Par_Pre);          //show prescaler ratio
    break;
  case PAR_INT:
    Disp_SetPos(5);                   //set display position
    if(!Par_Int)
    {
      Disp_PutString(Str_Off);        //show interpolator state
    }
    else
    {
      Disp_PutString(Str_On);         //show interpolator state
      Disp_Val(7, 0, Calib);          //show interpolator calibration value
    }
    break;
  }
  Disp_Update();                      //update display
}

//------------------------------- Move DP: -----------------------------------

bool MoveDP(char key)
{
  char s = Scale & ~AUTO_SCALE;       //scale code
  bool a = Scale & AUTO_SCALE;        //auto scale flag

  if(key == KEY_DN) { a = 0; s--; }   //scale left
  if(key == KEY_UP) { a = 0; s++; }   //scale right
  if(key == KEY_UD) { a = !a; }       //invert auto scale flag

  if(s > 7) s = 7;                    //limit scale code in top
  switch(Par_Mode)                    //limit scale code in bot
  {
  case MODE_F:
  case MODE_FIF:
  case MODE_P:
  case MODE_D: if(s < 1) s = 1; break;
  case MODE_R: if(s < 3) s = 3; break;
  default: if(s < 2) s = 2;
  }
  char sc = s | (a? AUTO_SCALE : 0);  //combine scale code and auto flag
  if(Scale == sc) return(0);          //no changes, return
  Scale = sc;                         //save new scale value
  return(1);
}

//-------------------------- Change param up: --------------------------------

bool Param_Up(char m)
{
  switch(m)
  {
  case PAR_MODE:                      //change mode
    {
      if(Par_Mode < MODES - 1)
        Par_Mode++;
          else Par_Mode = 0;
      Scale = EScale[Par_Mode];
      Count_SetMode(Par_Mode);
      Count_SetScale(Scale);
      return(1);
    }
  case PAR_GATE:                      //change gate
    {
      if(Par_Gate < PAR_GTE_MAX)
      {
        if(Par_Gate == 20 ||
           Par_Gate == 200 ||
           Par_Gate == 2000)
          Par_Gate = Par_Gate * 2 + Par_Gate / 2; //* 2.5
            else Par_Gate = Par_Gate * 2;         //* 2
        Count_SetGate(Par_Gate);
        return(1);
      }
      break;
    }
  case PAR_AVG:                       //change average
    {
      if(Par_Avg < PAR_AVG_MAX)
      {
        if(Par_Avg == 2 ||
           Par_Avg == 20)
          Par_Avg = Par_Avg * 2 + Par_Avg / 2; //* 2.5
            else Par_Avg = Par_Avg * 2;        //* 2
        Count_SetAvg(Par_Avg);
        return(1);
      }
      break;
    }
  case PAR_IF:                        //change IF
    {
      if(Par_IF < PAR_IF_MAX)
      {
        if(!ReplayF) Par_IF = Par_IF + 1;
          else Par_IF = Par_IF + 100;
        Count_SetIF(Par_IF);
        return(1);
      }
      break;
    }
  case PAR_PRE:                       //change prescaler ratio
    {
      if(Par_Pre < PAR_PRE_MAX)
      {
        Par_Pre = Par_Pre + 1;
        Count_SetPre(Par_Pre);
        return(1);
      }
      break;
    }
  case PAR_INT:                       //change interpolator state
    {
      if(!Par_Int)
      {
        Par_Int = 1;                  //interpolator on
        Count_SetInt(Par_Int);
        return(1);
      }
      break;
    }
  }
  return(0);
}

//-------------------------- Change param down: ------------------------------

bool Param_Dn(char m)
{
  switch(m)
  {
  case PAR_MODE:                      //change mode
    {
      if(Par_Mode > 0)
        Par_Mode--;
          else Par_Mode = MODES - 1;
      Scale = EScale[Par_Mode];
      Count_SetMode(Par_Mode);
      Count_SetScale(Scale);
      return(1);
    }
  case PAR_GATE:                      //change gate
    {
      if(Par_Gate > PAR_GTE_MIN)
      {
        if(Par_Gate == 50 ||
           Par_Gate == 500 ||
           Par_Gate == 5000)
          Par_Gate = (Par_Gate * 2) / 5;  // / 2.5
            else Par_Gate = Par_Gate / 2; // / 2
        Count_SetGate(Par_Gate);
        return(1);
      }
      break;
    }
  case PAR_AVG:                       //change average
    {
      if(Par_Avg > PAR_AVG_MIN)
      {
        if(Par_Avg == 5 ||
           Par_Avg == 50)
          Par_Avg = (Par_Avg * 2) / 5;  // / 2.5
            else Par_Avg = Par_Avg / 2; // / 2
        Count_SetAvg(Par_Avg);
        return(1);
      }
      break;
    }
  case PAR_IF:                        //change IF
    {
      if(Par_IF > PAR_IF_MIN)
      {
        if(!ReplayF) Par_IF = Par_IF - 1;
          else Par_IF = Par_IF - 100;
        Count_SetIF(Par_IF);
        return(1);
      }
      break;
    }
  case PAR_PRE:                       //change prescaler ratio
    {
      if(Par_Pre > PAR_PRE_MIN)
      {
        Par_Pre = Par_Pre - 1;
        Count_SetPre(Par_Pre);
        return(1);
      }
      break;
    }
  case PAR_INT:                       //change interpolator state
    {
      if(Par_Int)
      {
        Par_Int = 0;                  //interpolator off
        Count_SetInt(Par_Int);
        return(1);
      }
      break;
    }
  }
  return(0);
}

//----------------------------- Param reset: ---------------------------------

bool Param_Reset(char m)
{
  switch(m)
  {
  case PAR_GATE:                      //reset gate to nominal value
    Par_Gate = PAR_GTE_NOM;
    Count_SetGate(Par_Gate);
    return(1);
  case PAR_AVG:                       //reset average to nominal value
    Par_Avg = PAR_AVG_NOM;
    Count_SetAvg(Par_Avg);
    return(1);
  case PAR_IF:                        //reset IF to nominal value
    Par_IF = PAR_IF_NOM;
    Count_SetIF(Par_IF);
    return(1);
  case PAR_PRE:                       //reset prescaler to nominal value
    Par_Pre = PAR_PRE_NOM;
    Count_SetPre(Par_Pre);
    return(1);
  }
  return(0);
}


//----------------------------------------------------------------------------


