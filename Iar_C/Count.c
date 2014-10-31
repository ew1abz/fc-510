//----------------------------------------------------------------------------

//Frequency Counter FC-510
//count module

//----------------------------------------------------------------------------

#include "Main.h"
#include "Count.h"

//----------------------------- Constants: -----------------------------------

#define N_CALIB    5 //pre- and post- calibrate cycles count
#define T_GATE  1000 //default gate time, mS
#define T_PAUSE  100 //default pause time, mS
#define AVERAGE    1 //default averages number
#define MAX_AVG  100 //max number of averages
#define IFREQ      0 //default IF value
#define PRESCALE   1 //default prescaler ratio
#define INT_EN     1 //default interpolator enable

#define NLR       16 //window for non-linear filter, E-1 (±6.25% for NLR = 16)
#define SCALE      1 //default scale

#define F_REF ((unsigned long long)(F_CLK * 1E6)) //reference frequency, Hz

//Counter states:

enum
{
  ST_STOP,   //counter is stopping
  ST_START,  //counter is starting
  ST_PAUSE,  //counter waits for pause
  ST_WAIT,   //counter waits for input signal
  ST_COUNT,  //count in progress
  ST_FINISH, //counter completes count
  ST_READY,  //counter ready
  ST_ERROR,  //counter error
  ST_CALIB   //interpolator calibration
};

//----------------------------- Variables: -----------------------------------

static char Count_N;           //MCU input pulses count
static int Count_M;            //MCU reference pulses count
static long Count_Nx;          //total input pulses count
static long Count_Mx;          //total reference pulses count
static int Count_Ix;           //interpolator count
static int Cal;                //interpolator calibration value
static int Cnt_Timer;          //counter timer
static long long Freq;         //current  frequency
static bool First;             //first measure flag
static long long Fmax;         //statistics: max frequency
static long long Fmin;         //statistics: min frequency
static long long Fnom;         //statistics: nom frequency
static long long Fdev;         //statistics: dev frequency

static char State;             //counter state
static char Mode;              //counter mode
static char Scale;             //output value scale
static long Avg[MAX_AVG];      //averaging filter array
static long long AvgVal;       //sum averaging value
static bool DutyH;             //duty H-pulse measure phase
static long PulseH;            //duty H-pulse width
static long PulseL;            //duty L-pulse width

static int  T_Gate;            //gate time, ms
static int  T_Pause;           //pause time, ms
static char Average;           //number of averages
static char AvgPnt;            //averaging filter pointer
static long IFreq;             //IF value
static int  Prescale;          //prescaler ratio
static bool Interpolate;       //interpolator enable

//------------------------- Function prototypes: -----------------------------

void Count_Clear(void);        //clear counters
int Count_Calib(char n);       //calibrate interpolator
#pragma vector = TIMER0_OVF_vect
__interrupt void Timer0(void); //timer 0 overflow interrupt (reference)
#pragma vector = TIMER1_OVF_vect
__interrupt void Timer1(void); //timer 1 overflow interrupt (input)
char Get_CPLD(void);           //read data byte from CPLD
void Count_Read(void);         //read counters
void Count_Make(void);         //calculate frequency
void PresetFilter(long v);     //preset averaging filter array

//------------------------- Counter module init: -----------------------------

void Count_Init(void)
{
  ACSR = (1 << ACD);                    //analog comparator disable
  TCCR0 = (1 << CS02) | (1 << CS01);    //timer 0 <- T0, fall edge
  TCCR1B = (1 << CS12) | (1 << CS11);   //timer 1 <- T1, fall edge
  TIFR = (1 << TOIE1) | (1 << TOIE0);   //clear pending interrupts
  TIMSK |= (1 << TOIE1) | (1 << TOIE0); //OVF0 and OVF1 interrupts enable

  Mode = MODE_F;             //default mode
  Count_SetMode(Mode);       //set mode
  Count_ClearStat();         //statistics clear

  T_Gate  = ms2sys(T_GATE);  //load default gate time
  T_Pause = ms2sys(T_PAUSE); //load default pause time
  Average = AVERAGE;         //load default number of averages
  IFreq = IFREQ;             //load default IF value
  Prescale = PRESCALE;       //load default prescaler ratio
  Interpolate = INT_EN;      //load default interpolator state

  Scale = SCALE;             //default scale
  First = 1;                 //first measure flag set
  State = ST_STOP;           //counter stopped
}

//---------------------------- Count process: --------------------------------

void Count_Exe(bool t)
{
  if(t)
  {
    if(Cnt_Timer) Cnt_Timer--;

    switch(State)
    {
    case ST_START:            //START state:
      {
        Cnt_Timer = T_Pause;  //load pause interval
        State = ST_PAUSE;     //switch to PAUSE state
        break;
      }
    case ST_PAUSE:            //PAUSE state:
      {
        if(Cnt_Timer) break;  //wait for pause time
        Cal = Count_Calib(N_CALIB); //pre-calibration
        Count_Clear();        //counters clear
        Port_GATE_1;          //enable count
        Cnt_Timer = T_Gate;   //load gate interval
        State = ST_WAIT;      //switch to WAIT state
        break;
      }
    case ST_WAIT:             //WAIT state:
      {
        if(Pin_SDATA)         //check for start:
        {                     //start occurs,
          Port_LED_1;         //GATE LED on
          Cnt_Timer = T_Gate; //reload gate interval
          State = ST_COUNT;   //switch to COUNT state
        }
        else                  //no start
        {                     //check for wait interval over
          if(!Cnt_Timer)      //check for wait time
            State = ST_ERROR; //no signal - error
        }
        break;
      }
    case ST_COUNT:            //COUNT state:
      {
        if(!Cnt_Timer)        //check for gate time
        {                     //if count time is over
          Port_GATE_0;        //disable count
          Cnt_Timer = T_Gate; //reload gate interval
          State = ST_FINISH;  //switch to FINISH state
        }
        break;
      }
    case ST_FINISH:           //FINISH state:
      {
        if(!Pin_SDATA)        //check for count complete
        {                     //count is over
          Port_LED_0;         //GATE LED off
          Count_Read();       //read counters
          Cal += Count_Calib(N_CALIB); //post-calibration
          Count_Make();       //calculate frequency
          State = ST_READY;   //switch to READY state
        }
        else                  //count not over
        {                     //check for complete interval over
          if(!Cnt_Timer)      //check for complete time
            State = ST_ERROR; //no necessary slope, error
        }
        break;
      }
    case ST_CALIB:            //CALIB state:
      {
        Port_LED_1;           //GATE LED on
        Cal = Count_Calib(N_CALIB); //calibration
        Port_LED_0;           //GATE LED off
        State = ST_STOP;      //switch to STOP state
        break;
      }
    }
    if(State == ST_ERROR)     //if error
    {
      Port_GATE_0;            //disable count
      Port_LED_0;             //GATE LED off
      Freq = PulseH = PulseL = 0; //clear count
      State = ST_READY;       //switch to READY state
    }
  }
}

//--------------------------- Clear counters: --------------------------------

void Count_Clear(void)
{
  Port_RESET_0; //reset CPLD
  TCNT0 = 0;    //timer 0 clear
  TCNT1 = 0;    //timer 1 clear
  Count_M = 0;  //reference pulse count clear
  Count_N = 0;  //input pulse count clear
  Port_RESET_1; //release CPLD reset
}

//---------- Timer 0 overflow interrupt (reference pulses count): ------------

#pragma vector = TIMER0_OVF_vect
__interrupt void Timer0(void)
{
  Count_M++;
}

//------------ Timer 1 overflow interrupt (input pulses count): --------------

#pragma vector = TIMER1_OVF_vect
__interrupt void Timer1(void)
{
  Count_N++;
}

//-------------------------- Read data from CPLD: ----------------------------

char Get_CPLD(void)
{
  char d = 0;
  for(char i = 0; i < 8; i++) //bit count
  {
    d = d >> 1;               //shift data byte
    Port_SCLOCK_1;            //clock 0 -> 1
    __delay_cycles(2);        //set-up delay
    Port_SCLOCK_0;            //clock 1 -> 0
    if(Pin_SDATA) d |= 0x80;  //shift data bit
  }
  return(d);
}

//------------------------ Calibrate interpolator: ---------------------------

int Count_Calib(char n)
{
  signed char d;
  int cal = 0;
  for(char i = 0; i < n; i++)
  {
    Count_Clear();  //counters clear

    Port_CALIB_1;   //start calibrate
    Delay_us(25);   //delay for interpolator
    Port_CALIB_0;   //stop calibrate
    Delay_us(25);   //delay for interpolator

    Port_FSYNC_0;
    d = Get_CPLD(); //dummy read M0
    d = Get_CPLD(); //dummy read N0
    d = Get_CPLD(); //read Cal
    Port_FSYNC_1;
    cal += d;       //summ calibration values
  }
  return(cal);
}

//-------------------------- Read counter value: -----------------------------

//updates Count_Mx, Count_Nx, Count_Ix

void Count_Read(void)
{
  Port_FSYNC_0;
  char CntM0 = Get_CPLD(); //read M0
  char CntN0 = Get_CPLD(); //read N0
  Count_Ix = (signed char)Get_CPLD(); //read interpolator
  Port_FSYNC_1;

  //Total reference pulse number:
  //12.8 MHz * 10 s * 2 (finishing) = 256000000 (F 42 40 00)
  Count_Mx = ((long)Count_M << 16) + ((long)TCNT0 << 8) + CntM0;
  //Total input pulse number:
  //100 MHz max * 10 s = 1000000000 (3B 9A CA 00)
  Count_Nx = ((long)Count_N << 24) + ((long)TCNT1 << 8) + CntN0;
}

//------------------------- Calculate frequency: -----------------------------

//updates Freq, Fmin, Fmax, Fdev, PulseH, PulseL

void Count_Make(void)
{
  //Scale pulse number:
  //2 GHz max * 10 s * 12800000 Hz = 256000000000000000 (3 8D 7E A4 C6 80 00 00)
  int pre = Pin_FDIV? Prescale : 1;
  long long Nx = (long long)Count_Nx * F_REF * pre;

  //save pulse width for duty cycle calculation:
  if(DutyH) PulseH = Count_Mx;
    else PulseL = Count_Mx;

  //scale to 1/100 of resolution:
  //256000000 (F 42 40 00) * 100 = 25600000000 (5 F5 E1 00 00)
  long long Mx = (long long)Count_Mx * 100;

  //Calculate total interpolated pulse number, scaled by 100:
  //127 * 100 * 2 * 5 (nom) = 127000
  if(Interpolate && !((Mode == MODE_HI) || (Mode == MODE_LO)))
    Mx = Mx + ((long)Count_Ix * (100 * 2 * N_CALIB)) / (-Cal);

  //frequency calculation (x100 uHz):
  //2 GHz max with prescaler = 20000000000000 x100 uHz (12 30 9C E5 40 00)
  Freq = 0;
  if((Mode == MODE_HI) || (Mode == MODE_LO) || (Mode == MODE_P))
  {
    //calculate pulse width:
    Freq = Mx * 100000000 / (long long)Count_Nx / F_REF / pre;
  }
  else
  {
    if(Nx && Mx)
    {
           if(Nx < 0x7FFFFFFFFFFFFFFF / 1000000) Freq = Nx * 1000000 / Mx;
      else if(Nx < 0x7FFFFFFFFFFFFFFF / 100000)  Freq = Nx * 100000 / Mx * 10;
      else if(Nx < 0x7FFFFFFFFFFFFFFF / 10000)   Freq = Nx * 10000 / Mx * 100;
      else if(Nx < 0x7FFFFFFFFFFFFFFF / 1000)    Freq = Nx * 1000 / Mx * 1000;
      else if(Nx < 0x7FFFFFFFFFFFFFFF / 100)     Freq = Nx * 100 / Mx * 10000;
      else                                       Freq = Nx * 10 / Mx * 100000;
    }
  }
  //statistics:
  if(Freq < Fmin) Fmin = Freq;
  if(Freq > Fmax) Fmax = Freq;
  Fdev = Freq - Fnom;
  if(First) { Count_ClearStat(); First = 0; };
}

//----------------------- Preset averaging filter: ---------------------------

void PresetFilter(long v)
{
  for(char i = 0; i < Average; i++)
    Avg[i] = v;
  AvgPnt = 0;
  AvgVal = (long long)v * Average;
}

//----------------------------------------------------------------------------
//------------------------- Interface functions: -----------------------------
//----------------------------------------------------------------------------

//-------------------------- Set counter mode: -------------------------------

void Count_SetMode(char m)
{
  Mode = m;
  //set CPLD mode:
  if(m == MODE_HI) { Port_MODE0_1; Port_MODE1_0; Port_MODE2_0; }
  else if(m == MODE_LO) { Port_MODE0_0; Port_MODE1_1; Port_MODE2_0; }
  else { Port_MODE0_0; Port_MODE1_0; Port_MODE2_0; }
  //clear count:
  Freq = PulseH = PulseL = 0;
  PresetFilter(0);
}

//--------------------------- Set gate time: ---------------------------------

void Count_SetGate(int g)
{
  T_Gate = (g * 1000L) / (int)T_SYS;
}

//----------------------- Set number of averages: ----------------------------

void Count_SetAvg(char n)
{
  if(n < 1) n = 1;
  if(n > MAX_AVG) n = MAX_AVG;
  Average = n;
  PresetFilter(0);
}

//--------------------------- Set IF value: ----------------------------------

void Count_SetIF(long f)
{
  IFreq = f;
}

//------------------------ Set prescaler ratio: ------------------------------

void Count_SetPre(int p)
{
  if(p < 1) p = 1;
  Prescale = p;
}

//-------------------- Interpolator enable/disable: --------------------------

void Count_SetInt(bool s)
{
  Interpolate = s;
}

//------------------------- Set result scale: --------------------------------

void Count_SetScale(char s)
{
  s = s & 0x0F;
  if(s < 1) s = 1;
  if(s > 7) s = 7;
  Scale = s;
}

//--------------------------- Stop counter: ----------------------------------

void Count_Stop(void)
{
  Port_GATE_0;         //disable count
  Port_LED_0;          //GATE LED off
  State = ST_STOP;
}

//-------------------------- Start counter: ----------------------------------

void Count_Start(void)
{
  if(Mode == MODE_D)
  {
    DutyH = !DutyH;
    if(DutyH) { Port_MODE0_1; Port_MODE1_0; Port_MODE2_0; }
      else { Port_MODE0_0; Port_MODE1_1; Port_MODE2_0; }
  }
  State = ST_START;
}

//---------------------- Enter calibration mode: -----------------------------

void Count_StartCalib(void)
{
  Cnt_Timer = 0;
  State = ST_CALIB;
}

//----------------- Read interpolator calibration value: ---------------------

int Count_GetCalib(void)
{
  return(-Cal / N_CALIB);
}

//------------------------ Read counter ready: -------------------------------

bool Count_Ready(void)
{
  return(State == ST_READY);
}

//------------------------- Statistics clear: --------------------------------

void Count_ClearStat(void)
{
  Fmax = Fmin = Fnom = Freq;
  Fdev = 0;
}

//------------------------ Read counter result: ------------------------------

//0.0000000 Scale = 1 Res = 100 uHz F = Freq / 1
//0000000.0 Scale = 7 Res = 100 Hz  F = Freq / 1000000

//0.0000000 Scale = 1 Res = 100 ps P = 100000000000000 / Freq / 1
//0000000.0 Scale = 7 Res = 100 us P = 100000000000000 / Freq / 1000000

const __flash long ScaleTable[] = { 1, 10, 100, 1000, 10000, 100000, 1000000 };

long Count_GetValue(void)
{
  long s = ScaleTable[Scale - 1]; //scale factor
  long long v = 0; //result

  switch(Mode)
  {
  case MODE_F:  //frequency:
    v = Freq + (long long)IFreq * 1000000;
    break;
  case MODE_P:  //period:
  case MODE_HI: //high pulse duration:
  case MODE_LO: //low pulse duration:
    v = Freq;
    break;
  case MODE_D: //duty cycle:
    if(PulseH && PulseL)
    {
      v = ((long long)PulseH * 10000000 +
             (PulseL + PulseH) / 2) / (PulseL + PulseH);
    }
    break;
  case MODE_R: //rpm:
    v = Freq * 60000;
    break;
  case MODE_FH: //frequency high:
    v = Fmax;
    break;
  case MODE_FL: //frequency low:
    v = Fmin;
    break;
  case MODE_DF: //frequency deviation:
    v = Fdev;
    break;
  }

  if(v < 0) v = (v - s / 2) / s;
    else v = (v + s / 2) / s;
  if(v > 0x7FFFFFFF) v = 0x7FFFFFFF;   //limit to long
  if(v < -0x7FFFFFFF) v = -0x7FFFFFFF;
  long res = (long)v;

  //averaging:
  if(Average > 1)
  {
    //calculate sum:
    AvgVal = AvgVal - Avg[AvgPnt] + res;
    //update array:
    Avg[AvgPnt] = res;
    //calculate averaged value:
    long av = (AvgVal + Average / 2) / Average;
    //advance pointer:
    if(++AvgPnt >= Average) AvgPnt = 0;
    //non-linear filtering:
    long top = av + av / NLR;
    long bot = av - av / NLR;
    if(res > top || res < bot)
      PresetFilter(res);
        else res = av;
  }
  return(res);
}

//----------------------------------------------------------------------------
