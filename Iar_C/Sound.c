//----------------------------------------------------------------------------

//Sound generation module

//----------------------------------------------------------------------------

#include "Main.h"
#include "Sound.h"

//----------------------------- Constants: -----------------------------------

#define TICK_D  30.0 //short tick duration, mS
#define BEEP_D 100.0 //beep duration, mS
#define BELL_D 300.0 //error bell duration, mS

#define MS2P(x) (int)(x * 500.0 / T_SYS) //convert mS to timer periods

//------------------------------ Variables: ----------------------------------

int SndTimer; //sound duration in periods

//--------------------------- Tick generation: -------------------------------

__monitor void Sound_Tick(void)
{
  SndTimer = MS2P(TICK_D);
}

//--------------------------- Beep generation: -------------------------------

__monitor void Sound_Beep(void)
{
  SndTimer = MS2P(BEEP_D);
}

//------------------------- Error bell generation: ---------------------------

__monitor void Sound_Bell(void)
{
  SndTimer = MS2P(BELL_D);
}

//----------------------------------------------------------------------------
