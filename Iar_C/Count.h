//----------------------------------------------------------------------------

//Frequency Counter FC-510
//count module header file

//----------------------------------------------------------------------------

#ifndef CountH
#define CountH

//------------------------------- Constants: ---------------------------------

//Counter modes:

enum
{
  MODE_F,  //frequency meter mode
  MODE_P,  //period meter mode
  MODE_HI, //hi-pulse duration meter mode
  MODE_LO, //lo-pulse duration meter mode
  MODE_D,  //duty cycle meter mode
  MODE_R,  //rpm meter mode
  MODE_FH, //frequency high statictic mode
  MODE_FL, //frequency low statictic mode
  MODE_DF, //frequency deviation statictic mode
  //MODE_N   //event counter mode
  //MODE_GC  //gated counter mode
  //MODE_CU  //up counter mode
  //MODE_CD  //down counter mode
  MODES    //modes count  
};

//------------------------- Function prototypes: -----------------------------

void Count_Init(void);       //counter module init
void Count_Exe(bool t);      //execute count process

void Count_SetMode(char m);  //set counter mode
void Count_SetGate(int g);   //set gate time, ms
void Count_SetAvg(char n);   //set number of averages
void Count_SetIF(long f);    //set IF value
void Count_SetPre(int p);    //set prescaler ratio
void Count_SetInt(bool s);   //interpolator enable/disable
void Count_SetScale(char s); //set result scale

void Count_Stop(void);       //stop counter
void Count_Start(void);      //start counter
bool Count_Ready(void);      //read counter ready
long Count_GetValue(void);   //read counter result
void Count_StartCalib(void); //enter calibration mode
int  Count_GetCalib(void);   //read interpolator calibration value
void Count_ClearStat(void);  //clear statistics

//----------------------------------------------------------------------------

#endif
