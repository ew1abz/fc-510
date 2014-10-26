//----------------------------------------------------------------------------

//Sound generation module: header file

//----------------------------------------------------------------------------

#ifndef SoundH
#define SoundH

//------------------------------ Variables: ----------------------------------

extern int SndTimer;   //sound duration in periods

//------------------------- Function prototypes: -----------------------------

#pragma inline = forced
void Sound_Gen(void)   //generation sound
{
  if(SndTimer)
  {
    if(Port_SND) Port_SND_0;
     else { Port_SND_1; SndTimer--; }
  }
}

void Sound_Tick(void); //generation short tick
void Sound_Beep(void); //generation beep
void Sound_Bell(void); //generation error bell

//----------------------------------------------------------------------------

#endif

