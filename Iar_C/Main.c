//----------------------------------------------------------------------------

//Project:         Frequency Counter FC-510
//Version:         V1.4
//Compiler:        IAR EWAVR 5.30
//Microcontroller: ATmega8
//E-mail:          wubblick@yahoo.com

//----------------------------------------------------------------------------

//Fuses: 0xD920
//SPI Enable          (SPIEN = 0)
//External clock      (SKSEL3 = 0, SKSEL2 = 0, SKSEL1 = 0, SKSEL0 = 0)  
//Startup 6CK + 65 ms (SUT0 = 0)
//BOD enabled, 4.0V   (BODLEVEL = 0, BODEN = 0)

//----------------------------------------------------------------------------

#include "Main.h"
#include "Count.h"
#include "Menu.h"
#include "Sound.h"

//------------------------------ Variables: ----------------------------------

volatile bool fTick; //system timer update flag
static bool tick;    //system tick

//------------------------- Function prototypes: -----------------------------

void main(void);                  //main program
void Main_Wdt_Init(void);         //watchdog timer init
void Main_Rst_Wdt(bool t);        //watchdog timer restart
void Main_Ports_Init(void);       //ports init
void Main_Timer_Init(void);       //system timer init
bool Main_GetTick(void);          //get new tick
#pragma vector = TIMER2_COMP_vect
__interrupt void Timer(void);     //system timer interrupt

//----------------------------------------------------------------------------
//---------------------------- Main program: ---------------------------------
//----------------------------------------------------------------------------

void main(void)
{
  Main_Wdt_Init();           //watchdog timer init
  Main_Ports_Init();         //ports init
  Main_Timer_Init();         //system timer init
  Count_Init();              //counter init
  Menu_Init();               //menu init
  __enable_interrupt();      //interrupts enable

  while(1)                   //main cycle
  {
    tick = Main_GetTick();   //tick update check
    Count_Exe(tick);         //do count
    Menu_Exe(tick);          //process menu
    Main_Rst_Wdt(tick);      //watchdog timer restart
  }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//------------------------- Watchdog timer init: -----------------------------

void Main_Wdt_Init(void)
{
  __watchdog_reset();
  WDTCR = (1 << WDCE) | (1 << WDE);
  WDTCR = (1 << WDE) | (1 << WDP2) | (1 << WDP1); //1 s
}

//------------------------ Watchdog timer restart: ---------------------------

void Main_Rst_Wdt(bool t)
{
  if(t)                 //if new tick,
    __watchdog_reset(); //restart watchdog timer
}

//----------------------------- Ports init: ----------------------------------

void Main_Ports_Init(void)
{
  DDRB  = I_DDRB;
  PORTB = I_PORTB;
  DDRC  = I_DDRC;
  PORTC = I_PORTC;
  DDRD  = I_DDRD;
  PORTD = I_PORTD;
}

//------------------------- System timer init: -------------------------------

void Main_Timer_Init(void)
{
  TCCR2 = (1 << WGM21) | (1 << CS22); //timer 2 mode: CTC, CK/64
  OCR2 = (char)(F_CLK * T_SYS / 64.0 - 0.5); //load compare register
  TIFR = (1 << OCIE2);                //pending interrupts clear
  TIMSK |= (1 << OCIE2);              //compare interrupt enable
  fTick = 1;                          //force update
}

//------------------------- System timer check: ------------------------------

__monitor bool Main_GetTick(void)
{
  if(!fTick) return(0); //check system timer update
  fTick = 0; return(1); //new system tick, clear update flag
}

//------------------------ System timer interrupt: ---------------------------

#pragma vector = TIMER2_COMP_vect
__interrupt void Timer(void)
{
  fTick = 1;   //clear timer update flag
  Sound_Gen(); //sound generation
}

//----------------------------------------------------------------------------
